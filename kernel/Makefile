#
# Bunny Kernel creation makefile
#
include ../Makefile.inc

### Dirs ###
BIN_DIR   = bin
OBJ_DIR   = obj
SRC_DIRS  = src ../shared
INC_DIRS += $(SRC_DIRS)

### Files ###
KERNEL_BIN = kernel.bin
KERNEL_RAW = kernel.raw
KERNEL_MAP = kernel.map
LDS_FILE   = ../shared/kernel.lds

TEST_DIRS = 

ifneq ($(KERNEL_TEST),)
	CPPFLAGS  += -DKERNEL_TEST
	CFLAGS    += -DKERNEL_TEST
	TEST_DIRS  = $(sort $(dir $(addprefix ../, $(KERNEL_TEST))))
	SRC_DIRS  += $(TEST_DIRS)
	VPATH     += $(TEST_DIRS)
endif

ifneq ($(USER_TEST),)
	CPPFLAGS += -DUSER_TEST
	CFLAGS   += -DUSER_TEST
endif

SRC_FILES += $(shell find $(SRC_DIRS) -name "*.cpp" -o -name "*.S" -o -name "*.c")

### Dependencies ###
DEPEND = $(CC) $(addprefix -I ,$(INC_DIRS)) -MM $(SRC_FILES)   2>/dev/null | sed "s%^\([^\ \t\n]*\.o\)%$(OBJ_DIR)/\1%"


Makefile.depend: $(OBJS)
	@$(DEPEND) > Makefile.depend
#	@echo DEPENDING $(KERNEL_TEST) "|" $(TEST_DIRS)"|" $(VPATH)"|" $(OBJS) "|" $(SRC_FILES)

OBJS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SRC_FILES)))))


### Rules ###
kernel: $(BIN_DIR)/$(KERNEL_BIN)

$(BIN_DIR)/$(KERNEL_BIN): $(BIN_DIR)/$(KERNEL_RAW)
	@echo -e "$(COL_WHITE)Creating kernel.bin image... $(COL_GREEN)$@$(COL_END) <--$(COL_YELLOW) $^$(COL_END)"
	@$(OBJCOPY) -O binary  $^ $@

$(BIN_DIR)/$(KERNEL_RAW): Makefile.depend $(OBJS) $(LDS_FILE)
	@echo -e "$(COL_WHITE)Linking kernel RAW image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(OBJS)$(COL_END)"
	@$(LD) $(LDFLAGS) -T $(LDS_FILE) -Map $(BIN_DIR)/$(KERNEL_MAP) -o $@ $(OBJS)

.PHONY: Makefile.depend

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
