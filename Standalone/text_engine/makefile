REPO_ROOT := $(abspath ../..)
LIB_DIR   := $(REPO_ROOT)/Tools/FE-CLib-Mokha
EA_DIR    := $(REPO_ROOT)/Tools/EventAssembler

FE8_REF := $(LIB_DIR)/reference/fireemblem8.ref.o
FE8_SYM := $(LIB_DIR)/reference/fireemblem8.sym

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
AS := $(DEVKITARM)/bin/arm-none-eabi-as
LYN := $(EA_DIR)/Tools/lyn
LYN_PROTECTOR := $(REPO_ROOT)/Tools/scripts/lynjump-protector.sh
PNG2DMP := $(EA_DIR)/Tools/Png2Dmp

ARCH   := -mcpu=arm7tdmi -mthumb -mthumb-interwork
CFLAGS := $(ARCH) -I Source -I $(LIB_DIR)/include -I $(REPO_ROOT)/include -Wall -Wextra -Werror -Wno-unused-parameter -O2 -mtune=arm7tdmi -std=gnu99

SOURCES := Source/TextEngine.c Source/TalkOnIdle.c
OBJECTS := $(SOURCES:.c=.o)
RAM_SRC := Source/RamAlloc.s
RAM_OBJ := Source/RamAlloc.o
OUTPUT  := Source/TextEngine.lyn.event

# Conversation log (TEXT_ENGINE_CHATLOG in Installer.event) and its no-op stand-in
CHATLOG_OBJ     := Source/Chatlog.o
CHATLOG_RAM_SRC := Source/ChatlogRamAlloc.s
CHATLOG_RAM_OBJ := Source/ChatlogRamAlloc.o
CHATLOG_OUTPUT  := Source/Chatlog.lyn.event
STUBS_OBJ       := Source/ChatlogStubs.o
STUBS_OUTPUT    := Source/ChatlogStubs.lyn.event

# Half-body / 32-colour portraits (TEXT_ENGINE_HALF_BODY_PORTRAITS in Installer.event),
# compiled from Standalone/portrait_formats, and their no-op stand-in
PORTRAITS_DIR     := ../portrait_formats/Source
PORTRAITS_OBJ     := Source/Portraits.o
PORTRAITS_RAM_OBJ := Source/PortraitsRamAlloc.o
PORTRAITS_OUTPUT  := Source/Portraits.lyn.event
PSTUBS_OBJ        := Source/PortraitStubs.o
PSTUBS_OUTPUT     := Source/PortraitStubs.lyn.event

BOX_PNGS := $(wildcard Data/BoxGfx/*.png)
BOX_DMPS := $(BOX_PNGS:.png=.dmp)

.PHONY: all clean

all: $(OUTPUT) $(CHATLOG_OUTPUT) $(STUBS_OUTPUT) $(PORTRAITS_OUTPUT) $(PSTUBS_OUTPUT) $(BOX_DMPS)

$(OUTPUT): $(OBJECTS) $(RAM_OBJ) $(FE8_REF) $(FE8_SYM)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(OBJECTS) $(RAM_OBJ) $(FE8_REF) > $@
	@$(LYN_PROTECTOR) $@ $(FE8_SYM) >> $@

$(CHATLOG_OUTPUT): $(CHATLOG_OBJ) $(CHATLOG_RAM_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(CHATLOG_OBJ) $(CHATLOG_RAM_OBJ) $(FE8_REF) > $@

$(STUBS_OUTPUT): $(STUBS_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(STUBS_OBJ) $(FE8_REF) > $@

$(PORTRAITS_OUTPUT): $(PORTRAITS_OBJ) $(PORTRAITS_RAM_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(PORTRAITS_OBJ) $(PORTRAITS_RAM_OBJ) $(FE8_REF) > $@

$(PSTUBS_OUTPUT): $(PSTUBS_OBJ) $(FE8_REF)
	@echo "[LYN]	$@"
	@$(LYN) -longcalls -nohook $(PSTUBS_OBJ) $(FE8_REF) > $@

# Same source as Standalone/portrait_formats, with face sprites drawn through
# TextEngine_PutFaceSprite so the screen effects move them.
$(PORTRAITS_OBJ): $(PORTRAITS_DIR)/Portraits.c $(PORTRAITS_DIR)/Portraits.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -DPORTRAITS_TEXT_ENGINE -mlong-calls -c $< -o $@

$(PORTRAITS_RAM_OBJ): $(PORTRAITS_DIR)/RamAlloc.s
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

Source/%.o: Source/%.c Source/TextEngine.h Source/Chatlog.h
	@echo "[CC ]	$@"
	@$(CC) $(CFLAGS) -mlong-calls -c $< -o $@

$(RAM_OBJ): $(RAM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

$(CHATLOG_RAM_OBJ): $(CHATLOG_RAM_SRC)
	@echo "[AS ]	$@"
	@$(AS) $(ARCH) -o $@ $<

Data/BoxGfx/%.dmp: Data/BoxGfx/%.png
	@echo "[DMP]	$@"
	@$(PNG2DMP) $< --lz77 -o $@

clean:
	rm -f $(OBJECTS) $(RAM_OBJ) $(OUTPUT) $(CHATLOG_OBJ) $(CHATLOG_RAM_OBJ) $(CHATLOG_OUTPUT) $(STUBS_OBJ) $(STUBS_OUTPUT) $(PORTRAITS_OBJ) $(PORTRAITS_RAM_OBJ) $(PORTRAITS_OUTPUT) $(PSTUBS_OBJ) $(PSTUBS_OUTPUT)
