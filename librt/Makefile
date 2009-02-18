#
# Bunny Kernel creation makefile
#
include ../Makefile.inc
### Dirs ###
BIN_DIR   = bin
OBJ_DIR   = obj
SRC_DIRS  = src ../shared
INC_DIRS  += src

### Files ###
LIBRT_BIN = librt.a
SRC_FILES += $(shell find $(SRC_DIRS) -name "*.cpp" -o -name "*.S" -o -name "*.c")

### Dependencies ###
DEPEND = $(CC) $(addprefix -I ,$(INC_DIRS)) -MM  $(SRC_FILES)   2>/dev/null | sed "s%^\([^\ \t\n]*\.o\)%$(OBJ_DIR)/\1%"

Makefile.depend: $(OBJS)
	@$(DEPEND) > Makefile.depend
	@#echo DEPENDING $(KERNEL_TEST) "|" $(TEST_DIRS)"|" $(VPATH)"|" $(OBJS)

OBJS := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SRC_FILES)))))


### Rules ###
librt: $(BIN_DIR)/$(LIBRT_BIN)

$(BIN_DIR)/$(LIBRT_BIN): Makefile.depend $(OBJS) 
	@echo -e "$(COL_WHITE)Creating librt archive...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(OBJS)$(COL_END)"
	@$(AR) rcs $@ $(OBJS)

-include Makefile.depend
.SUFFIXES:

$(OBJ_DIR)/%.o : %.cpp
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLUE)$< $(COL_END)"
	@$(CC) $(CPPFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.c
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_VIOLET)$< $(COL_END)"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.S
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLACK)$< $(COL_END)"
	@$(CC) $(ASFLAGS) -c -o $@ $<

### Cleaning ###
.PHONY: clean
clean: 
	@echo -e "$(COL_WHITE)Cleaning object files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/$(LIBRT_BIN)
#	@rm -vf $(BIN_DIR)/$(KERNEL_MAP)
	@rm -vf $(OBJ_DIR)/*.o
	@echo -e "$(COL_WHITE)done$(COL_END)"

### Distcleaning ###
.PHONY: distclean
distclean: clean
	@echo -e "$(COL_WHITE)Cleaning bin files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/$(LIBRT_BIN)
	@echo -e "$(COL_WHITE)done$(COL_END)"
