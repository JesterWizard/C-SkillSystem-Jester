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
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE := Source/ResTerrainWindow.c
OBJECT := Source/ResTerrainWindow.o
OUTPUT := Source/ResTerrainWindow.lyn.event

FONT_PNG := Gfx/TerrainWindowFont.png
FONT_DMP := Gfx/TerrainWindowFont.dmp

.PHONY: all clean

all: $(OUTPUT) $(FONT_DMP)

$(OUTPUT): $(OBJECT) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECT) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(FONT_DMP): $(FONT_PNG)
	@echo "[DMP]	$@"
	@$(PNG2DMP) $< --lz77 -o $@

clean:
	rm -f $(OBJECT) $(OUTPUT)
