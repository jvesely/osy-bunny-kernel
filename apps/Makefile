#
# Bunny Kernel creation makefile
#
include ../Makefile.inc

### Dirs ###
BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
INC_DIR = ../shared

### Files ###
LIBRT_DIR  = ../librt/bin/
LDS_FILE   = $(SRC_DIR)/apps.lds

OBJS += $(shell find src -name "*.cpp" -o -name "*.S" -o -name "*.c")
TEST_DIRS = 

ifneq ($(USER_TEST),)
	CPPFLAGS  += -DUSER_TEST
	CFLAGS    += -DUSER_TEST
	TEST_DIRS += $(dir $(addprefix ../, $(USER_TEST)))
	VPATH     += $(TEST_DIRS)
	OBJS      += $(USER_TEST)
endif

# Define DEBUG_MUTEX for mutex5 test
#ifneq (, $(findstring mutex5,$(KERNEL_TEST)))
#	CPPFLAGS += -DDEBUG_MUTEX
#	CFLAGS   += -DDEBUG_MUTEX
#endif

OBJS := $(addsuffix .o, $(basename $(notdir $(OBJS))))
BINS := $(addprefix $(BIN_DIR)/, $(addsuffix .bin, $(basename $(OBJS))))

.PRECIOUS: $(OBJ_DIR)/%.o $(BIN_DIR)/%.raw

### Rules ###
.PHONY: apps
apps: $(BINS)

-include Makefile.depend
.SUFFIXES:


$(BIN_DIR)/%.bin : $(BIN_DIR)/%.raw
	@echo -e "$(COL_WHITE)Creating image... $(COL_GREEN)$@$(COL_END) <--$(COL_YELLOW) $^$(COL_END)"
	@$(OBJCOPY) -O binary  $^ $@

$(BIN_DIR)/%.raw : $(OBJ_DIR)/%.o $(LIBRT_DIR)/librt.a $(LDS_FILE)
	@echo -e "$(COL_WHITE)Linking image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(^)$(COL_END)"
	@$(LD) $(LDFLAGS) -T $(LDS_FILE) -Map $@.map -o $@ $(<) -L$(LIBRT_DIR) -lrt

$(OBJ_DIR)/%.o : %.cpp
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLUE)$< $(COL_END)"
	@$(CC) $(CPPFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.c
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_VIOLET)$< $(COL_END)"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.S
	@echo -e "$(COL_WHITE)Compiling $(COL_CYAN)$@: $(COL_BLACK)$< $(COL_END)"
	@$(CC) $(ASFLAGS) -c -o $@ $<




### Dependencies ###
DEPEND = find $(SRC_DIR) $(TEST_DIRS) -name "*.cpp" -o -name "*.c" | xargs $(CC) -I $(SRC_DIR) -I $(INC_DIR) -MM 2>/dev/null | sed "s%^\([^\ \t]\)%$(OBJ_DIR)/\1%"

.PHONY: depend
depend:
	@touch $(addprefix ../, $(USER_TEST)) /dev/null
	@$(DEPEND) > Makefile.depend
#	@echo DEPENDING $(KERNEL_TEST) "|" $(TEST_DIRS)"|" $(VPATH)"|" $(OBJS)
		


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
