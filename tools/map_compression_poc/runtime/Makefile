ROOT := $(abspath ../../..)
OUT := $(ROOT)/tools/map_compression_poc/artifacts/runtime
CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -mcpu=arm7tdmi -march=armv4t -mthumb -mthumb-interwork -mabi=apcs-gnu -Wa,-meabi=5 -O2 -ffreestanding -fno-builtin -fdata-sections -ffunction-sections -DRELEASE=1 -DNDEBUG -DMODERN=1 -DTESTING=0 -DPOKEMON_WAYFARER -DPOKEMON_HNS -iquote $(ROOT)/game/include -I$(OUT)/data
LDFLAGS := -nostdlib -Wl,-T,$(CURDIR)/linker.ld -Wl,--gc-sections

all: $(OUT)/map-compression-runtime.gba
$(OUT)/data/maps.s $(OUT)/data/identity.json $(OUT)/data/generated.h: generate_data.py
	python3 $<
$(OUT)/map-compression-runtime.gba: start.s harness.c fastlz_adapter.c compat.c $(ROOT)/game/src/malloc.c $(ROOT)/game/src/decompress_asm.s $(OUT)/data/maps.s $(OUT)/data/generated.h $(OUT)/data/legacy.inc linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUT)/map-compression-runtime.elf start.s harness.c fastlz_adapter.c compat.c $(ROOT)/game/src/malloc.c $(ROOT)/game/src/decompress_asm.s $(OUT)/data/maps.s -lgcc
	$(OBJCOPY) -O binary $(OUT)/map-compression-runtime.elf $@
	$(ROOT)/game/tools/gbafix/gbafix $@ -tMAPCOMPPOC -cMCPC -m01 --silent
	arm-none-eabi-size $(OUT)/map-compression-runtime.elf > $(OUT)/size.txt
	arm-none-eabi-nm -S --size-sort $(OUT)/map-compression-runtime.elf > $(OUT)/symbols.txt
$(OUT)/data/legacy.inc: extract_legacy.py $(ROOT)/game/src/fieldmap.c
	python3 extract_legacy.py
run: all
	$(ROOT)/game/tools/mgba/mgba-rom-test -S 3 -R r0 $(OUT)/map-compression-runtime.gba
