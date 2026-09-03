ENTRY(Start)

gNumMusicPlayers = 4;
gMaxLines = 0;
gInitialMainCB2 = CB2_InitCopyrightScreenAfterBootup;

MEMORY
{
    EWRAM (rwx) : ORIGIN = 0x2000000, LENGTH = 256K
    IWRAM (rwx) : ORIGIN = 0x3000000, LENGTH = 32K
    ROM    (rx) : ORIGIN = 0x8000000, LENGTH = 32M
}

SECTIONS {


    .ewram ORIGIN(EWRAM) : AT (__ewram_lma)
    ALIGN(4)
    {
        __ewram_start = .;
        *(.ewram*)
        . = ALIGN(4);
        __ewram_end = .;
    } > EWRAM

    .ewram.sbss (NOLOAD) :
    ALIGN(4)
    {
        *.o(.sbss);
    } > EWRAM

    .iwram ORIGIN(IWRAM) : AT (__iwram_lma)
    ALIGN(4)
    {
        __iwram_start = .;
        *(.iwram*);
        . = ALIGN(4);
        __iwram_end = .;
    } > IWRAM

    .iwram.bss (NOLOAD) :
    ALIGN(4)
    {
        *.o(.bss*);
        *.o(common_data);
        src/*.o(COMMON);
        *libc.a:*.o(COMMON);
        *libnosys.a:*.o(COMMON);
    } > IWRAM

    /* BEGIN ROM DATA */

    __rom_start = ORIGIN(ROM);

    .text ORIGIN(ROM) :
    ALIGN(4)
    {
        __rom_category_code_start = .;
        src/rom_header.o(.text*);
        KEEP(*.o(.text.header_gf));
        KEEP(*.o(.text.header_rhh));
        src/crt0.o(.text);
        *libagbsyscall.a:*.o(.text*);
        *libgcc.a:*.o(.text*);
        *libc.a:*.o(.text*);
        *libnosys.a:*.o(.text*);
        asm/*.o(.text*);
        KEEP(*.o(.text.consts));
        *.o(.text*);
        *.o(.eh_frame);
        . = ALIGN(4);
    } > ROM =0

    script_data :
    ALIGN(4)
    {
        __rom_category_scripts_start = .;
        data/*.o(script_data);
        . = ALIGN(4);
    } > ROM =0

    /*
     * Keep these ROM budget sections contiguous.  rom_report.py verifies that
     * they partition [__rom_start, __rom_end) exactly.  The object selectors
     * cover regular builds; the named input-section selectors also preserve
     * useful categories when release LTO replaces original C object names.
     */
    .rom_maps_layouts :
    ALIGN(4)
    {
        __rom_category_maps_layouts_start = .;
        data/maps.o(.rodata*);
        data/map_events.o(.rodata*);
        . = ALIGN(4);
    } > ROM =0

    .rom_graphics :
    ALIGN(4)
    {
        __rom_category_graphics_start = .;
        src/graphics.o(.rodata*);
        src/tilesets.o(.rodata*);
        *(.rodata.*Gfx*);
        *(.rodata.*Graphics*);
        *(.rodata.*Tiles*);
        *(.rodata.*Tilemap*);
        *(.rodata.*Pal*);
        *(.rodata.*Palette*);
        *(.rodata.*Icon*);
        *(.rodata.*Image*);
        *(.rodata.*Sprite*);
        . = ALIGN(4);
    } > ROM =0

    .rom_audio :
    ALIGN(4)
    {
        __rom_category_audio_start = .;
        data/sound_data.o(.rodata*);
        sound/songs/*.o(.rodata*);
        . = ALIGN(4);
    } > ROM =0

    .rom_trainer_data :
    ALIGN(4)
    {
        __rom_category_trainer_data_start = .;
        src/data.o(.rodata*);
        *(.rodata.gTrainers*);
        *(.rodata.gTrainerParties*);
        *(.rodata.sParty*);
        *(.rodata.*TrainerMons*);
        *(.rodata.*TrainerMon*);
        . = ALIGN(4);
    } > ROM =0

    .rom_encounter_data :
    ALIGN(4)
    {
        __rom_category_encounter_data_start = .;
        src/wild_encounter.o(.rodata*);
        *(.rodata.gWildMon*);
        *(.rodata.s*LandMons*);
        *(.rodata.s*WaterMons*);
        *(.rodata.s*RockSmashMons*);
        *(.rodata.s*FishingMons*);
        *(.rodata.*WildEncounter*);
        . = ALIGN(4);
    } > ROM =0

    .rom_other :
    ALIGN(4)
    {
        __rom_category_other_start = .;
        *.o(.rodata*);
        *libgcc.a:*.o(.rodata*);
        *libc.a:*.o(.rodata*);
        *libc.a:*.o(.data*);
        . = ALIGN(4);
    } > ROM =0

    .data.iwram :
    ALIGN(4)
    {
        __iwram_lma = .;
        . = . + (__iwram_end - __iwram_start);
    } > ROM = 0

    .data.ewram :
    ALIGN(4)
    {
        __ewram_lma = .;
        . = . + (__ewram_end - __ewram_start);
    } > ROM = 0

    /*
     * Output sections can acquire stricter alignment from their input
     * sections.  Attribute each resulting inter-section pad to the category
     * before it so the report still forms an exact partition of used ROM.
     */
    __rom_category_code_end = ADDR(script_data);
    __rom_category_scripts_end = ADDR(.rom_maps_layouts);
    __rom_category_maps_layouts_end = ADDR(.rom_graphics);
    __rom_category_graphics_end = ADDR(.rom_audio);
    __rom_category_audio_end = ADDR(.rom_trainer_data);
    __rom_category_trainer_data_end = ADDR(.rom_encounter_data);
    __rom_category_encounter_data_end = ADDR(.rom_other);

    . = ALIGN(4);
    __rom_category_other_end = .;
    __rom_end = .;

    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */

    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }

    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }

    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }

    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }

    /* DWARF 5*/
    .debug_line_str     0 : { *(.debug_line_str) }

    /* Discard everything not specifically mentioned above. */
    /DISCARD/ :
    {
        *(*);
    }
}
