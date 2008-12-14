#
# Bunny Kernel creation makefile
#

#use all paths
VPATH = $(shell find src -type d | grep src | grep -v \.svn)
VPATH += ../common/

CC = $(shell which mipsel-linux-gnu-gcc)
OBJCOPY = $(shell which mipsel-linux-gnu-objcopy)
AS = $(shell which mipsel-linux-gnu-as)
LD = $(shell which mipsel-linux-gnu-ld)

CPPFLAGS = -O2 -fno-optimize-sibling-calls -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -g3 -fno-rtti -fno-threadsafe-statics -nostartfiles -nostdlib -fno-exceptions -I. -I ../common -I $(SRC_DIR)

CFLAGS =  -O2 -fno-optimize-sibling-calls -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -I $(SRC_DIR)

ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -I ../common -I $(SRC_DIR) -D__ASM__

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
INC_DIR = ../common

### Files ###
KERNEL_BIN = kernel.bin
KERNEL_RAW = kernel.raw
KERNEL_MAP = kernel.map
LDS_FILE = kernel.lds
OBJS += $(shell find src -name "*.cpp" -o -name "*.S" -o -name "*.c")
TEST_DIRS = 

ifneq ($(KERNEL_TEST),)

	OBJS += $(KERNEL_TEST)
	CPPFLAGS += -DKERNEL_TEST
	TEST_DIRS += $(dir $(addprefix ../, $(KERNEL_TEST)))
	VPATH += $(TEST_DIRS)
endif

# Define DEBUG_MUTEX for mutex5 test
ifneq (, $(findstring mutex5,$(KERNEL_TEST)))
	CPPFLAGS += -DDEBUG_MUTEX
	CFLAGS += -DDEBUG_MUTEX
endif

OBJS := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(OBJS)))))


### Rules ###
kernel: $(BIN_DIR)/$(KERNEL_BIN)

$(BIN_DIR)/$(KERNEL_BIN): $(BIN_DIR)/$(KERNEL_RAW)
	@echo -e "$(COL_WHITE)Creating kernel.bin image... $(COL_GREEN)$@$(COL_END) <--$(COL_YELLOW) $^$(COL_END)"
	@$(OBJCOPY) -O binary  $^ $@

$(BIN_DIR)/$(KERNEL_RAW): depend $(OBJS) $(INC_DIR)/$(LDS_FILE)
	@echo -e "$(COL_WHITE)Linking kernel RAW image...$(COL_YELLOW)$@$(COL_END) <-- $(COL_CYAN)$(OBJS)$(COL_END)"
	@$(LD) $(LDFLAGS) -T $(INC_DIR)/$(LDS_FILE) -Map $(BIN_DIR)/$(KERNEL_MAP) -o $@ $(OBJS)

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




### Dependencies ###
DEPEND = find $(SRC_DIR) $(TEST_DIRS) -name "*.cpp" -o -name "*.c" | xargs $(CC) -I $(SRC_DIR) -MM 2>/dev/null | sed "s%^\([^\ \t]\)%$(OBJ_DIR)/\1%"
.PHONY: depend
depend:
	@$(DEPEND) > Makefile.depend
	#echo DEPENDING $(KERNEL_TEST) "|" $(TEST_DIRS)"|" $(VPATH)"|" $(OBJS)
		


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
