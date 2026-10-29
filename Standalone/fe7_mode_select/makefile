REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
LYN := $(EA_DIR)/Tools/lyn

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

SOURCE := Source/ModeSelect.c
OBJECT := Source/ModeSelect.o
OUTPUT := Source/ModeSelect.lyn.event

.PHONY: all clean

all: $(OUTPUT)

# -nohook: every hook is written by hand in Installer.event.
$(OUTPUT): $(OBJECT)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECT) $(FE8_REF) > $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECT) $(OUTPUT)
