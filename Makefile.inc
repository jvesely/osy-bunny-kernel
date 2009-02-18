#
# kernel, loader & librt global Makefile.inc
# sets compiler, linker,... variables, all directories and C/CPP FLAGS
# Defines colours
#

#use all dirs
VPATH    = $(shell find src ../shared -type d | grep -v \.svn)
INC_DIRS = . ../shared

CC      = $(shell which mipsel-linux-gnu-gcc     2> /dev/null)
OBJCOPY = $(shell which mipsel-linux-gnu-objcopy 2> /dev/null)
AS      = $(shell which mipsel-linux-gnu-as      2> /dev/null)
LD      = $(shell which mipsel-linux-gnu-ld      2> /dev/null)
AR      = $(shell which mipsel-linux-gnu-ar      2> /dev/null)

CPPFLAGS = -O2 -fno-optimize-sibling-calls -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -g3 -fno-rtti -fno-threadsafe-statics -nostartfiles -nostdlib -fno-exceptions $(addprefix -I , $(INC_DIRS))

CFLAGS =  -O2 -fno-optimize-sibling-calls -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 $(addprefix -I , $(INC_DIRS))

ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 $(addprefix -I , $(INC_DIRS)) -D__ASM__

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

