#pragma once

#include "types.h"

const uint VF_VA_SIZE = 1;		// 1 bit
const uint VF_VA_SHIFT = 0;		// from 0-th bit
const uint VF_VA_MASK = 0x1;	// 0..00001
const uint VF_VIRT_ADDR = 0x1;

const uint VF_VA_AUTO = 0x0;
const uint VF_VA_USER = 0x1;

const uint VF_AT_SIZE = 3;		// 3 bits
const uint VF_AT_SHIFT = 1;		// from 1-st bit
const uint VF_AT_MASK = 0x7 << 1;	// 0..01110
const uint VF_ADDR_TYPE = 0x7 << 1;

const uint VF_AT_KSEG0 = 0x0;	// 0..00000
const uint VF_AT_KSEG1 = 0x1;	// 0..00010
const uint VF_AT_KUSEG = 0x2;	// 0..00100
const uint VF_AT_KSSEG = 0x3;	// 0..00110
const uint VF_AT_KSEG3 = 0x4;	// 0..01000
