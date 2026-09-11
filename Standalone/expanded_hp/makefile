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

OBJECTS := Source/ExpandedHp.o Source/ExpandedHpGauge.o
OUTPUT  := Source/ExpandedHp.lyn.event

PALETTES := Gfx/HPBarYellow.dmp Gfx/HPBarRed.dmp Gfx/HPBarBlue.dmp

.PHONY: all clean gfx

all: $(OUTPUT)

gfx: $(PALETTES)

Gfx/HPBar%.dmp: Gfx/HPBar%.png
	@echo "[GFX]	$@"
	@$(PNG2DMP) $< --palette-only -po $@ >/dev/null

$(OUTPUT): $(OBJECTS) $(FE8_REF) $(FE8_SYM) $(PALETTES)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECTS) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

Source/%.o: Source/%.c
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECTS) $(OUTPUT)
