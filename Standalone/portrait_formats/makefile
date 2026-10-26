REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS  := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
PYTHON ?= python3

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I Source -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

# One object per lyn file: lyn misplaces calls between global functions of
# the same object when that object isn't the first one it links.
PORTRAITS_OBJ    := Source/Portraits.o
PORTRAITS_OUTPUT := Source/Portraits.lyn.event
TALK_OBJ         := Source/VanillaTalk.o
TALK_OUTPUT      := Source/VanillaTalk.lyn.event
RAM_OBJ          := Source/RamAlloc.o

# Sample portraits, formatted with the tools in Tools/
SAMPLE_HALFBODY  := Samples/Anna_halfbody.dmp
SAMPLE_32COLOR   := Samples/Eirika_32color.dmp

.PHONY: all samples clean

all: $(PORTRAITS_OUTPUT) $(TALK_OUTPUT) samples

samples: $(SAMPLE_HALFBODY) $(SAMPLE_32COLOR)

$(PORTRAITS_OUTPUT): $(PORTRAITS_OBJ) $(RAM_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(PORTRAITS_OBJ) $(RAM_OBJ) $(FE8_REF) > $@

$(TALK_OUTPUT): $(TALK_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(TALK_OBJ) $(FE8_REF) > $@

Source/%.o: Source/%.c Source/Portraits.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(RAM_OBJ): Source/RamAlloc.s
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

Samples/%_halfbody.dmp: Samples/%_halfbody.png Tools/HalfbodyFormatter.py
	@echo "[HB ]	$@"
	@$(PYTHON) Tools/HalfbodyFormatter.py $< -o $@

Samples/%_32color.dmp: Samples/%_32color.png Tools/PortraitFormatter32.py
	@echo "[32C]	$@"
	@$(PYTHON) Tools/PortraitFormatter32.py $< -o $@

clean:
	rm -f $(PORTRAITS_OBJ) $(TALK_OBJ) $(RAM_OBJ) $(PORTRAITS_OUTPUT) $(TALK_OUTPUT)
