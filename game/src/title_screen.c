#include "global.h"
#include "battle.h"
#include "title_screen.h"
#include "sprite.h"
#include "gba/m4a_internal.h"
#include "clear_save_data_menu.h"
#include "decompress.h"
#include "event_data.h"
#include "intro.h"
#include "m4a.h"
#include "main.h"
#include "main_menu.h"
#include "palette.h"
#include "reset_rtc_screen.h"
#include "berry_fix_program.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "scanline_effect.h"
#include "gpu_regs.h"
#include "trig.h"
#include "graphics.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#if IS_WAYFARER

// Scene 1 uses this shared blend ramp while the Game Freak mark disappears.
const u16 gTitleScreenAlphaBlend[64] =
{
    BLDALPHA_BLEND(16, 0), BLDALPHA_BLEND(16, 1), BLDALPHA_BLEND(16, 2), BLDALPHA_BLEND(16, 3),
    BLDALPHA_BLEND(16, 4), BLDALPHA_BLEND(16, 5), BLDALPHA_BLEND(16, 6), BLDALPHA_BLEND(16, 7),
    BLDALPHA_BLEND(16, 8), BLDALPHA_BLEND(16, 9), BLDALPHA_BLEND(16, 10), BLDALPHA_BLEND(16, 11),
    BLDALPHA_BLEND(16, 12), BLDALPHA_BLEND(16, 13), BLDALPHA_BLEND(16, 14), BLDALPHA_BLEND(16, 15),
    BLDALPHA_BLEND(15, 16), BLDALPHA_BLEND(14, 16), BLDALPHA_BLEND(13, 16), BLDALPHA_BLEND(12, 16),
    BLDALPHA_BLEND(11, 16), BLDALPHA_BLEND(10, 16), BLDALPHA_BLEND(9, 16), BLDALPHA_BLEND(8, 16),
    BLDALPHA_BLEND(7, 16), BLDALPHA_BLEND(6, 16), BLDALPHA_BLEND(5, 16), BLDALPHA_BLEND(4, 16),
    BLDALPHA_BLEND(3, 16), BLDALPHA_BLEND(2, 16), BLDALPHA_BLEND(1, 16), BLDALPHA_BLEND(0, 16),
    [32 ... 63] = BLDALPHA_BLEND(0, 16),
};

// Scene 1 owns every mode-0 background.  This title path deliberately never
// calls the normal initializer, which clears VRAM and replaces that landscape.
enum
{
    TAG_WAYFARER_POKEMON_LOGO = 1000,
    TAG_WAYFARER_VERSION,
    TAG_WAYFARER_PRESS_START,
    TAG_WAYFARER_VOLBEAT,
    TAG_WAYFARER_TORCHIC,
};

#define WAYFARER_LOGO_Y 32
#define WAYFARER_VERSION_Y 66
#define WAYFARER_PRESS_START_Y 108
#define WAYFARER_COPYRIGHT_Y 148
#define WAYFARER_VOLBEAT_Y 96
#define WAYFARER_TORCHIC_Y 88
#define CLEAR_SAVE_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON | DPAD_UP)
#define RESET_RTC_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON | DPAD_LEFT)

static void MainCB2_WayfarerTitleScreen(void);
static void VBlankCB_WayfarerTitleScreen(void);
static void Task_WayfarerTitleReveal(u8 taskId);
static void Task_WayfarerTitleInput(u8 taskId);
static void Task_WayfarerPokemonPass(u8 taskId);
static void CB2_GoToMainMenu(void);
static void CB2_GoToClearSaveDataScreen(void);
static void CB2_GoToResetRtcScreen(void);
static void SpriteCB_PressStart(struct Sprite *sprite);

static const u16 sWayfarerOverlayPalette[] = INCBIN_U16("graphics/title_screen/wayfarer/overlays/overlay_palette.gbapal");
static const u32 sWayfarerPokemonLogoGfx[] = INCBIN_U32("graphics/title_screen/wayfarer/overlays/pokemon_logo_obj.8bpp.smol");
static const u16 sWayfarerVersionPal[] = INCBIN_U16("graphics/title_screen/wayfarer/wayfarer_version.gbapal");
static const u32 sWayfarerVersionGfx[] = INCBIN_U32("graphics/title_screen/wayfarer/wayfarer_version.4bpp.smol");

static const struct OamData sOamData_WayfarerPokemonLogo =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_8BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sOamData_WayfarerVersion =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sOamData_WayfarerPressStart =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sOamData_WayfarerPassingPokemon =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 3, // Behind the moving grass (BG2), but in front of the mountain (BG3).
    .paletteNum = 0,
    .affineParam = 0,
};

#define LOGO_ANIM(offset) { ANIMCMD_FRAME(offset, 1), ANIMCMD_END }
static const union AnimCmd sAnim_WayfarerPokemonLogo0[] = LOGO_ANIM(0);
static const union AnimCmd sAnim_WayfarerPokemonLogo1[] = LOGO_ANIM(128);
static const union AnimCmd sAnim_WayfarerPokemonLogo2[] = LOGO_ANIM(256);
static const union AnimCmd sAnim_WayfarerPokemonLogo3[] = LOGO_ANIM(384);
#undef LOGO_ANIM
static const union AnimCmd *const sAnims_WayfarerPokemonLogo0[] = {sAnim_WayfarerPokemonLogo0};
static const union AnimCmd *const sAnims_WayfarerPokemonLogo1[] = {sAnim_WayfarerPokemonLogo1};
static const union AnimCmd *const sAnims_WayfarerPokemonLogo2[] = {sAnim_WayfarerPokemonLogo2};
static const union AnimCmd *const sAnims_WayfarerPokemonLogo3[] = {sAnim_WayfarerPokemonLogo3};

static const struct SpriteTemplate sSpriteTemplate_WayfarerPokemonLogo[] =
{
    {TAG_WAYFARER_POKEMON_LOGO, TAG_WAYFARER_POKEMON_LOGO, &sOamData_WayfarerPokemonLogo, sAnims_WayfarerPokemonLogo0, NULL, NULL, SpriteCallbackDummy},
    {TAG_WAYFARER_POKEMON_LOGO, TAG_WAYFARER_POKEMON_LOGO, &sOamData_WayfarerPokemonLogo, sAnims_WayfarerPokemonLogo1, NULL, NULL, SpriteCallbackDummy},
    {TAG_WAYFARER_POKEMON_LOGO, TAG_WAYFARER_POKEMON_LOGO, &sOamData_WayfarerPokemonLogo, sAnims_WayfarerPokemonLogo2, NULL, NULL, SpriteCallbackDummy},
    {TAG_WAYFARER_POKEMON_LOGO, TAG_WAYFARER_POKEMON_LOGO, &sOamData_WayfarerPokemonLogo, sAnims_WayfarerPokemonLogo3, NULL, NULL, SpriteCallbackDummy},
};

static const union AnimCmd sAnim_WayfarerVersionLeft[] = {ANIMCMD_FRAME(0, 1), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerVersionRight[] = {ANIMCMD_FRAME(32, 1), ANIMCMD_END};
static const union AnimCmd *const sAnims_WayfarerVersionLeft[] = {sAnim_WayfarerVersionLeft};
static const union AnimCmd *const sAnims_WayfarerVersionRight[] = {sAnim_WayfarerVersionRight};
static const struct SpriteTemplate sSpriteTemplate_WayfarerVersionLeft =
{
    TAG_WAYFARER_VERSION, TAG_WAYFARER_VERSION, &sOamData_WayfarerVersion, sAnims_WayfarerVersionLeft, NULL, NULL, SpriteCallbackDummy,
};
static const struct SpriteTemplate sSpriteTemplate_WayfarerVersionRight =
{
    TAG_WAYFARER_VERSION, TAG_WAYFARER_VERSION, &sOamData_WayfarerVersion, sAnims_WayfarerVersionRight, NULL, NULL, SpriteCallbackDummy,
};

static const union AnimCmd sAnim_WayfarerPressStart[] = {ANIMCMD_FRAME(1, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerPressStart1[] = {ANIMCMD_FRAME(5, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerPressStart2[] = {ANIMCMD_FRAME(9, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerPressStart3[] = {ANIMCMD_FRAME(13, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerPressStart4[] = {ANIMCMD_FRAME(17, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerCopyright[] = {ANIMCMD_FRAME(21, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerCopyright1[] = {ANIMCMD_FRAME(25, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerCopyright2[] = {ANIMCMD_FRAME(29, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerCopyright3[] = {ANIMCMD_FRAME(33, 4), ANIMCMD_END};
static const union AnimCmd sAnim_WayfarerCopyright4[] = {ANIMCMD_FRAME(37, 4), ANIMCMD_END};
static const union AnimCmd *const sAnims_WayfarerPressStart[] = {
    sAnim_WayfarerPressStart, sAnim_WayfarerPressStart1, sAnim_WayfarerPressStart2,
    sAnim_WayfarerPressStart3, sAnim_WayfarerPressStart4, sAnim_WayfarerCopyright,
    sAnim_WayfarerCopyright1, sAnim_WayfarerCopyright2, sAnim_WayfarerCopyright3, sAnim_WayfarerCopyright4,
};
static const struct SpriteTemplate sSpriteTemplate_WayfarerPressStart =
{
    TAG_WAYFARER_PRESS_START, TAG_WAYFARER_PRESS_START, &sOamData_WayfarerPressStart, sAnims_WayfarerPressStart, NULL, NULL, SpriteCB_PressStart,
};

static const union AnimCmd sAnim_WayfarerVolbeat[] =
{
    ANIMCMD_FRAME(0, 2), ANIMCMD_FRAME(16, 2), ANIMCMD_JUMP(0),
};
static const union AnimCmd *const sAnims_WayfarerVolbeat[] = {sAnim_WayfarerVolbeat};
static const union AnimCmd sAnim_WayfarerTorchic[] =
{
    ANIMCMD_FRAME(0, 3), ANIMCMD_FRAME(16, 3), ANIMCMD_FRAME(32, 3),
    ANIMCMD_FRAME(16, 3), ANIMCMD_JUMP(0),
};
static const union AnimCmd sAnim_WayfarerTorchicTrip[] =
{
    ANIMCMD_FRAME(48, 4), ANIMCMD_FRAME(64, 6), ANIMCMD_FRAME(80, 0), ANIMCMD_END,
};
static const union AnimCmd sAnim_WayfarerTorchicGetUp[] =
{
    ANIMCMD_FRAME(80, 4), ANIMCMD_FRAME(64, 6), ANIMCMD_FRAME(48, 4),
    ANIMCMD_FRAME(32, 3), ANIMCMD_END,
};
enum
{
    WAYFARER_TORCHIC_ANIM_RUN,
    WAYFARER_TORCHIC_ANIM_TRIP,
    WAYFARER_TORCHIC_ANIM_GET_UP,
};
static const union AnimCmd *const sAnims_WayfarerTorchic[] =
{
    sAnim_WayfarerTorchic, sAnim_WayfarerTorchicTrip, sAnim_WayfarerTorchicGetUp,
};
enum
{
    WAYFARER_TORCHIC_RUN,
    WAYFARER_TORCHIC_TRIP,
    WAYFARER_TORCHIC_FALLEN,
    WAYFARER_TORCHIC_GET_UP,
    WAYFARER_TORCHIC_RUN_AGAIN,
};
static const struct SpriteTemplate sSpriteTemplate_WayfarerVolbeat =
{
    TAG_WAYFARER_VOLBEAT, TAG_WAYFARER_VOLBEAT, &sOamData_WayfarerPassingPokemon, sAnims_WayfarerVolbeat, NULL, NULL, SpriteCallbackDummy,
};
static const struct SpriteTemplate sSpriteTemplate_WayfarerTorchic =
{
    TAG_WAYFARER_TORCHIC, TAG_WAYFARER_TORCHIC, &sOamData_WayfarerPassingPokemon, sAnims_WayfarerTorchic, NULL, NULL, SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_WayfarerPokemonLogo =
{
    sWayfarerPokemonLogoGfx, 0x4000, TAG_WAYFARER_POKEMON_LOGO,
};
static const struct CompressedSpriteSheet sSpriteSheet_WayfarerVersion =
{
    sWayfarerVersionGfx, 0x800, TAG_WAYFARER_VERSION,
};
static const struct CompressedSpriteSheet sSpriteSheet_WayfarerPressStart =
{
    gTitleScreenPressStartGfx, 0x520, TAG_WAYFARER_PRESS_START,
};
static const struct CompressedSpriteSheet sSpriteSheet_WayfarerVolbeat =
{
    gIntroVolbeat_Gfx, 0x400, TAG_WAYFARER_VOLBEAT,
};
static const struct CompressedSpriteSheet sSpriteSheet_WayfarerTorchic =
{
    gIntroTorchic_Gfx, 0xC00, TAG_WAYFARER_TORCHIC,
};

static void CreateWayfarerPressStart(s16 y)
{
    u8 i;
    for (i = 0; i < 5; i++)
    {
        u8 spriteId = CreateSprite(&sSpriteTemplate_WayfarerPressStart, 64 + i * 32, y, 0);
        StartSpriteAnim(&gSprites[spriteId], i);
    }
}

static void CreateWayfarerTitleSprites(void)
{
    // Center the visible lettering, not its 256px atlas with empty right space.
    static const s16 sLogoX[] = {64, 128, 192, 256};
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sLogoX); i++)
        CreateSprite(&sSpriteTemplate_WayfarerPokemonLogo[i], sLogoX[i], WAYFARER_LOGO_Y, 0);
    CreateSprite(&sSpriteTemplate_WayfarerVersionLeft, 88, WAYFARER_VERSION_Y, 1);
    CreateSprite(&sSpriteTemplate_WayfarerVersionRight, 152, WAYFARER_VERSION_Y, 1);
    CreateWayfarerPressStart(WAYFARER_PRESS_START_Y);
}

static void SpriteCB_PressStart(struct Sprite *sprite)
{
    if (++sprite->data[0] & 16)
        sprite->invisible = FALSE;
    else
        sprite->invisible = TRUE;
}

static void Task_WayfarerPokemonPass(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    struct Sprite *sprite = &gSprites[task->data[task->data[1] + 2]];

    if (task->data[0] != 0)
    {
        task->data[0]--;
        return;
    }

    sprite->invisible = FALSE;
    if (task->data[1] == 0) // Volbeat flies; Torchic stays on the ground.
    {
        sprite->x += 2;
        sprite->y2 = Sin((u8)(task->data[4] += 4), 3);
    }
    else
    {
        switch (task->data[5])
        {
        case WAYFARER_TORCHIC_RUN:
            sprite->x -= 2;
            if (sprite->x <= 120)
            {
                sprite->x = 120;
                StartSpriteAnim(sprite, WAYFARER_TORCHIC_ANIM_TRIP);
                task->data[5] = WAYFARER_TORCHIC_TRIP;
            }
            break;
        case WAYFARER_TORCHIC_TRIP:
            if (sprite->animEnded)
            {
                task->data[6] = 24;
                task->data[5] = WAYFARER_TORCHIC_FALLEN;
            }
            break;
        case WAYFARER_TORCHIC_FALLEN:
            if (--task->data[6] == 0)
            {
                StartSpriteAnim(sprite, WAYFARER_TORCHIC_ANIM_GET_UP);
                task->data[5] = WAYFARER_TORCHIC_GET_UP;
            }
            break;
        case WAYFARER_TORCHIC_GET_UP:
            if (sprite->animEnded)
            {
                StartSpriteAnim(sprite, WAYFARER_TORCHIC_ANIM_RUN);
                task->data[5] = WAYFARER_TORCHIC_RUN_AGAIN;
            }
            break;
        case WAYFARER_TORCHIC_RUN_AGAIN:
            sprite->x -= 2;
            break;
        }
    }
    if ((task->data[1] == 0 && sprite->x > DISPLAY_WIDTH + 16)
     || (task->data[1] != 0 && sprite->x < -16))
    {
        if (task->data[1] != 0)
        {
            StartSpriteAnim(sprite, WAYFARER_TORCHIC_ANIM_RUN);
            task->data[5] = WAYFARER_TORCHIC_RUN;
        }
        sprite->x = task->data[1] == 0 ? -16 : DISPLAY_WIDTH + 16;
        sprite->y2 = 0;
        sprite->invisible = TRUE;
        task->data[0] = 600;
        task->data[1] ^= 1;
        task->data[4] = 0;
    }
}

// Called by Scene 1 after it has fixed its backgrounds, Flygon, and matrix.
void InitWayfarerTitleScreenFromIntro(const u16 *retainedSpritePalette, u16 retainedSpritePaletteTag)
{
    struct SpritePalette retainedPalette = {retainedSpritePalette, retainedSpritePaletteTag};
    u8 taskId;

    ScanlineEffect_Stop();
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    LoadPalette(sWayfarerOverlayPalette, OBJ_PLTT_ID(0), 16 * PLTT_SIZE_4BPP);
    FreeAllSpritePalettes();
    AllocSpritePalette(TAG_WAYFARER_POKEMON_LOGO); // 8bpp logo and banner share OBJ palette 0.
    LoadSpritePaletteInSlot(&(struct SpritePalette){sWayfarerVersionPal, TAG_WAYFARER_VERSION}, 11);
    LoadSpritePaletteInSlot(&(struct SpritePalette){gTitleScreenPressStartPal, TAG_WAYFARER_PRESS_START}, 12);
    LoadSpritePaletteInSlot(&retainedPalette, 13);
    LoadSpritePaletteInSlot(&(struct SpritePalette){gIntroVolbeat_Pal, TAG_WAYFARER_VOLBEAT}, 14);
    LoadSpritePaletteInSlot(&(struct SpritePalette){gIntroTorchic_Pal, TAG_WAYFARER_TORCHIC}, 15);
    LoadCompressedSpriteSheet(&sSpriteSheet_WayfarerPokemonLogo);
    LoadCompressedSpriteSheet(&sSpriteSheet_WayfarerVersion);
    LoadCompressedSpriteSheet(&sSpriteSheet_WayfarerPressStart);
    LoadCompressedSpriteSheet(&sSpriteSheet_WayfarerVolbeat);
    LoadCompressedSpriteSheet(&sSpriteSheet_WayfarerTorchic);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_ON);
    SetVBlankCallback(VBlankCB_WayfarerTitleScreen);
    taskId = CreateTask(Task_WayfarerTitleReveal, 0);
    // Let Scene 1's final music phrase land on the frozen mountain before the
    // title artwork and title theme take over.
    gTasks[taskId].data[0] = 117;
    SetMainCallback2(MainCB2_WayfarerTitleScreen);
}

static void VBlankCB_WayfarerTitleScreen(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void MainCB2_WayfarerTitleScreen(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void Task_WayfarerTitleReveal(u8 taskId)
{
    if (gTasks[taskId].data[0] != 0)
    {
        gTasks[taskId].data[0]--;
        return;
    }
    FadeOutBGM(4);
    m4aSongNumStart(MUS_HG_TITLE, FlagGet(FLAG_SYS_GBS_ENABLED));
    CreateWayfarerTitleSprites();
    {
        u8 passTaskId = CreateTask(Task_WayfarerPokemonPass, 0);
        gTasks[passTaskId].data[0] = 120;
        gTasks[passTaskId].data[2] = CreateSprite(&sSpriteTemplate_WayfarerVolbeat, -16, WAYFARER_VOLBEAT_Y, 2);
        gTasks[passTaskId].data[3] = CreateSprite(&sSpriteTemplate_WayfarerTorchic, DISPLAY_WIDTH + 16, WAYFARER_TORCHIC_Y, 2);
        gSprites[gTasks[passTaskId].data[2]].hFlip = TRUE;
        gSprites[gTasks[passTaskId].data[2]].invisible = TRUE;
        gSprites[gTasks[passTaskId].data[3]].invisible = TRUE;
    }
    taskId = CreateTask(Task_WayfarerTitleInput, 0);
    gTasks[taskId].data[1] = TRUE; // Consume the cinematic skip press.
    DestroyTask(FindTaskIdByFunc(Task_WayfarerTitleReveal));
}

static void Task_WayfarerTitleInput(u8 taskId)
{
    if (gTasks[taskId].data[1])
    {
        gTasks[taskId].data[1] = FALSE;
        return;
    }
    if (JOY_NEW(A_BUTTON) || JOY_NEW(START_BUTTON))
    {
        FadeOutBGM(4);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITEALPHA);
        SetMainCallback2(CB2_GoToMainMenu);
    }
    else if (JOY_HELD(CLEAR_SAVE_BUTTON_COMBO) == CLEAR_SAVE_BUTTON_COMBO)
    {
        SetMainCallback2(CB2_GoToClearSaveDataScreen);
    }
    else if (JOY_HELD(RESET_RTC_BUTTON_COMBO) == RESET_RTC_BUTTON_COMBO && CanResetRTC())
    {
        FadeOutBGM(4);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        SetMainCallback2(CB2_GoToResetRtcScreen);
    }
    else if ((gMPlayInfo_BGM.status & 0xFFFF) == 0)
    {
        // A music end must keep this held composition; never return to intro.
        m4aSongNumStart(MUS_HG_TITLE, FlagGet(FLAG_SYS_GBS_ENABLED));
    }
}

static void CB2_GoToMainMenu(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitMainMenu);
}

static void CB2_GoToClearSaveDataScreen(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitClearSaveDataScreen);
}

static void CB2_GoToResetRtcScreen(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitResetRtcScreen);
}

// Auxiliary screens still use this public entry point.  Reload Scene 1, then
// immediately establish the same held title contract without replaying it.
void CB2_InitTitleScreen(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        CpuFill32(0, (void *)OAM, OAM_SIZE);
        CpuFill16(0, (void *)PLTT, PLTT_SIZE);
        ResetPaletteFade();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    default:
        RequestWayfarerIntroTitleHold();
        CreateTask(Task_Scene1_Load, 0);
        SetMainCallback2(MainCB2_Intro);
        break;
    }
}

#else

enum {
    TAG_VERSION = 1000,
    TAG_PRESS_START_COPYRIGHT,
    TAG_LOGO_SHINE,
};

#define VERSION_BANNER_RIGHT_TILEOFFSET 64
#define VERSION_BANNER_LEFT_X 98
#define VERSION_BANNER_RIGHT_X 162
#define VERSION_BANNER_Y 2
#if IS_HNS
#define VERSION_BANNER_Y_GOAL 58
#else
#define VERSION_BANNER_Y_GOAL 66
#endif
#define START_BANNER_X 128

#define CLEAR_SAVE_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON | DPAD_UP)
#define RESET_RTC_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON | DPAD_LEFT)
#if ENABLE_BERRY_GLITCH_FIX_MULTIBOOT
#define BERRY_UPDATE_BUTTON_COMBO (B_BUTTON | SELECT_BUTTON)
#endif
#define A_B_START_SELECT (A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON)

static void MainCB2(void);
static void Task_TitleScreenPhase1(u8);
static void Task_TitleScreenPhase2(u8);
static void Task_TitleScreenPhase3(u8);
static void CB2_GoToMainMenu(void);
static void CB2_GoToClearSaveDataScreen(void);
static void CB2_GoToResetRtcScreen(void);
#if ENABLE_BERRY_GLITCH_FIX_MULTIBOOT
static void CB2_GoToBerryFixScreen(void);
#endif
static void CB2_GoToCopyrightScreen(void);
static void UpdateLegendaryMarkingColor(u8);

static void SpriteCB_VersionBannerLeft(struct Sprite *sprite);
static void SpriteCB_VersionBannerRight(struct Sprite *sprite);
static void SpriteCB_PressStartCopyrightBanner(struct Sprite *sprite);
static void SpriteCB_PokemonLogoShine(struct Sprite *sprite);

// const rom data
static const u16 sUnusedUnknownPal[] = INCBIN_U16("graphics/title_screen/unused.gbapal");

#if IS_WAYFARER
static const u32 sTitleScreenRayquazaGfx[] = INCBIN_U32("graphics/title_screen/wayfarer/background/scene.8bpp.smol");
static const u32 sTitleScreenRayquazaTilemap[] = INCBIN_U32("graphics/title_screen/wayfarer/background/scene.bin.smolTM");
static const u32 sTitleScreenLogoShineGfx[] = INCBIN_U32("graphics/title_screen/hns/logo_shine.4bpp.smol");
#elif IS_HNS
static const u32 sTitleScreenRayquazaGfx[] = INCBIN_U32("graphics/title_screen/hns/rayquaza.4bpp.smol");
static const u32 sTitleScreenRayquazaTilemap[] = INCBIN_U32("graphics/title_screen/hns/rayquaza.bin.smolTM");
static const u32 sTitleScreenLogoShineGfx[] = INCBIN_U32("graphics/title_screen/hns/logo_shine.4bpp.smol");
#else
static const u32 sTitleScreenRayquazaGfx[] = INCBIN_U32("graphics/title_screen/rayquaza.4bpp.smol");
static const u32 sTitleScreenRayquazaTilemap[] = INCBIN_U32("graphics/title_screen/rayquaza.bin.smolTM");
static const u32 sTitleScreenLogoShineGfx[] = INCBIN_U32("graphics/title_screen/logo_shine.4bpp.smol");
static const u32 sTitleScreenCloudsGfx[] = INCBIN_U32("graphics/title_screen/clouds.4bpp.smol");
#endif



// Used to blend "Emerald Version" as it passes over over the Pokémon banner.
// Also used by the intro to blend the Game Freak name/logo in and out as they appear and disappear
const u16 gTitleScreenAlphaBlend[64] =
{
    BLDALPHA_BLEND(16, 0),
    BLDALPHA_BLEND(16, 1),
    BLDALPHA_BLEND(16, 2),
    BLDALPHA_BLEND(16, 3),
    BLDALPHA_BLEND(16, 4),
    BLDALPHA_BLEND(16, 5),
    BLDALPHA_BLEND(16, 6),
    BLDALPHA_BLEND(16, 7),
    BLDALPHA_BLEND(16, 8),
    BLDALPHA_BLEND(16, 9),
    BLDALPHA_BLEND(16, 10),
    BLDALPHA_BLEND(16, 11),
    BLDALPHA_BLEND(16, 12),
    BLDALPHA_BLEND(16, 13),
    BLDALPHA_BLEND(16, 14),
    BLDALPHA_BLEND(16, 15),
    BLDALPHA_BLEND(15, 16),
    BLDALPHA_BLEND(14, 16),
    BLDALPHA_BLEND(13, 16),
    BLDALPHA_BLEND(12, 16),
    BLDALPHA_BLEND(11, 16),
    BLDALPHA_BLEND(10, 16),
    BLDALPHA_BLEND(9, 16),
    BLDALPHA_BLEND(8, 16),
    BLDALPHA_BLEND(7, 16),
    BLDALPHA_BLEND(6, 16),
    BLDALPHA_BLEND(5, 16),
    BLDALPHA_BLEND(4, 16),
    BLDALPHA_BLEND(3, 16),
    BLDALPHA_BLEND(2, 16),
    BLDALPHA_BLEND(1, 16),
    BLDALPHA_BLEND(0, 16),
    [32 ... 63] = BLDALPHA_BLEND(0, 16)
};

static const struct OamData sVersionBannerLeftOamData =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_8BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sVersionBannerRightOamData =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_8BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sVersionBannerLeftAnimSequence[] =
{
    ANIMCMD_FRAME(0, 30),
    ANIMCMD_END,
};

static const union AnimCmd sVersionBannerRightAnimSequence[] =
{
    ANIMCMD_FRAME(VERSION_BANNER_RIGHT_TILEOFFSET, 30),
    ANIMCMD_END,
};

static const union AnimCmd *const sVersionBannerLeftAnimTable[] =
{
    sVersionBannerLeftAnimSequence,
};

static const union AnimCmd *const sVersionBannerRightAnimTable[] =
{
    sVersionBannerRightAnimSequence,
};

static const struct SpriteTemplate sVersionBannerLeftSpriteTemplate =
{
    .tileTag = TAG_VERSION,
    .paletteTag = TAG_VERSION,
    .oam = &sVersionBannerLeftOamData,
    .anims = sVersionBannerLeftAnimTable,
    .callback = SpriteCB_VersionBannerLeft,
};

static const struct SpriteTemplate sVersionBannerRightSpriteTemplate =
{
    .tileTag = TAG_VERSION,
    .paletteTag = TAG_VERSION,
    .oam = &sVersionBannerRightOamData,
    .anims = sVersionBannerRightAnimTable,
    .callback = SpriteCB_VersionBannerRight,
};

static const struct CompressedSpriteSheet sSpriteSheet_EmeraldVersion[] =
{
    {
        .data = gTitleScreenEmeraldVersionGfx,
        .size = 0x1000,
        .tag = TAG_VERSION
    },
    {},
};

static const struct OamData sOamData_CopyrightBanner =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sAnim_PressStart_0[] =
{
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_1[] =
{
    ANIMCMD_FRAME(5, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_2[] =
{
    ANIMCMD_FRAME(9, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_3[] =
{
    ANIMCMD_FRAME(13, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_PressStart_4[] =
{
    ANIMCMD_FRAME(17, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_0[] =
{
    ANIMCMD_FRAME(21, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_1[] =
{
    ANIMCMD_FRAME(25, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_2[] =
{
    ANIMCMD_FRAME(29, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_3[] =
{
    ANIMCMD_FRAME(33, 4),
    ANIMCMD_END,
};
static const union AnimCmd sAnim_Copyright_4[] =
{
    ANIMCMD_FRAME(37, 4),
    ANIMCMD_END,
};

// The "Press Start" and copyright graphics are each 5 32x8 segments long
#define NUM_PRESS_START_FRAMES 5
#define NUM_COPYRIGHT_FRAMES 5

static const union AnimCmd *const sStartCopyrightBannerAnimTable[NUM_PRESS_START_FRAMES + NUM_COPYRIGHT_FRAMES] =
{
    sAnim_PressStart_0,
    sAnim_PressStart_1,
    sAnim_PressStart_2,
    sAnim_PressStart_3,
    sAnim_PressStart_4,
    [NUM_PRESS_START_FRAMES] =
    sAnim_Copyright_0,
    sAnim_Copyright_1,
    sAnim_Copyright_2,
    sAnim_Copyright_3,
    sAnim_Copyright_4,
};

static const struct SpriteTemplate sStartCopyrightBannerSpriteTemplate =
{
    .tileTag = TAG_PRESS_START_COPYRIGHT,
    .paletteTag = TAG_PRESS_START_COPYRIGHT,
    .oam = &sOamData_CopyrightBanner,
    .anims = sStartCopyrightBannerAnimTable,
    .callback = SpriteCB_PressStartCopyrightBanner,
};

static const struct CompressedSpriteSheet sSpriteSheet_PressStart[] =
{
    {
        .data = gTitleScreenPressStartGfx,
        .size = 0x520,
        .tag = TAG_PRESS_START_COPYRIGHT
    },
    {},
};

static const struct SpritePalette sSpritePalette_PressStart[] =
{
    {
        .data = gTitleScreenPressStartPal,
        .tag = TAG_PRESS_START_COPYRIGHT
    },
    {},
};

static const struct OamData sPokemonLogoShineOamData =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sPokemonLogoShineAnimSequence[] =
{
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_END,
};

static const union AnimCmd *const sPokemonLogoShineAnimTable[] =
{
    sPokemonLogoShineAnimSequence,
};

static const struct SpriteTemplate sPokemonLogoShineSpriteTemplate =
{
    .tileTag = TAG_LOGO_SHINE,
    .paletteTag = TAG_PRESS_START_COPYRIGHT,
    .oam = &sPokemonLogoShineOamData,
    .anims = sPokemonLogoShineAnimTable,
    .callback = SpriteCB_PokemonLogoShine,
};

static const struct CompressedSpriteSheet sPokemonLogoShineSpriteSheet[] =
{
    {
        .data = sTitleScreenLogoShineGfx,
        .size = 0x800,
        .tag = TAG_LOGO_SHINE
    },
    {},
};

// Task data for the main title screen tasks (Task_TitleScreenPhase#)
#define tCounter    data[0]
#define tSkipToNext data[1]
#define tPointless  data[2] // Incremented but never used to do anything.
#define tBg2Y       data[3]
#define tBg1Y       data[4]

// Sprite data for sVersionBannerLeftSpriteTemplate / sVersionBannerRightSpriteTemplate
#define sAlphaBlendIdx data[0]
#define sParentTaskId  data[1]

static void SpriteCB_VersionBannerLeft(struct Sprite *sprite)
{
    if (gTasks[sprite->sParentTaskId].tSkipToNext)
    {
        sprite->oam.objMode = ST_OAM_OBJ_NORMAL;
        sprite->y = VERSION_BANNER_Y_GOAL;
    }
    else
    {
        if (sprite->y != VERSION_BANNER_Y_GOAL)
            sprite->y++;
        if (sprite->sAlphaBlendIdx != 0)
            sprite->sAlphaBlendIdx--;
        SetGpuReg(REG_OFFSET_BLDALPHA, gTitleScreenAlphaBlend[sprite->sAlphaBlendIdx]);
    }
}

static void SpriteCB_VersionBannerRight(struct Sprite *sprite)
{
    if (gTasks[sprite->sParentTaskId].tSkipToNext)
    {
        sprite->oam.objMode = ST_OAM_OBJ_NORMAL;
        sprite->y = VERSION_BANNER_Y_GOAL;
    }
    else
    {
        if (sprite->y != VERSION_BANNER_Y_GOAL)
            sprite->y++;
    }
}

// Sprite data for SpriteCB_PressStartCopyrightBanner
#define sAnimate data[0]
#define sTimer   data[1]

static void SpriteCB_PressStartCopyrightBanner(struct Sprite *sprite)
{
    if (sprite->sAnimate == TRUE)
    {
        // Alternate between hidden and shown every 16th frame
        if (++sprite->sTimer & 16)
            sprite->invisible = FALSE;
        else
            sprite->invisible = TRUE;
    }
    else
    {
        sprite->invisible = FALSE;
    }
}

static void CreatePressStartBanner(s16 x, s16 y)
{
    u8 i;
    u8 spriteId;

    x -= 64;
    for (i = 0; i < NUM_PRESS_START_FRAMES; i++, x += 32)
    {
        spriteId = CreateSprite(&sStartCopyrightBannerSpriteTemplate, x, y, 0);
        StartSpriteAnim(&gSprites[spriteId], i);
        gSprites[spriteId].sAnimate = TRUE;
    }
}

static void CreateCopyrightBanner(s16 x, s16 y)
{
    u8 i;
    u8 spriteId;

    x -= 64;
    for (i = 0; i < NUM_COPYRIGHT_FRAMES; i++, x += 32)
    {
        spriteId = CreateSprite(&sStartCopyrightBannerSpriteTemplate, x, y, 0);
        StartSpriteAnim(&gSprites[spriteId], i + NUM_PRESS_START_FRAMES);
        #if IS_HNS
        gSprites[spriteId].sAnimate = TRUE;
        #endif
    }
}

#undef sAnimate
#undef sTimer

// Defines for SpriteCB_PokemonLogoShine
enum {
    SHINE_MODE_SINGLE_NO_BG_COLOR,
    SHINE_MODE_DOUBLE,
    SHINE_MODE_SINGLE,
};

#define SHINE_SPEED  4

#define sMode     data[0]
#define sBgColor  data[1]

static void SpriteCB_PokemonLogoShine(struct Sprite *sprite)
{
    if (sprite->x < DISPLAY_WIDTH + 32)
    {
        // In any mode except SHINE_MODE_SINGLE_NO_BG_COLOR the background
        // color will change, in addition to the shine sprite moving.
        if (sprite->sMode != SHINE_MODE_SINGLE_NO_BG_COLOR)
        {
            u16 backgroundColor;

            if (sprite->x < DISPLAY_WIDTH / 2)
            {
                // Brighten background color
                if (sprite->sBgColor < 31)
                    sprite->sBgColor++;
                if (sprite->sBgColor < 31)
                    sprite->sBgColor++;
            }
            else
            {
                // Darken background color
                if (sprite->sBgColor != 0)
                    sprite->sBgColor--;
                if (sprite->sBgColor != 0)
                    sprite->sBgColor--;
            }

            backgroundColor = _RGB(sprite->sBgColor, sprite->sBgColor, sprite->sBgColor);

            // Flash the background for 4 frames of movement.
            // Otherwise use the updating color.
            if (sprite->x == DISPLAY_WIDTH / 2 + (3 * SHINE_SPEED)
             || sprite->x == DISPLAY_WIDTH / 2 + (4 * SHINE_SPEED)
             || sprite->x == DISPLAY_WIDTH / 2 + (5 * SHINE_SPEED)
             || sprite->x == DISPLAY_WIDTH / 2 + (6 * SHINE_SPEED))
                #if IS_HNS
                gPlttBufferFaded[0] = RGB(1, 1, 1);
                #else
                gPlttBufferFaded[0] = RGB(24, 31, 12);
                #endif
            else
                gPlttBufferFaded[0] = backgroundColor;
        }

        sprite->x += SHINE_SPEED;
    }
    else
    {
        // Sprite has moved fully offscreen
        gPlttBufferFaded[0] = RGB_BLACK;
        DestroySprite(sprite);
    }
}

static void SpriteCB_PokemonLogoShine_Fast(struct Sprite *sprite)
{
    if (sprite->x < DISPLAY_WIDTH + 32)
        sprite->x += SHINE_SPEED * 2;
    else
        DestroySprite(sprite);
}

static void StartPokemonLogoShine(u8 mode)
{
    u8 spriteId;

    switch (mode)
    {
    case SHINE_MODE_SINGLE_NO_BG_COLOR:
    case SHINE_MODE_SINGLE:
        // Create one regular shine sprite.
        // If mode is SHINE_MODE_SINGLE it will also change the background color.
        spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, 0, 68, 0);
        gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;
        gSprites[spriteId].sMode = mode;
        break;
    case SHINE_MODE_DOUBLE:
        // Create an invisible sprite with mode set to update the background color
        spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, 0, 68, 0);
        gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;
        gSprites[spriteId].sMode = mode;
        gSprites[spriteId].invisible = TRUE;

        // Create two faster shine sprites
        spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, 0, 68, 0);
        gSprites[spriteId].callback = SpriteCB_PokemonLogoShine_Fast;
        gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;

        spriteId = CreateSprite(&sPokemonLogoShineSpriteTemplate, -80, 68, 0);
        gSprites[spriteId].callback = SpriteCB_PokemonLogoShine_Fast;
        gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;
        break;
    }
}

#undef sMode
#undef sBgColor

static void VBlankCB(void)
{
    ScanlineEffect_InitHBlankDmaTransfer();
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
    SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);
}

void CB2_InitTitleScreen(void)
{
    if (IS_FRLG)
    {
        CB2_InitTitleScreenFrlg();
        return;
    }
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        *((u16 *)PLTT) = RGB_WHITE;
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BG2CNT, 0);
        SetGpuReg(REG_OFFSET_BG1CNT, 0);
        SetGpuReg(REG_OFFSET_BG0CNT, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        DmaFill16(3, 0, (void *)VRAM, VRAM_SIZE);
        DmaFill32(3, 0, (void *)OAM, OAM_SIZE);
        DmaFill16(3, 0, (void *)(PLTT + 2), PLTT_SIZE - 2);
        ResetPaletteFade();
        gMain.state = 1;
        break;
    case 1:
        // bg2
        DecompressDataWithHeaderVram(gTitleScreenPokemonLogoGfx, (void *)(BG_CHAR_ADDR(0)));
        #if IS_WAYFARER
        // Logo tiles occupy 0x0000-0x3fff. The 8bpp scene starts at 0x4000;
        // both maps live above its reserved 0xb000-byte tile region.
        DecompressDataWithHeaderVram(gTitleScreenPokemonLogoTilemap, (void *)(BG_SCREEN_ADDR(30)));
        LoadPalette(gTitleScreenBgPalettes, BG_PLTT_ID(0), 16 * PLTT_SIZE_4BPP);
        DecompressDataWithHeaderVram(sTitleScreenRayquazaGfx, (void *)(BG_CHAR_ADDR(1)));
        DecompressDataWithHeaderVram(sTitleScreenRayquazaTilemap, (void *)(BG_SCREEN_ADDR(31)));
        #else
        DecompressDataWithHeaderVram(gTitleScreenPokemonLogoTilemap, (void *)(BG_SCREEN_ADDR(9)));
        LoadPalette(gTitleScreenBgPalettes, BG_PLTT_ID(0), 15 * PLTT_SIZE_4BPP);
        // bg3
        DecompressDataWithHeaderVram(sTitleScreenRayquazaGfx, (void *)(BG_CHAR_ADDR(2)));
        DecompressDataWithHeaderVram(sTitleScreenRayquazaTilemap, (void *)(BG_SCREEN_ADDR(26)));
        #endif
        // bg1 - HnS doesn't use clouds
        #if !IS_HNS
        DecompressDataWithHeaderVram(sTitleScreenCloudsGfx, (void *)(BG_CHAR_ADDR(3)));
        DecompressDataWithHeaderVram(gTitleScreenCloudsTilemap, (void *)(BG_SCREEN_ADDR(27)));
        #endif
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 9;
        LoadCompressedSpriteSheet(&sSpriteSheet_EmeraldVersion[0]);
        LoadCompressedSpriteSheet(&sSpriteSheet_PressStart[0]);
        LoadCompressedSpriteSheet(&sPokemonLogoShineSpriteSheet[0]);
        LoadPalette(gTitleScreenEmeraldVersionPal, OBJ_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadSpritePalette(&sSpritePalette_PressStart[0]);
        gMain.state = 2;
        break;
    case 2:
    {
        u8 taskId = CreateTask(Task_TitleScreenPhase1, 0);

        gTasks[taskId].tCounter = 256;
        gTasks[taskId].tSkipToNext = FALSE;
        gTasks[taskId].tPointless = -16;
        gTasks[taskId].tBg2Y = -32;
        gMain.state = 3;
        break;
    }
    case 3:
        BeginNormalPaletteFade(PALETTES_ALL, 1, 16, 0, RGB_WHITEALPHA);
        SetVBlankCallback(VBlankCB);
        gMain.state = 4;
        break;
    case 4:
        PanFadeAndZoomScreen(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, 0x100, 0);
        SetGpuReg(REG_OFFSET_BG2X_L, -29 * 256);
        SetGpuReg(REG_OFFSET_BG2X_H, -1);
        SetGpuReg(REG_OFFSET_BG2Y_L, -32 * 256);
        SetGpuReg(REG_OFFSET_BG2Y_H, -1);
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN1H, 0);
        SetGpuReg(REG_OFFSET_WIN1V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN1_BG_ALL | WININ_WIN1_OBJ);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ | WINOUT_WINOBJ_ALL);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG2 | BLDCNT_EFFECT_LIGHTEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 12);
        #if IS_WAYFARER
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(3) | BGCNT_CHARBASE(1) | BGCNT_SCREENBASE(31) | BGCNT_256COLOR | BGCNT_TXT256x256);
        SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(1) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(30) | BGCNT_256COLOR | BGCNT_AFF256x256);
        #else
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(3) | BGCNT_CHARBASE(2) | BGCNT_SCREENBASE(26) | BGCNT_16COLOR | BGCNT_TXT256x256);
        SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(2) | BGCNT_CHARBASE(3) | BGCNT_SCREENBASE(27) | BGCNT_16COLOR | BGCNT_TXT256x256);
        SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(1) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(9) | BGCNT_256COLOR | BGCNT_AFF256x256);
        #endif
        EnableInterrupts(INTR_FLAG_VBLANK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_1
                                    | DISPCNT_OBJ_1D_MAP
                                    | DISPCNT_BG2_ON
                                    | DISPCNT_OBJ_ON
                                    | DISPCNT_WIN0_ON
                                    | DISPCNT_OBJWIN_ON);
        #if IS_HNS
        m4aSongNumStart(MUS_HG_TITLE, FlagGet(FLAG_SYS_GBS_ENABLED));
        #else
        m4aSongNumStart(MUS_TITLE, FlagGet(FLAG_SYS_GBS_ENABLED));
        #endif
        gMain.state = 5;
        break;
    case 5:
        if (!UpdatePaletteFade())
        {
            StartPokemonLogoShine(SHINE_MODE_SINGLE_NO_BG_COLOR);
            ScanlineEffect_InitWave(0, DISPLAY_HEIGHT, 4, 4, 0, SCANLINE_EFFECT_REG_BG1HOFS, TRUE);
            SetMainCallback2(MainCB2);
        }
        break;
    }
}

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

// Shine the Pokémon logo two more times, and fade in the version banner
static void Task_TitleScreenPhase1(u8 taskId)
{
    // Skip to next phase when A, B, Start, or Select is pressed
    if (JOY_NEW(A_B_START_SELECT) || gTasks[taskId].tSkipToNext)
    {
        gTasks[taskId].tSkipToNext = TRUE;
        gTasks[taskId].tCounter = 0;
    }

    if (gTasks[taskId].tCounter != 0)
    {
        u16 frameNum = gTasks[taskId].tCounter;
        if (frameNum == 176)
            StartPokemonLogoShine(SHINE_MODE_DOUBLE);
        else if (frameNum == 64)
            StartPokemonLogoShine(SHINE_MODE_SINGLE);

        gTasks[taskId].tCounter--;
    }
    else
    {
        u8 spriteId;

        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_1 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG2_ON | DISPCNT_OBJ_ON);
        SetGpuReg(REG_OFFSET_WININ, 0);
        SetGpuReg(REG_OFFSET_WINOUT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_OBJ | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(16, 0));
        SetGpuReg(REG_OFFSET_BLDY, 0);

        // Create left side of version banner
        spriteId = CreateSprite(&sVersionBannerLeftSpriteTemplate, VERSION_BANNER_LEFT_X, VERSION_BANNER_Y, 0);
        gSprites[spriteId].sAlphaBlendIdx = ARRAY_COUNT(gTitleScreenAlphaBlend);
        gSprites[spriteId].sParentTaskId = taskId;

        // Create right side of version banner
        spriteId = CreateSprite(&sVersionBannerRightSpriteTemplate, VERSION_BANNER_RIGHT_X, VERSION_BANNER_Y, 0);
        gSprites[spriteId].sParentTaskId = taskId;

        gTasks[taskId].tCounter = 144;
        gTasks[taskId].func = Task_TitleScreenPhase2;
    }
}

#undef sParentTaskId
#undef sAlphaBlendIdx

// Create "Press Start" and copyright banners, and slide Pokémon logo up
static void Task_TitleScreenPhase2(u8 taskId)
{
    u32 yPos;

    // Skip to next phase when A, B, Start, or Select is pressed
    if (JOY_NEW(A_B_START_SELECT) || gTasks[taskId].tSkipToNext)
    {
        gTasks[taskId].tSkipToNext = TRUE;
        gTasks[taskId].tCounter = 0;
    }

    if (gTasks[taskId].tCounter != 0)
    {
        gTasks[taskId].tCounter--;
    }
    else
    {
        gTasks[taskId].tSkipToNext = TRUE;
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG0 | BLDCNT_TGT2_BD);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(6, 15));
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_1
                                    | DISPCNT_OBJ_1D_MAP
                                    | DISPCNT_BG0_ON
                                    // Wayfarer reserves the unused cloud layer's VRAM for its 8bpp scene.
                                    #if !IS_WAYFARER
                                    | DISPCNT_BG1_ON
                                    #endif
                                    | DISPCNT_BG2_ON
                                    | DISPCNT_OBJ_ON);
        #if IS_HNS
        CreatePressStartBanner(START_BANNER_X, 138);
        #else
        CreatePressStartBanner(START_BANNER_X, 108);
        #endif
        CreateCopyrightBanner(START_BANNER_X, 148);
        gTasks[taskId].tBg1Y = 0;
        gTasks[taskId].func = Task_TitleScreenPhase3;
    }

    if (!(gTasks[taskId].tCounter & 3) && gTasks[taskId].tPointless != 0)
        gTasks[taskId].tPointless++;
    if (!(gTasks[taskId].tCounter & 1) && gTasks[taskId].tBg2Y != 0)
        gTasks[taskId].tBg2Y++;

    // Slide Pokémon logo up
    yPos = gTasks[taskId].tBg2Y * 256;
    SetGpuReg(REG_OFFSET_BG2Y_L, yPos);
    SetGpuReg(REG_OFFSET_BG2Y_H, yPos / 0x10000);

    gTasks[taskId].data[5] = 15; // Unused
    gTasks[taskId].data[6] = 6;  // Unused
}

// Show Rayquaza silhouette and process main title screen input
static void Task_TitleScreenPhase3(u8 taskId)
{
    if (JOY_NEW(A_BUTTON) || JOY_NEW(START_BUTTON))
    {
        FadeOutBGM(4);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITEALPHA);
        SetMainCallback2(CB2_GoToMainMenu);
    }
    else if (JOY_HELD(CLEAR_SAVE_BUTTON_COMBO) == CLEAR_SAVE_BUTTON_COMBO)
    {
        SetMainCallback2(CB2_GoToClearSaveDataScreen);
    }
    else if (JOY_HELD(RESET_RTC_BUTTON_COMBO) == RESET_RTC_BUTTON_COMBO
      && CanResetRTC() == TRUE)
    {
        FadeOutBGM(4);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        SetMainCallback2(CB2_GoToResetRtcScreen);
    }
#if ENABLE_BERRY_GLITCH_FIX_MULTIBOOT
    else if (JOY_HELD(BERRY_UPDATE_BUTTON_COMBO) == BERRY_UPDATE_BUTTON_COMBO)
    {
        FadeOutBGM(4);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        SetMainCallback2(CB2_GoToBerryFixScreen);
    }
#endif
    else
    {
        SetGpuReg(REG_OFFSET_BG2Y_L, 0);
        SetGpuReg(REG_OFFSET_BG2Y_H, 0);
        if (++gTasks[taskId].tCounter & 1)
        {
            gTasks[taskId].tBg1Y++;
            gBattle_BG1_Y = gTasks[taskId].tBg1Y / 2;
            gBattle_BG1_X = 0;
        }
        UpdateLegendaryMarkingColor(gTasks[taskId].tCounter);
        if ((gMPlayInfo_BGM.status & 0xFFFF) == 0)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITEALPHA);
            SetMainCallback2(CB2_GoToCopyrightScreen);
        }
    }
}

static void CB2_GoToMainMenu(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitMainMenu);
}

static void CB2_GoToCopyrightScreen(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitCopyrightScreenAfterTitleScreen);
}

static void CB2_GoToClearSaveDataScreen(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitClearSaveDataScreen);
}

static void CB2_GoToResetRtcScreen(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitResetRtcScreen);
}

#if ENABLE_BERRY_GLITCH_FIX_MULTIBOOT
static void CB2_GoToBerryFixScreen(void)
{
    if (!UpdatePaletteFade())
    {
        m4aMPlayAllStop();
        SetMainCallback2(CB2_InitBerryFixProgram);
    }
}
#endif

static void UpdateLegendaryMarkingColor(u8 frameNum)
{
#if IS_HNS
    return;
#endif
    if ((frameNum % 4) == 0) // Change color every 4th frame
    {
        s32 intensity = Cos(frameNum, Q_8_8(0.5)) + Q_8_8(0.5);
        u32 r = 31 - Q_8_8_TO_INT(intensity * 31);
        u32 g = 31 - Q_8_8_TO_INT(intensity * 22);
        u32 b = 12;

        u16 color = RGB(r, g, b);
        LoadPalette(&color, BG_PLTT_ID(14) + 15, sizeof(color));
   }
}

#endif // IS_WAYFARER
