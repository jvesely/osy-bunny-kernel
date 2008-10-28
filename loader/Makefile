#
# Loader creation makefile
#

CC = mipsel-linux-gnu-gcc
OBJCOPY = mipsel-linux-gnu-objcopy
LD = mipsel-linux-gnu-ld

CFLAGS = -O2 -fno-optimize-sibling-calls -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -fno-rtti -fno-threadsafe-statics

ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -D__ASM__

LDFLAGS = -G 0 -static -g

### Colours ###
COL_BLACK = \033[01;30m
COL_GREEN = \033[01;32m
COL_BLUE = \033[01;34m
COL_RED = \033[01;31m
COL_YELLOW = \033[01;33m
COL_VIOLET = \033[01;35m
COL_CYAN = \033[01;36m
COL_WHITE = \033[01;37m
COL_END = \033[0m

### Dirs ###
BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
COMMON_DIR = ../common

### Files ###

VPATH = $(shell find src -type d )
VPATH += ../common

LOADER_BIN = loader.bin
LOADER_RAW = loader.raw
LOADER_MAP = loader.map
LDS_FILE = kernel.lds
OBJ = loader.o

### Rules ###
loader: $(BIN_DIR)/$(LOADER_BIN)

.SUFFIXES:

$(BIN_DIR)/$(LOADER_BIN): $(BIN_DIR)/$(LOADER_RAW)
	@echo -e "$(COL_WHITE)Creating loader.bin image... $(COL_GREEN)$@$(COL_END) <--$(COL_YELLOW) $^$(COL_END)"
	@$(OBJCOPY) -O binary  $< $@

$(BIN_DIR)/$(LOADER_RAW): $(COMMON_DIR)/$(LDS_FILE) $(OBJ_DIR)/$(OBJ)
	@echo -e "$(COL_WHITE)Linking kernel RAW image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(OBJ_DIR)/$(OBJ)$(COL_END)"
	@$(LD) $(LD_FLAGS) -T $(COMMON_DIR)/$(LDS_FILE) -Map $(BIN_DIR)/$(LOADER_MAP) -o $@ $(OBJ_DIR)/$(OBJ)

$(OBJ_DIR)/%.o : %.S 
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLACK)$< $(COL_END)"
	@$(CC) $(ASFLAGS) -I $(COMMON_DIR) -c -o $@ $^



### Cleaning ###
.PHONY: clean
clean:
	@echo -e "$(COL_WHITE)Cleaning object files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/$(LOADER_MAP)
	@rm -vf $(BIN_DIR)/$(LOADER_RAW)
	@rm -vf $(OBJ_DIR)/*.o
	@echo -e "$(COL_WHITE)done$(COL_END)"

### Dist clean ###
.PHONY: distclean
distclean: clean
	@echo -e "$(COL_WHITE)Cleaning binary files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/$(LOADER_BIN)
	@echo -e "$(COL_WHITE)done$(COL_END)"

