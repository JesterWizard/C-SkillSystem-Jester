REPO_ROOT := $(abspath ../../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I include -I $(LIB_DIR)/include -Wall -Wextra -Werror -O2

SOURCES := Source/HexCards.c Source/HexCardTypes.c Source/HexItem.c Source/HexCore.c Source/HexArt.c Source/HexScreen.c Source/HexPrep.c
OBJECTS := $(SOURCES:.c=.o)
RAM_SRC := Source/RamAlloc.s
RAM_OBJ := Source/RamAlloc.o
HEADERS := Source/Hex.h Source/HexCore.h ../Source/PrepMenuExt.h
OUTPUT  := Source/Hex.lyn.event

.PHONY: all clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) $(RAM_OBJ) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECTS) $(RAM_OBJ) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

Source/%.o: Source/%.c $(HEADERS)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(RAM_OBJ): $(RAM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

clean:
	rm -f $(OBJECTS) $(RAM_OBJ) $(OUTPUT)
