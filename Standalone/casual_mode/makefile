REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS  := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE  := Source/CasualMode.c
OBJECT  := Source/CasualMode.o
ASM_SRC := Source/CasualModeHook.s
ASM_OBJ := Source/CasualModeHook.o
OUTPUT  := Source/CasualMode.lyn.event

.PHONY: all clean

all: $(OUTPUT)

# -nohook: the hook is written by hand in Installer.event.
$(OUTPUT): $(OBJECT) $(ASM_OBJ)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECT) $(ASM_OBJ) $(FE8_REF) > $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(ASM_OBJ): $(ASM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

clean:
	rm -f $(OBJECT) $(ASM_OBJ) $(OUTPUT)
