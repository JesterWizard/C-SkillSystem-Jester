REPO_ROOT := $(abspath ../../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I include -I $(LIB_DIR)/include -Wall -Wextra -Werror -O2

SOURCES := Source/HexCards.c Source/HexCardTypes.c Source/HexItem.c
OBJECTS := $(SOURCES:.c=.o)
HEADER  := Source/Hex.h
OUTPUT  := Source/Hex.lyn.event

.PHONY: all clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECTS) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

Source/%.o: Source/%.c $(HEADER)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECTS) $(OUTPUT)
