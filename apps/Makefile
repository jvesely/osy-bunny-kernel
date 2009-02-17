#
# Bunny Kernel creation makefile
#
include ../Makefile.inc

### Dirs ###
BIN_DIR   = bin
OBJ_DIR   = obj
SRC_DIRS  = src
INC_DIRS += src

### Files ###
LIBRT_DIR  = ../librt/bin/
LDS_FILE   = $(shell find $(SRC_DIRS) -name apps.lds)


ifneq ($(USER_TEST),)
	CPPFLAGS  += -DUSER_TEST
	CFLAGS    += -DUSER_TEST
	TEST_DIRS  = $(dir $(addprefix ../, $(USER_TEST)))
	VPATH     += $(TEST_DIRS)
	SRC_DIRS  += $(TEST_DIRS)
	INC_DIRS  += $(TEST_DIRS)
endif

SRC_FILES  = $(shell find $(SRC_DIRS)  -name "*.cpp" -o -name "*.S" -o -name "*.c")
OBJS := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SRC_FILES)))))
BINS := $(addprefix $(BIN_DIR)/, $(addsuffix .bin, $(basename $(notdir $(SRC_FILES)))))

### Rules ###
.PHONY: apps
apps: Makefile.depend $(BINS) $(OBJS)

### Dependencies ###
DEPEND = find $(SRC_DIRS) -name "*.cpp" -o -name "*.c" | xargs $(CC) $(addprefix -I, $(INC_DIRS)) -MM 2>/dev/null | sed "s%^\([^\ \t]\)%$(OBJ_DIR)/\1%"


.PHONY: Makefile.depend
Makefile.depend: 
	@$(DEPEND) > Makefile.depend
#	@echo DEPENDING $(USER_TEST) "|" $(SRC_DIRS)"|" $(VPATH)"|" $(OBJS) "|" $(BINS) "|" $(SRC_FILES)
#	@cat Makefile.depend


-include Makefile.depend

.SUFFIXES: .c .o .raw

.PHONY: $(BIN_DIR)/%.bin
$(BIN_DIR)/%.bin : $(BIN_DIR)/%.raw
	@echo -e "$(COL_WHITE)Creating image... $(COL_GREEN)$@$(COL_END) <--$(COL_YELLOW) $^$(COL_END)"
	@$(OBJCOPY) -O binary  $< $@

$(BIN_DIR)/%.raw : $(OBJ_DIR)/%.o $(LIBRT_DIR)librt.a $(LDS_FILE)
	@echo -e "$(COL_WHITE)Linking image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(^)$(COL_END)"
	@$(LD) $(LDFLAGS) -T $(LDS_FILE) -Map $(basename $@).map -o $@ $(<) -L$(LIBRT_DIR) -lrt

$(OBJ_DIR)/%.o : %.cpp
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLUE)$< $(COL_END)"
	@$(CC) $(CPPFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.c
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_VIOLET)$< $(COL_END)"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.S
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLACK)$< $(COL_END)"
	@$(CC) $(ASFLAGS) -c -o $@ $<


.NOTPARALLEL: clean distclean
### Cleaning ###
.PHONY: clean
clean: 
	@echo -e "$(COL_WHITE)Cleaning files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/*.bin
	@rm -vf $(BIN_DIR)/*.raw
	@rm -vf $(BIN_DIR)/*.map
	@rm -vf $(OBJ_DIR)/*.o
	@echo -e "$(COL_WHITE)done$(COL_END)"

### Distcleaning ###
.PHONY: distclean
distclean: clean
	@echo -e "$(COL_WHITE)Cleaning bin files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/*.bin
	@echo -e "$(COL_WHITE)done$(COL_END)"
