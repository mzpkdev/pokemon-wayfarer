/* Bounded experiment, never linked into the game. No shipping failure UI/API. */
#include "global.h"
#include "fieldmap.h"
#include "malloc.h"
#include "generated.h"

extern const u8 poc_route47_raw[], poc_route47_lz[], poc_route48_raw[], poc_route48_lz[];
extern void FastLZ77UnCompWram(const u32 *, void *);
extern u8 __bss_start[], __bss_end[];
extern void PaintStack(void);
extern u32 ReadStack(void);
u32 gPocStackTop;

struct ConnectionFlags { u8 south:1; u8 north:1; u8 west:1; u8 east:1; };
u16 ALIGNED(4) sBackupMapData[MAX_MAP_DATA_SIZE];
struct BackupMapLayout gBackupMapLayout;
static struct ConnectionFlags sMapConnectionFlags;
static const struct ConnectionFlags sDummyConnectionFlags;
static u16 baseline[MAX_MAP_DATA_SIZE];
static const struct MapHeader *sNeighbor;
const struct MapHeader *const GetMapHeaderFromConnection(const struct MapConnection *connection)
{
    (void)connection;
    return sNeighbor;
}
#include "legacy.inc"

struct Descriptor {
    const u8 *payload;
    u32 storedBytes, decodedBytes, logicalTileBytes, storedCrc, decodedCrc;
    u8 schemaVersion, codec;
    u16 flags;
};
_Static_assert(sizeof(struct Descriptor)==28, "descriptor ABI");
struct Stats { u32 before, during, after, scratch; };
static struct Stats stats;
static u32 stage[4];
static volatile u32 sink;

static inline volatile u16 *reg16(u32 addr) { return (volatile u16 *)addr; }
static void timer_start(void)
{
    REG_TM2CNT_H=0; REG_TM3CNT_H=0;
    REG_TM2CNT_L=0; REG_TM3CNT_L=0;
    REG_TM3CNT_H=0x84; REG_TM2CNT_H=0x80;
}
static u32 timer_end(void)
{
    /* Stop the source before sampling both halves: no torn carry read. */
    REG_TM2CNT_H=0;
    u32 value=REG_TM2CNT_L | ((u32)REG_TM3CNT_L<<16);
    REG_TM3CNT_H=0;
    return value;
}
static void debug(const char *text)
{
    volatile char *out=(volatile char *)0x04fff600;
    *reg16(0x04fff780)=0xc0de;
    u32 i=0;
    while(text[i] && i<255) { out[i]=text[i]; i++; }
    out[i]=0; *reg16(0x04fff700)=0x103;
}
static void number(char **p,u32 v)
{
    char b[11]; u32 n=0;
    do { b[n++]='0'+v%10; v/=10; } while(v);
    while(n) *(*p)++=b[--n];
}
static void record(const char *tag,u32 a,u32 b,u32 c,u32 d)
{
    char buf[160],*p=buf;
    while(*tag) *p++=*tag++;
    u32 values[]={a,b,c,d};
    for(u32 i=0;i<4;i++) { *p++=' '; number(&p,values[i]); }
    *p=0; debug(buf);
}
static int equal(const void *left,const void *right,u32 size)
{
    const u8 *a=left,*b=right;
    while(size--) if(*a++!=*b++) return 0;
    return 1;
}
static u32 largest_free(void)
{
    const struct MemBlock *head=HeapHead(),*p=head; u32 largest=0;
    do { if(!p->allocated && p->size>largest) largest=p->size; p=p->next; } while(p!=head);
    return largest;
}
static __attribute__((noinline)) u32 crc32(const u8 *p,u32 size)
{
    u32 crc=~0u;
    while(size--) crc=poc_crc32_table[(crc^*p++)&255]^(crc>>8);
    return ~crc;
}
static __attribute__((noinline)) int preflight(const struct Descriptor *d)
{
    const u8 *p=d->payload; u32 out=0,pos=4;
    if(d->storedBytes<4 || p[0]!=0x10 || ((u32)p[1]|(u32)p[2]<<8|(u32)p[3]<<16)!=d->decodedBytes) return 0;
    while(out<d->decodedBytes) {
        if(pos>=d->storedBytes) return 0;
        u8 flags=p[pos++];
        for(u32 bit=0;bit<8 && out<d->decodedBytes;bit++) {
            if(flags&(0x80>>bit)) {
                if(d->storedBytes-pos<2) return 0;
                u32 token=(u32)p[pos]<<8|p[pos+1]; pos+=2;
                u32 count=(token>>12)+3,distance=(token&0xfff)+1;
                if(distance>out || count>d->decodedBytes-out) return 0;
                out+=count;
            } else { if(pos>=d->storedBytes) return 0; pos++; out++; }
        }
    }
    if(((pos+3)&~3u)!=d->storedBytes) return 0;
    while(pos<d->storedBytes) if(p[pos++]) return 0;
    return 1;
}
static const u16 *open_view(const struct Descriptor *d,const struct MapLayout *layout,u16 *scratch,u32 capacity)
{
    if(d->schemaVersion!=1 || d->codec>1 || d->flags || !d->payload || ((u32)d->payload&3)
       || !d->decodedBytes || d->decodedBytes>14640 || d->logicalTileBytes!=layout->width*layout->height*2
       || d->logicalTileBytes>d->decodedBytes || !d->storedBytes || d->storedBytes>14640) return NULL;
    u32 crc=crc32(d->payload,d->storedBytes);
    if(crc!=d->storedCrc) return NULL;
    if(!d->codec) {
        if(d->storedBytes!=d->decodedBytes || crc!=d->decodedCrc) return NULL;
        return (const u16 *)d->payload;
    }
    if(d->decodedBytes>capacity || !scratch || !preflight(d)) return NULL;
    FastLZ77UnCompWram((const u32 *)d->payload,scratch);
    if(crc32((const u8 *)scratch,d->decodedBytes)!=d->decodedCrc) return NULL;
    return scratch;
}
static void clear_grid(const struct MapLayout *layout)
{
    CpuFastFill16(MAPGRID_UNDEFINED,sBackupMapData,sizeof(sBackupMapData));
    gBackupMapLayout.map=sBackupMapData;
    gBackupMapLayout.width=layout->width+MAP_OFFSET_W;
    gBackupMapLayout.height=layout->height+MAP_OFFSET_H;
}
/* Same copy primitives as the oracle; one scratch, reused before a neighbor opens. */
static __attribute__((noinline)) int candidate(struct MapHeader *header,const struct Descriptor *a,const struct Descriptor *b,int inspect)
{
    clear_grid(header->mapLayout);
    if(gBackupMapLayout.width*gBackupMapLayout.height>MAX_MAP_DATA_SIZE) return 0;
    u32 capacity=a->codec?a->decodedBytes:0;
    if(b && b->codec && b->decodedBytes>capacity) capacity=b->decodedBytes;
    u16 *scratch=capacity?Alloc_(capacity,NULL):NULL;
    if(capacity && !scratch) return 0;
    if(inspect) { stats.scratch=capacity; stats.during=largest_free(); }
    const u16 *view=open_view(a,header->mapLayout,scratch,capacity);
    if(!view) goto fail;
    InitBackupMapLayoutData(view,header->mapLayout->width,header->mapLayout->height);
    if(header->connections) {
        struct MapLayout local=*sNeighbor->mapLayout;
        struct MapHeader neighbor=*sNeighbor;
        neighbor.mapLayout=&local;
        local.map=open_view(b,&local,scratch,capacity);
        if(!local.map) goto fail;
        /* Each fixture has exactly one real connection; no invented strip geometry. */
        const struct MapConnection *connection=header->connections->connections;
        sMapConnectionFlags=sDummyConnectionFlags;
        if(connection->direction==CONNECTION_NORTH) {
            FillNorthConnection(header,&neighbor,connection->offset); sMapConnectionFlags.north=TRUE;
        } else {
            FillSouthConnection(header,&neighbor,connection->offset); sMapConnectionFlags.south=TRUE;
        }
    }
    /* This scribble is functional-only, outside the benchmark configuration. */
    if(inspect && scratch) for(u32 i=0;i<capacity/2;i++) scratch[i]=0xdead;
    Free(scratch);
    return 1;
fail:
    Free(scratch);
    clear_grid(header->mapLayout);
    return 0;
}
static int source_point(const struct Descriptor *d,const struct MapLayout *layout,u32 x,u32 y,u16 *value)
{
    if(x>=layout->width || y>=layout->height) return 0;
    u16 *scratch=d->codec?Alloc_(d->decodedBytes,NULL):NULL;
    const u16 *view=open_view(d,layout,scratch,d->decodedBytes);
    if(view) *value=view[y*layout->width+x];
    Free(scratch);
    return view!=NULL;
}
static void profile(const struct Descriptor *d)
{
    u16 *scratch=Alloc_(d->decodedBytes,NULL);
    timer_start(); sink=crc32(d->payload,d->storedBytes); stage[0]+=timer_end();
    timer_start(); sink=preflight(d); stage[1]+=timer_end();
    timer_start(); FastLZ77UnCompWram((const u32*)d->payload,scratch); stage[2]+=timer_end();
    timer_start(); sink=crc32((const u8*)scratch,d->decodedBytes); stage[3]+=timer_end();
    Free(scratch);
}
static int run_case(u32 id,struct MapHeader *header,const struct Descriptor *ar,const struct Descriptor *br,const struct Descriptor *ah,const struct Descriptor *bh)
{
    sMapConnectionFlags=sDummyConnectionFlags;
    InitMapLayoutData(header);
    memcpy(baseline,sBackupMapData,sizeof baseline);
    u32 width=gBackupMapLayout.width,height=gBackupMapLayout.height;
    struct ConnectionFlags flags=sMapConnectionFlags;
    stats.before=largest_free();
    if(!candidate(header,ah,bh,1)) return 0;
    stats.after=largest_free();
    if(stats.before!=stats.after || !equal(baseline,sBackupMapData,sizeof baseline)) return 0;
    record("HEAP",id,stats.before,stats.during,stats.after);
    record("SCRATCH",id,stats.scratch,stats.scratch+16,0);
    PaintStack();
    int ok=candidate(header,ah,bh,0);
    u32 stack=ReadStack();
    if(!ok) return 0;
    record("STACK",id,stack,gPocStackTop,0);
    for(u32 n=0;n<100;n++) for(u32 mode=0;mode<3;mode++) {
        sMapConnectionFlags=sDummyConnectionFlags;
        timer_start();
        if(!mode) InitMapLayoutData(header);
        else ok=candidate(header,mode==1?ar:ah,mode==1?br:bh,0);
        u32 cycles=timer_end();
        if(!ok || !equal(baseline,sBackupMapData,sizeof baseline) || gBackupMapLayout.map!=sBackupMapData
           || width!=gBackupMapLayout.width || height!=gBackupMapLayout.height
           || !equal(&flags,&sMapConnectionFlags,sizeof flags)) return 0;
        record("S",id,mode,n,cycles);
    }
    u16 value;
    if(!source_point(ah,header->mapLayout,4,3,&value) || value!=header->mapLayout->map[3*header->mapLayout->width+4]
       || UNPACK_COLLISION(value)!=UNPACK_COLLISION(baseline[(3+7)*width+4+7])) return 0;
    record("POINT",id,value,UNPACK_COLLISION(value),UNPACK_ELEVATION(value));
    for(u32 i=0;i<4;i++) stage[i]=0;
    profile(ah); if(bh) profile(bh);
    record("STAGES",stage[0],stage[1],stage[2],stage[3]);
    return largest_free()==stats.before;
}
void main(void)
{
    for(u8 *p=__bss_start;p<__bss_end;p++) *p=0;
    REG_WAITCNT=0x40b4;
    const struct MapLayout la={.width=120,.height=61,.map=(const u16*)poc_route47_raw};
    const struct MapLayout lb={.width=55,.height=32,.map=(const u16*)poc_route48_raw};
    const struct MapConnection north={.direction=CONNECTION_NORTH,.offset=19};
    const struct MapConnection south={.direction=CONNECTION_SOUTH,.offset=-19};
    const struct MapConnections nc={.count=1,.connections=&north},sc={.count=1,.connections=&south};
    struct MapHeader a={.mapLayout=&la}, b={.mapLayout=&lb};
    const struct Descriptor ar={poc_route47_raw,ROUTE47_RAW_BYTES,ROUTE47_RAW_BYTES,ROUTE47_RAW_BYTES,ROUTE47_RAW_CRC,ROUTE47_RAW_CRC,1,0,0};
    const struct Descriptor br={poc_route48_raw,ROUTE48_RAW_BYTES,ROUTE48_RAW_BYTES,ROUTE48_RAW_BYTES,ROUTE48_RAW_CRC,ROUTE48_RAW_CRC,1,0,0};
    const struct Descriptor ah={poc_route47_lz,ROUTE47_LZ_BYTES,ROUTE47_RAW_BYTES,ROUTE47_RAW_BYTES,ROUTE47_LZ_CRC,ROUTE47_RAW_CRC,1,1,0};
    const struct Descriptor bh={poc_route48_lz,ROUTE48_LZ_BYTES,ROUTE48_RAW_BYTES,ROUTE48_RAW_BYTES,ROUTE48_LZ_CRC,ROUTE48_RAW_CRC,1,1,0};
    InitHeap(gHeap,HEAP_SIZE);
    int ok=crc32((const u8*)"123456789",9)==0xcbf43926;
    sNeighbor=&b;
    ok=ok && run_case(0,&a,&ar,NULL,&ah,NULL);
    a.connections=&nc;
    ok=ok && run_case(1,&a,&ar,&br,&ah,&bh);
    b.connections=&sc; sNeighbor=&a;
    ok=ok && run_case(2,&b,&br,&ar,&bh,&ah);
    /* Fragmentation counterexample: total free > request, largest block < request. */
    void *blocks[8];
    for(u32 i=0;i<8;i++) blocks[i]=Alloc_(14000,NULL);
    for(u32 i=0;i<8;i+=2) Free(blocks[i]);
    u32 before=largest_free();
    ok=ok && before==14000 && !candidate(&b,&bh,&ah,0) && largest_free()==before;
    for(u32 i=0;i<MAX_MAP_DATA_SIZE;i++) if(sBackupMapData[i]!=MAPGRID_UNDEFINED) ok=0;
    for(u32 i=1;i<8;i+=2) Free(blocks[i]);
    record("FRAGMENT",before,largest_free(),14640,ok);
    debug(ok?"PASS":"FAIL");
    register u32 code __asm__("r0")=ok?0:1;
    __asm__ volatile("swi 3"::"r"(code));
    for(;;) {}
}
