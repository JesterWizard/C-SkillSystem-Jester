REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh
PNG2DMP := $(EA_DIR)/Tools/Png2Dmp

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I Source -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

SOURCES := Source/TextEngine.c Source/TalkOnIdle.c
OBJECTS := $(SOURCES:.c=.o)
RAM_SRC := Source/RamAlloc.s
RAM_OBJ := Source/RamAlloc.o
OUTPUT  := Source/TextEngine.lyn.event

BOX_PNGS := $(wildcard Data/BoxGfx/*.png)
BOX_DMPS := $(BOX_PNGS:.png=.dmp)

.PHONY: all clean

all: $(OUTPUT) $(BOX_DMPS)

$(OUTPUT): $(OBJECTS) $(RAM_OBJ) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECTS) $(RAM_OBJ) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

Source/%.o: Source/%.c Source/TextEngine.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(RAM_OBJ): $(RAM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

Data/BoxGfx/%.dmp: Data/BoxGfx/%.png
	@echo "[DMP]	$@"
	@$(PNG2DMP) $< --lz77 -o $@

clean:
	rm -f $(OBJECTS) $(RAM_OBJ) $(OUTPUT)
