REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o

CC  := $(DEVKITARM)/bin/arm-none-eabi-gcc
LYN := $(EA_DIR)/Tools/lyn

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -O2

SOURCE := Source/SendInventoryOnDeath.c
OBJECT := Source/SendInventoryOnDeath.o
OUTPUT := Source/SendInventoryOnDeath.lyn.event

.PHONY: all clean

all: $(OUTPUT)

# -nohook: the hooks are written by hand in Installer.event.
$(OUTPUT): $(OBJECT)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECT) $(FE8_REF) > $@

$(OBJECT): $(SOURCE)
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

clean:
	rm -f $(OBJECT) $(OUTPUT)
