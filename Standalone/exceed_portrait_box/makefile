REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
LYN := $(EA_DIR)/Tools/lyn

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I Source -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

SOURCE := Source/ExceedPortraitBox.c
OBJECT := Source/ExceedPortraitBox.o
OUTPUT := Source/ExceedPortraitBox.lyn.event

.PHONY: all clean

all: $(OUTPUT)

# Installer.event places the hooks, so lyn adds none (-nohook).
$(OUTPUT): $(OBJECT) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECT) $(FE8_REF) > $@

$(OBJECT): $(SOURCE) Source/ExceedPortraitBox.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECT) $(OUTPUT)
