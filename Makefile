# Makefile for Mpic

all:

SRC_DIR := src
BUILD_DIR := build
$(BUILD_DIR):
	@mkdir -p $@

SRC = block.c mpic.c pic.c rgb.c state.c
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ := $(subst $(SRC_DIR), $(BUILD_DIR), $(SRC:.c=.o))

.PHONY: all run clean win

all: mpic

run: mpic
	./mpic

clean:
	@rm mpic
	@rm -rf $(BUILD_DIR)

mpic: $(OBJ)
	$(CC) -lSDL2 -lSDL2_image $^ -o $@

WINLIBS := libSDL2.a libSDL2_image.a
WINLIBS := $(addprefix third-party/lib/, $(WINLIBS))
WINEXTRA := kernel32 user32 gdi32 winmm imm32 ole32 oleaut32 version uuid \
	    advapi32 setupapi shell32
WINEXTRA := $(addprefix -l, $(WINEXTRA))

win: CC = x86_64-w64-mingw32-gcc
win: $(OBJ)
	$(CC) -mwindows -o mpic.exe $^ $(WINLIBS) $(WINEXTRA)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -O2 -c -MMD -Ithird-party/include $< -o $@
