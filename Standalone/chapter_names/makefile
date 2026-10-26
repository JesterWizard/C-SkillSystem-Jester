REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS      := $(DEVKITARM)/bin/arm-none-eabi-as
OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy
LYN     := $(EA_DIR)/Tools/lyn
PNG2DMP := $(EA_DIR)/Tools/Png2Dmp
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE := Source/SaveMenuChapterTitle.c
OBJECT := Source/SaveMenuChapterTitle.o
OUTPUT := Source/SaveMenuChapterTitle.lyn.event

ASM_SRC := Source/ChapterNames.s
ASM_OBJ := Source/ChapterNames.o
ASM_DMP := Source/ChapterNames.dmp

FONT_PNG := Data/font.png
FONT_DMP := Data/font.dmp

.PHONY: all clean

all: $(OUTPUT) $(ASM_DMP) $(FONT_DMP)

$(OUTPUT): $(OBJECT) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECT) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(ASM_DMP): $(ASM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) $< -o $(ASM_OBJ)
	@$(OBJCOPY) -O binary $(ASM_OBJ) $@
	@rm -f $(ASM_OBJ)

$(FONT_DMP): $(FONT_PNG)
	@echo "[DMP]	$@"
	@$(PNG2DMP) $< --lz77 -o $@

clean:
	rm -f $(OBJECT) $(ASM_OBJ) $(OUTPUT)
