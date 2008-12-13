#pragma once

#include "types.h"

#define VF_VA_SIZE    1
#define VF_VA_SHIFT   0
#define VF_VA_MASK    0x1
#define VF_VIRT_ADDR(flags) (((flags) & (VF_VA_MASK)) >> (VF_VA_SHIFT))

#define VF_VA_AUTO    0x0
#define VF_VA_USER    0x1

#define VF_AT_SIZE    3
#define VF_AT_SHIFT   1
#define VF_AT_MASK    (0x7 << 1)
#define VF_ADDR_TYPE(flags) (((flags) & (VF_AT_MASK)) >> (VF_AT_SHIFT))

#define VF_AT_KSEG0   0x0
#define VF_AT_KSEG1   0x1
#define VF_AT_KUSEG   0x2
#define VF_AT_KSSEG   0x3
#define VF_AT_KSEG3   0x4
#define TF_NEW_VMM    0x1
