#
# Loader creation makefile
#
include ../Makefile.inc

### Dirs ###
BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
COMMON_DIR = ../shared

### Files ###

VPATH = $(shell find src -type d )
VPATH += COMMON_DIR

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
	@echo -e "$(COL_WHITE)Linking loader RAW image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(OBJ_DIR)/$(OBJ)$(COL_END)"
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
