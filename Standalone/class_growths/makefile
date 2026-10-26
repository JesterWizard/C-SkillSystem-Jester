REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS  := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

C_OBJ   := Source/ClassGrowths.o
S_OBJ   := Source/RamAlloc.o
OUTPUT  := Source/ClassGrowths.lyn.event

.PHONY: all clean

all: $(OUTPUT)

$(OUTPUT): $(C_OBJ) $(S_OBJ) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(C_OBJ) $(S_OBJ) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

Source/%.o: Source/%.c
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

Source/%.o: Source/%.s
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

clean:
	rm -f $(C_OBJ) $(S_OBJ) $(OUTPUT)
