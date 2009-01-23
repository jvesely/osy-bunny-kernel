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
KERNEL_BIN = kernel.bin
KERNEL_RAW = kernel.raw
KERNEL_MAP = kernel.map
LDS_FILE   = kernel.lds

OBJS += $(shell find src ../shared -name "*.cpp" -o -name "*.S" -o -name "*.c")
TEST_DIRS = 

ifneq ($(KERNEL_TEST),)
	OBJS      += $(addprefix ../, $(KERNEL_TEST))
	CPPFLAGS  += -DKERNEL_TEST
	CFLAGS    += -DKERNEL_TEST
	TEST_DIRS += $(dir $(addprefix ../, $(KERNEL_TEST)))
	VPATH     += $(TEST_DIRS)
endif

ifneq ($(USER_TEST),)
	CPPFLAGS += -DUSER_TEST
	CFLAGS   += -DUSER_TEST
endif

# Define DEBUG_MUTEX for mutex5 test
ifneq (, $(findstring mutex5,$(KERNEL_TEST)))
	CPPFLAGS += -DDEBUG_MUTEX
	CFLAGS   += -DDEBUG_MUTEX
endif

### Dependencies ###
DEPEND = find $(SRC_DIR) $(TEST_DIRS) -name "*.cpp" -o -name "*.c" | xargs $(CC) -I $(SRC_DIR) -I $(INC_DIR) -MM 2>/dev/null | sed "s%^\([^\ \t]\)%$(OBJ_DIR)/\1%"

Makefile.depend: $(OBJS)
	@$(DEPEND) > Makefile.depend
#	@echo DEPENDING $(KERNEL_TEST) "|" $(TEST_DIRS)"|" $(VPATH)"|" $(OBJS)

OBJS := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(OBJS)))))


### Rules ###
kernel: $(BIN_DIR)/$(KERNEL_BIN)

$(BIN_DIR)/$(KERNEL_BIN): $(BIN_DIR)/$(KERNEL_RAW)
	@echo -e "$(COL_WHITE)Creating kernel.bin image... $(COL_GREEN)$@$(COL_END) <--$(COL_YELLOW) $^$(COL_END)"
	@$(OBJCOPY) -O binary  $^ $@

$(BIN_DIR)/$(KERNEL_RAW): Makefile.depend $(OBJS) $(INC_DIR)/$(LDS_FILE)
	@echo -e "$(COL_WHITE)Linking kernel RAW image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(OBJS)$(COL_END)"
	@$(LD) $(LDFLAGS) -T $(INC_DIR)/$(LDS_FILE) -Map $(BIN_DIR)/$(KERNEL_MAP) -o $@ $(OBJS)

-include Makefile.depend
.SUFFIXES:

.INTERMEDIATE: $(OBJ_DIR)/test.o $(OBJ_DIR)/first_thread.o

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
	@rm -vf $(BIN_DIR)/$(KERNEL_RAW)
	@rm -vf $(BIN_DIR)/$(KERNEL_MAP)
	@rm -vf $(OBJ_DIR)/*.o
	@echo -e "$(COL_WHITE)done$(COL_END)"

### Distcleaning ###
.PHONY: distclean
distclean: clean
	@echo -e "$(COL_WHITE)Cleaning bin files...$(COL_RED)"
	@rm -vf $(BIN_DIR)/$(KERNEL_BIN)
	@echo -e "$(COL_WHITE)done$(COL_END)"
