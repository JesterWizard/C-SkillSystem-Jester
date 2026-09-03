REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE := Source/CheckBattleUnitLevelUp_Guaranteed.c
OBJECT := Source/CheckBattleUnitLevelUp_Guaranteed.o
OUTPUT := Source/CheckBattleUnitLevelUp_Guaranteed.lyn.event

.PHONY: all clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECT) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls $(OBJECT) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECT) $(OUTPUT)
