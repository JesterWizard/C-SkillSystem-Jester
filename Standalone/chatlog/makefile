REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I Source -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

# Keep Chatlog.c first: lyn misplaces calls between global functions inside
# any later object, so Source/ChatlogTalk.c only calls its own static ones.
SOURCES := Source/Chatlog.c Source/ChatlogTalk.c
OBJECTS := $(SOURCES:.c=.o)
RAM_SRC := Source/ChatlogRamAlloc.s
RAM_OBJ := Source/ChatlogRamAlloc.o
OUTPUT  := Source/Chatlog.lyn.event

.PHONY: all clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) $(RAM_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECTS) $(RAM_OBJ) $(FE8_REF) > $@

Source/%.o: Source/%.c Source/Chatlog.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(RAM_OBJ): $(RAM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

clean:
	rm -f $(OBJECTS) $(RAM_OBJ) $(OUTPUT)
