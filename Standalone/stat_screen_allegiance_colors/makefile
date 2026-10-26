REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS  := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
PNG2DMP := $(EA_DIR)/Tools/Png2Dmp
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE  := Source/StatScreenAllegianceColors.c
OBJECT  := Source/StatScreenAllegianceColors.o
ASM_SRC := Source/StatScreenAllegianceHook.s
ASM_OBJ := Source/StatScreenAllegianceHook.o
OUTPUT  := Source/StatScreenAllegianceColors.lyn.event

PAL_PNGS := Gfx/SSBlue.png Gfx/SSRed.png Gfx/SSGreen.png Gfx/SSGold.png
PAL_DMPS := $(PAL_PNGS:.png=Pal.dmp)

.PHONY: all clean gfx
all: $(OUTPUT)

gfx: $(PAL_DMPS)

Gfx/%Pal.dmp: Gfx/%.png
	@echo "[PAL]	$@"
	@$(PNG2DMP) $< --palette-only -po $@ > /dev/null

$(OUTPUT): $(OBJECT) $(ASM_OBJ) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECT) $(ASM_OBJ) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(ASM_OBJ): $(ASM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

clean:
	rm -f $(OBJECT) $(ASM_OBJ) $(OUTPUT)
