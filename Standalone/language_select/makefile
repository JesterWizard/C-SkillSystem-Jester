REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS  := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh
PNG2DMP := $(EA_DIR)/Tools/Png2Dmp

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE  := Source/LanguageSelect.c
OBJECT  := Source/LanguageSelect.o
RAM_SRC := Source/RamAlloc.s
RAM_OBJ := Source/RamAlloc.o
OUTPUT  := Source/LanguageSelect.lyn.event

FLAGS_PNG := Gfx/Img_LanguageSelectFlags.png
FLAGS_DMP := Gfx/Img_LanguageSelectFlags.dmp

.PHONY: all clean

all: $(OUTPUT) $(FLAGS_DMP)

# -nohook: the GetStringFromIndex replacements are installed (or not) by
# Installer.event, never by lyn.
$(OUTPUT): $(OBJECT) $(RAM_OBJ) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECT) $(RAM_OBJ) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(RAM_OBJ): $(RAM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

$(FLAGS_DMP): $(FLAGS_PNG)
	@echo "[DMP]	$@"
	@$(PNG2DMP) $< --lz77 -o $@

clean:
	rm -f $(OBJECT) $(RAM_OBJ) $(OUTPUT) $(FLAGS_DMP)
