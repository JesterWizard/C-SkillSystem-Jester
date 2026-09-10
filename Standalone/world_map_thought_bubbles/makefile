REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
LYN := $(EA_DIR)/Tools/lyn
PNG2DMP := $(EA_DIR)/Tools/Png2Dmp
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -I Source -Wall -Wextra -Werror -O2

SOURCE := Source/WorldMapThoughts.c
OBJECT := Source/WorldMapThoughts.o
OUTPUT := Source/WorldMapThoughts.lyn.event

GFX_PNGS := $(wildcard Gfx/*/*.png)
GFX_DMPS := $(GFX_PNGS:.png=.dmp)

.PHONY: all clean gfx

all: $(OUTPUT)

gfx: $(GFX_DMPS)

$(GFX_DMPS): %.dmp: %.png
	@echo "[GFX]	$@"
	@$(PNG2DMP) $< --lz77 -o $@

$(OUTPUT): $(OBJECT) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECT) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(OBJECT): $(SOURCE) Source/thought_bubbles.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECT) $(OUTPUT)
