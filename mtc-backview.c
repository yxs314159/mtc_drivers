#include <linux/delay.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include "mtc-car.h"

static struct i2c_driver mtc_backview_i2c_driver;

/*
 * =======================================
 *		Register Tables
 * =======================================
 */

static const u8 T132B_P2_S_MASK[] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const u8 T132B_P2_S_PAL[] = {
    0x32, 0xB,  0x4B, 2,    0xDD, 0x32, 0xA,  0x20, 0x60, 0x26, 0x60, 0,    0x80, 3, 0x37, 0x2C,
    0x2A, 0x89, 6,    0x82, 0x40, 0x64, 0x74, 0x87, 0x2A, 0xA,  0x7C, 0x1F, 0x20, 0, 0,    0};

static const u8 T132B_P2_S_NTSC[] = {
    0,    0xB,  0x4B, 0,    0xDD, 0x32, 0xA,  0x20, 0x60, 0x25, 0x70, 0,    0x80, 3, 0x37, 0x2C,
    0x2A, 0x89, 6,    0x82, 0x40, 0x64, 0x74, 0x87, 0x21, 0xF0, 0x7C, 0x1F, 0x20, 0, 0,    0};

static const u8 T132B_P2_S_PALM[] = {4, 0, 0, 0, 0, 0, 0, 0, 0,    0,    0,    0,    0, 0, 0, 0,
				     0, 0, 0, 0, 0, 0, 0, 0, 0x21, 0xE6, 0xEF, 0xA3, 0, 0, 0, 0};

static const u8 T132B_P2_S_PALN[] = {0x36, 0, 0, 0, 0, 0, 0, 0, 0,    0,    0,    0,    0, 0, 0, 0,
				     0,    0, 0, 0, 0, 0, 0, 0, 0x21, 0xF6, 0x94, 0x46, 0, 0, 0, 0};

static const u8 T132B_P2_S_SECAM[] = {0x38, 0, 0,    0,    0,    0,    0, 0, 0, 0, 0,
				      0,    0, 0,    0,    0,    0,    0, 0, 0, 0, 0,
				      0,    0, 0x28, 0xA3, 0x3B, 0xB2, 0, 0, 0, 0};

static const u8 T132B_P0_NTSC_YZ[] = {
    0,    0xE,  0,    0,    0x2F, 0,    0,    0x28, 0x28, 0x28, 0x60, 0x60, 0x60, 0x28, 0x48, 0x20,
    0x28, 0x98, 6,    8,    0xD1, 0x5A, 0xF3, 0x48, 0x15, 0x24, 2,    0,    0xF0, 0,    0xFF, 3,
    0,    0,    0,    0,    0,    0,    4,    0x40, 0,    0,    0,    0,    0,    0,    0,    0,
    0x82, 0x90, 0,    0,    0,    0xBC, 0x34, 0x40, 0,    0x10, 0,    0,    0,    0,    0x10, 0x1C,
    0xC0, 2,    0,    1,    0,    0x10, 1,    0,    0xD0, 2,    0xE0, 1,    0x10, 0,    5,    0,
    0,    0,    0,    0,    0xD4, 2,    0xF0, 0,    0x7E, 0xD6, 5,    1,    2,    0,    0,    0,
    0,    0x88, 4,    4,    3,    8,    8,    0,    0x80, 0x80, 0,    0x7F, 0x80, 0,    0xEB, 0,
    0x20, 0x1C, 0xC9, 0x1F, 0,    0x10, 0,    0,    0,    8,    0,    0,    0,    0x10, 0x80, 0x80,
    0,    0x14, 0x82, 0,    0xC7, 5,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    5,    4,    0x3B, 0,    0,    0x10, 0,    0x95, 0xCC, 0x64, 0xD0, 0x81, 1,    0x10, 0x80, 0x80,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x20, 0,    0x25, 0,    0xD0, 2,    0xE0, 1,    0x58, 3,    7,    1,    0x10, 0,    3,    0,
    1,    8,    0,    0,    0,    0x22, 0,    0,    0x2E, 1,    0x22, 2,    0,    0,    0,    3,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,
    0xB0, 0,    0x10, 0x80, 0,    0x80, 0,    0,    7,    0x80, 7,    0x80, 0,    0,    0x3C, 0,
    0x40, 4,    0,    0x11, 0x32, 0xAA, 0xD2, 0,    0,    0,    0,    0,    0xF,  7,    0,    0};

static const u8 T132B_P0_MASK[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const u8 T132B_P2_NTSC_YZ[] = {
    0,    0xB,  0x4B, 2,    0xDD, 0x32, 0xA,  0x20, 0x60, 0x25, 0x80, 0,    0x55, 3,    0x37, 0x2C,
    0x2A, 0x89, 6,    0x82, 0x40, 0x64, 0x74, 0x87, 0x21, 0xF0, 0x7C, 0x1F, 0x20, 0,    0,    0,
    0x3E, 0x3E, 0,    0x3E, 0xE9, 0xF,  0x2D, 0x50, 0x22, 0x4E, 0xD6, 0x4E, 0x23, 0x64, 0x81, 0x54,
    0x26, 0x61, 0x70, 0xE,  0x6C, 0x90, 0x70, 0xE,  0,    9,    0xE,  0,    2,    0x76, 0,    0,
    0x1C, 0x18, 0x10, 0x28, 0x28, 0x40, 0x20, 0x40, 0x40, 0x80, 0x33, 0x37, 0xA1, 0x2F, 0x68, 0x4B,
    0x87, 0xC1, 0xF0, 0x7C, 0xA8, 0x26, 0x2B, 0x2C, 0x48, 0x5B, 8,    0,    9,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0xB,  0,    0x10, 0x88, 0x82, 2,    0x10, 0xFC, 0x3E};

static const u8 T132B_P2_MASK[] = {
    1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0};

static const u8 T132B_P0_NTSC[] = {
    0,    0xE,  0,    0,    0x2F, 0,    0,    0x28, 0x28, 0x28, 0x80, 0x80, 0x80, 0x28, 0x48, 0x20,
    0x28, 0x98, 6,    8,    0xD1, 0x5A, 0xF3, 0x48, 0x15, 0x24, 2,    0,    0xF0, 0,    0xFF, 3,
    0,    0,    0,    0,    0,    0,    4,    0x40, 0,    0,    0,    0,    0,    0,    0,    0,
    0x82, 0x90, 0,    0,    0,    0xBC, 0x34, 0x40, 0,    0x10, 0,    0,    0,    0,    0x10, 0x1C,
    0xC0, 2,    0,    1,    0,    0x10, 1,    0,    0xD0, 2,    0xE0, 1,    0x10, 0,    5,    0,
    0,    0,    0,    0,    0xD4, 2,    0xF0, 0,    0x7E, 0xD6, 5,    1,    2,    0,    0,    0,
    0,    0x88, 4,    4,    3,    8,    8,    0,    0x80, 0x80, 0,    0x7F, 0x80, 0,    0xEB, 0,
    0x20, 0x1C, 0xC9, 0x1F, 0,    0x10, 0,    0,    0,    8,    0,    0,    0,    0x10, 0x80, 0x80,
    0,    0x14, 0x82, 0,    0xC7, 5,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    4,    4,    0x3B, 0,    0,    0x10, 0,    0x95, 0xCC, 0x64, 0xD0, 0x81, 0,    0x10, 0x80, 0x80,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x20, 0,    0x25, 0,    0xD0, 2,    0xE0, 1,    0x58, 3,    7,    1,    0x10, 0,    3,    0,
    1,    8,    0,    0,    0,    0x22, 0,    0,    0x2E, 1,    0x22, 2,    0,    0,    0,    3,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,
    0xB0, 0,    0x10, 0x80, 0,    0x80, 0,    0,    7,    0x80, 7,    0x80, 0,    0,    0x3C, 0,
    0x40, 4,    0,    0x11, 0x32, 0xAA, 0xD2, 0,    0,    0,    0,    0,    0xF,  7,    0,    0};

static const u8 T132B_P2_NTSC[] = {
    0,    0xB,  0x4B, 0,    0xDD, 0x32, 0xA,  0x20, 0x60, 0x25, 0x80, 0,    0x80, 3,    0x37, 0x2C,
    0x2A, 0x89, 6,    0x82, 0x40, 0x64, 0x74, 0x87, 0x21, 0xF0, 0x7C, 0x1F, 0x20, 0,    0,    0,
    0x3E, 0x3E, 0,    0x3E, 0xE9, 0xF,  0x2D, 0x50, 0x22, 0x4E, 0xD6, 0x4E, 0x23, 0x64, 0x81, 0x54,
    0x26, 0x61, 0x70, 0xE,  0x6C, 0x90, 0x70, 0xE,  0,    9,    0xE,  0,    2,    0x76, 0,    0,
    0x1C, 0x18, 0x10, 0x28, 0x28, 0x40, 0x20, 0x40, 0x40, 0x80, 0x33, 0x37, 0xA1, 0x2F, 0x68, 0x4B,
    0x87, 0xC1, 0xF0, 0x7C, 0xA8, 0x26, 0x2B, 0x2C, 0x48, 0x5B, 8,    0,    9,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0xB,  0,    0x10, 0x88, 0x82, 2,    0x10, 0xFC, 0x3E};

static const u8 T132B_P0_PAL[] = {
    0,    0xE,  0,    0,    0x2F, 0,    0,    0x28, 0x28, 0x28, 0x80, 0x80, 0x80, 0x28, 0x3C, 0x20,
    0x28, 0x98, 6,    8,    0xD1, 0x5A, 0xF3, 0x48, 0x15, 0x24, 2,    0,    0xF0, 0,    0xFF, 3,
    0,    0,    0,    0,    0,    0,    4,    0x40, 0,    0,    0,    0,    0,    0,    0,    0,
    0x82, 0x90, 0,    0,    0,    0xBC, 0x34, 0x40, 0,    0x10, 0,    0,    0,    0,    0x10, 0x1C,
    0xC0, 2,    0,    1,    0,    0x10, 1,    0,    0xD0, 2,    0xE0, 1,    0x10, 0,    5,    0,
    0,    0,    0,    0,    0xD6, 2,    0x22, 1,    0xFE, 0xD7, 0x38, 1,    2,    0,    0,    0,
    1,    0x88, 4,    4,    3,    8,    0x88, 0,    0x80, 0x80, 0,    0x7F, 0x80, 0,    0xEB, 0,
    0x20, 0xF,  0xC9, 0x1F, 0x21, 0x13, 0,    0,    0,    8,    0,    0,    0,    0x10, 0x80, 0x80,
    0,    0,    0,    0,    0xC7, 0x18, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    4,    4,    0x3B, 0,    0,    0x10, 0,    0x95, 0xCC, 0x64, 0xD0, 0x81, 0,    0x10, 0x80, 0x80,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x20, 0,    0x23, 0,    0xD0, 2,    0xE0, 1,    0x58, 3,    7,    1,    0x10, 0,    3,    0,
    1,    8,    0,    0,    0,    0x22, 0,    0,    0x26, 1,    0x22, 2,    0,    0,    0,    3,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,
    0xB0, 0,    0x10, 0x80, 0,    0x80, 0,    0,    7,    0x80, 7,    0x80, 0,    0,    0x3C, 0,
    0x40, 4,    0,    0x11, 0x32, 0xAA, 0xD2, 0,    0,    0,    0,    0,    0xF,  7,    0,    0};

static const u8 T132B_P2_PAL[] = {
    0x32, 0xB,  0x4B, 0xA,  0xDD, 0x32, 0xA,  0x20, 0x60, 0x26, 0x70, 0,    0x80, 3,    0x37, 0x2C,
    0x2A, 0x89, 6,    0x82, 0x40, 0x64, 0x74, 0x87, 0x2A, 0xA,  0x7C, 0x1F, 0x20, 0,    0,    0,
    0x41, 0x3E, 0,    0x41, 0xE9, 0xF,  0x2D, 0x50, 0x22, 0x4E, 0xD6, 0x4E, 0x23, 0x64, 0x88, 0x56,
    0x2D, 0xC5, 0x70, 0xE,  0x6C, 0x90, 0x70, 0xE,  0,    9,    0xE,  0,    0x17, 0x75, 0,    0,
    0x1C, 0x29, 0x10, 0x28, 0x28, 0x40, 0x20, 0x40, 0x40, 0x80, 0x33, 0x37, 0xA1, 0x2F, 0x68, 0x4B,
    0x87, 0xC1, 0xF0, 0x7C, 0xA8, 0x26, 0x2B, 0x2C, 0x48, 0x5B, 8,    0,    9,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0xB,  0,    0x10, 0x88, 0x82, 2,    0x10, 0xFC, 0x3E};

static const u8 ADV7181D_YUV_BACK[] = {
    0x42, 5,    1,    0x42, 6,    6,    0x42, 0xC3, 0x31, 0x42, 0xC4, 0xF2, 0x42, 0x1D, 0x47,
    0x42, 0xF,  2,    0x42, 0x3A, 0x11, 0x42, 0x3B, 0x81, 0x42, 0x3C, 0x3B, 0x42, 0x6B, 0x83,
    0x42, 0xC9, 0,    0x42, 0x73, 0xCF, 0x42, 0x74, 0xA3, 0x42, 0x75, 0xE8, 0x42, 0x76, 0xFA,
    0x42, 0x7B, 0x1C, 0x42, 0x7C, 0x90, 0x42, 0x85, 0x19, 0x42, 0x86, 0xB,  0x42, 0xBF, 6,
    0x42, 0xC0, 0x40, 0x42, 0xC1, 0xF0, 0x42, 0xC2, 0x80, 0x42, 0xC5, 1,    0x42, 0xC9, 8,
    0x42, 0xE,  0x80, 0x42, 0x52, 0x46, 0x42, 0x54, 0x80, 0x42, 0x57, 1,    0x42, 0xF6, 0x3B,
    0x42, 0xE,  0,    0x42, 0x67, 0x2F, 0xFF, 0,    0,    0};

static const u8 ADV7181D_RGB_HACT[] = {
    0x42, 5,    1,    0x42, 6,    6,    0x42, 0xC3, 0x46, 0x42, 0xC4, 0xB5, 0x42, 0x1D, 0x47, 0x42,
    0xF,  2,    0x42, 0x3A, 0x11, 0x42, 0x3B, 0x81, 0x42, 0x3C, 0x3B, 0x42, 0x6B, 0x83, 0x42, 0xC9,
    0,    0x42, 0x73, 0xD0, 0x42, 0x74, 4,    0x42, 0x75, 1,    0x42, 0x76, 0,    0x42, 0x77, 4,
    0x42, 0x78, 8,    0x42, 0x79, 2,    0x42, 0x7A, 0,    0x42, 0x7B, 0x1C, 0x42, 0x7C, 0x90, 0x42,
    0x52, 0,    0x42, 0x53, 0,    0x42, 0x54, 7,    0x42, 0x55, 0xC,  0x42, 0x56, 0x94, 0x42, 0x57,
    0x89, 0x42, 0x58, 0x48, 0x42, 0x59, 8,    0x42, 0x5A, 0,    0x42, 0x5B, 0x20, 0x42, 0x5C, 3,
    0x42, 0x5D, 0xA9, 0x42, 0x5E, 0x1A, 0x42, 0x6F, 0xB8, 0x42, 0x60, 8,    0x42, 0x61, 0,    0x42,
    0x62, 0x7A, 0x42, 0x63, 0xE1, 0x42, 0x64, 0,    0x42, 0x65, 0x19, 0x42, 0x66, 0x48, 0x42, 0xEE,
    0x80, 0x42, 0x85, 0xE9, 0x42, 0x86, 0xB,  0x42, 0xBF, 6,    0x42, 0xC0, 0x40, 0x42, 0xC1, 0xF0,
    0x42, 0xC2, 0x80, 0x42, 0xC5, 1,    0x42, 0xC9, 8,    0x42, 0xE,  0x80, 0x42, 0x52, 0x46, 0x42,
    0x54, 0x80, 0x42, 0x57, 1,    0x42, 0xF6, 0x3B, 0x42, 0xE,  0,    0x42, 0x67, 0x2F, 0xFF, 0};

static const u8 ADV7181D_RGB[] = {
    0x42, 5,    1,    0x42, 6,    6,    0x42, 0xC3, 0x46, 0x42, 0xC4, 0xB5, 0x42, 0x1D, 0x47, 0x42,
    0xF,  2,    0x42, 0x3A, 0x11, 0x42, 0x3B, 0x81, 0x42, 0x3C, 0x3B, 0x42, 0x6B, 0x83, 0x42, 0xC9,
    0,    0x42, 0x73, 0xD0, 0x42, 0x74, 4,    0x42, 0x75, 1,    0x42, 0x76, 0,    0x42, 0x77, 4,
    0x42, 0x78, 8,    0x42, 0x79, 2,    0x42, 0x7A, 0,    0x42, 0x7B, 0x1C, 0x42, 0x7C, 0x90, 0x42,
    0x52, 0,    0x42, 0x53, 0,    0x42, 0x54, 7,    0x42, 0x55, 0xC,  0x42, 0x56, 0x94, 0x42, 0x57,
    0x89, 0x42, 0x58, 0x48, 0x42, 0x59, 8,    0x42, 0x5A, 0,    0x42, 0x5B, 0x20, 0x42, 0x5C, 3,
    0x42, 0x5D, 0xA9, 0x42, 0x5E, 0x1A, 0x42, 0x6F, 0xB8, 0x42, 0x60, 8,    0x42, 0x61, 0,    0x42,
    0x62, 0x7A, 0x42, 0x63, 0xE1, 0x42, 0x64, 0,    0x42, 0x65, 0x19, 0x42, 0x66, 0x48, 0x42, 0xEE,
    0x80, 0x42, 0x85, 9,    0x42, 0x86, 0xB,  0x42, 0xBF, 6,    0x42, 0xC0, 0x40, 0x42, 0xC1, 0xF0,
    0x42, 0xC2, 0x80, 0x42, 0xC5, 1,    0x42, 0xC9, 8,    0x42, 0xE,  0x80, 0x42, 0x52, 0x46, 0x42,
    0x54, 0x80, 0x42, 0x57, 1,    0x42, 0xF6, 0x3B, 0x42, 0xE,  0,    0x42, 0x67, 0x2F, 0xFF, 0};

static const u8 ADV7181D_YUV[] = {
    0x42, 5,    1,    0x42, 6,    6,    0x42, 0xC3, 0x56, 0x42, 0xC4, 0xB4, 0x42, 0x1D, 0x47,
    0x42, 0xF,  2,    0x42, 0x3A, 0x11, 0x42, 0x3B, 0x81, 0x42, 0x3C, 0x3B, 0x42, 0x6B, 0x83,
    0x42, 0xC9, 0,    0x42, 0x73, 0xCF, 0x42, 0x74, 0xA3, 0x42, 0x75, 0xE8, 0x42, 0x76, 0xFA,
    0x42, 0x7B, 0x1C, 0x42, 0x7C, 0x90, 0x42, 0x85, 0x19, 0x42, 0x86, 0xB,  0x42, 0xBF, 6,
    0x42, 0xC0, 0x40, 0x42, 0xC1, 0xF0, 0x42, 0xC2, 0x80, 0x42, 0xC5, 1,    0x42, 0xC9, 8,
    0x42, 0xE,  0x80, 0x42, 0x52, 0x46, 0x42, 0x54, 0x80, 0x42, 0x57, 1,    0x42, 0xF6, 0x3B,
    0x42, 0xE,  0,    0x42, 0x67, 0x2F, 0xFF, 0,    0,    0};

static const u8 ADV7181D_CVBS[] = {
    0x42, 0,    0xB,  0x42, 3,    0xC,  0x42, 4,    0x75, 0x42, 0xC,  2,    0x42, 0xD,  0x88, 0x42,
    0x17, 0x41, 0x42, 0x1D, 0x47, 0x42, 0x31, 0x12, 0x42, 0x32, 0xC1, 0x42, 0x33, 4,    0x42, 0x34,
    1,    0x42, 0x35, 0xF0, 0x42, 0x36, 8,    0x42, 0x37, 0x81, 0x42, 0xF,  0xA,  0x42, 0x3A, 0x17,
    0x42, 0x3B, 0x81, 0x42, 0x3D, 0xA2, 0x42, 0x3E, 0x6A, 0x42, 0x3F, 0xA0, 0x42, 0x86, 0xB,  0x42,
    0xF3, 1,    0x42, 0xF9, 3,    0x42, 0x6B, 0x80, 0x42, 0xE,  0x80, 0x42, 0x52, 0x46, 0x42, 0x54,
    0x80, 0x42, 0x7F, 0xFF, 0x42, 0x81, 0x30, 0x42, 0x90, 0xC9, 0x42, 0x91, 0x40, 0x42, 0x92, 0x3C,
    0x42, 0x93, 0xCA, 0x42, 0x94, 0xD5, 0x42, 0xB1, 0xFF, 0x42, 0xB6, 8,    0x42, 0xC0, 0x9A, 0x42,
    0xCF, 0x50, 0x42, 0xD0, 0x4E, 0x42, 0xD1, 0xB9, 0x42, 0xD6, 0xDD, 0x42, 0xD7, 0xE2, 0x42, 0xE5,
    0x51, 0x42, 0xF6, 0x3B, 0x42, 0xE,  0,    0xFF};

static const u8 T132B_INIT[] = {
    0x40, 0xF,  0x70, 0x40, 0x16, 0xF3, 0x40, 0x18, 0x15, 0x40, 0x19, 0x24, 0x40, 0x1A, 0,
    0x40, 0x31, 0x90, 0x40, 0x61, 0x88, 0x40, 0x64, 4,    0x40, 0x90, 4,    0x40, 0xB0, 0x20,
    0x40, 0xB1, 0,    0x40, 0xB2, 0x10, 0x40, 0xB3, 0,    0x40, 0xB4, 0xD0, 0x40, 0xB5, 2,
    0x40, 0xB6, 0xE0, 0x40, 0xB7, 1,    0x40, 0xB8, 0x58, 0x40, 0xB9, 3,    0x40, 0xBE, 3,
    0x40, 0xC9, 1,    0x40, 0xCA, 0x22, 0x40, 0xCF, 3,    0x40, 0xDC, 0xD0, 0x40, 0xDD, 2,
    0x40, 0xDE, 0xE0, 0x40, 0xDF, 1,    0x40, 0xE0, 0xB0, 0x40, 0xE3, 0x80, 0x40, 0xFC, 0xF,
    0x40, 0xFD, 7,    0x42, 0xE5, 0,    0x46, 0x1B, 0x49, 0x46, 0x1C, 0x80, 0xFF};

static const u8 T132B_P0_RGB[] = {
    0xE,  0xE,  0xE,  0,    0x2F, 0,    0,    0x36, 0x36, 0x36, 0x90, 0x88, 0x90, 0x28, 0xA0, 0xF4,
    9,    0x98, 6,    8,    0xF0, 0x20, 0x83, 0x38, 0,    0x24, 0,    0x9F, 0xF0, 0,    0xFF, 3,
    0xD8, 0,    0,    0,    0,    0,    4,    0x40, 0,    0,    0,    0,    0,    0,    0,    0,
    0x82, 0x30, 0,    0,    0,    0xBC, 0x34, 0x40, 0,    0x10, 0,    0,    0,    0,    0x10, 0x1C,
    0xC2, 0x50, 0,    0,    0,    0,    6,    0,    0xD0, 2,    0xE0, 1,    0x20, 0,    0x1E, 0,
    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,    0x63, 0x6B, 0xD,  2,    2,    0,    3,    0,
    0,    0xC0, 0x10, 0x10, 2,    8,    8,    0,    0x80, 0x80, 0,    0x7F, 0xA5, 0,    0xEB, 0,
    0xB0, 0x13, 0x46, 0x1E, 0x90, 0x20, 0,    0,    0,    8,    0,    0,    0,    0x10, 0x80, 0x80,
    0,    0xF8, 0x75, 0,    0x15, 0x20, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    4,    4,    0x3B, 0,    0,    0x10, 0,    0x95, 0xCC, 0x64, 0xD0, 0x81, 0,    0x10, 0x80, 0x80,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x20, 0,    0x16, 0,    0xD0, 2,    0xE0, 1,    0x1F, 3,    6,    1,    8,    0,    6,    0,
    1,    8,    0,    0,    0,    0x22, 0,    0,    0x2A, 1,    0x22, 2,    0,    0,    0,    3,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,
    0xB0, 0,    0x10, 0x80, 0,    0x80, 0,    0,    7,    0x80, 7,    0x80, 0,    0,    0x3C, 0,
    0x40, 4,    0,    0x11, 0x32, 0xAA, 0xD2, 0,    0,    0,    0,    0,    0xF,  7,    0,    0};

static const u8 T132B_P0_MASK_RGB[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const u8 T132B_P0_YUV[] = {
    0xE,  0xE,  0xE,  1,    0x2F, 0,    0,    0x3C, 0x34, 0x3C, 0x84, 0x90, 0x70, 0x38, 0x10, 0xF5,
    9,    5,    6,    0xAA, 0xDC, 0x5C, 0x83, 0x38, 0,    0x24, 0,    0x1F, 0xF0, 0,    0xFF, 3,
    0x80, 0,    0,    0,    0,    0,    4,    0x40, 0,    0,    0,    0,    0,    0,    0,    0,
    0x82, 0x30, 0,    0,    0,    0xBC, 0x34, 0x40, 0,    0x10, 0,    0,    0,    0,    0x10, 0xC,
    0xC2, 2,    0,    4,    0,    0,    1,    0,    0xD0, 2,    0xE0, 1,    0x74, 0,    0x20, 0,
    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,    0x3E, 0x6B, 0xD,  2,    2,    0,    6,    0,
    0x81, 0xC2, 0x14, 0x10, 4,    8,    8,    0,    0x80, 0x7F, 0,    0x7F, 0x6E, 0,    0xEB, 0,
    0xB0, 0xCD, 0xFF, 0x1F, 0,    0x20, 0,    0,    0,    8,    0,    0,    0,    0x10, 0x80, 0x80,
    0,    0xA,  0x6F, 0,    0x15, 0x20, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    4,    4,    0x3B, 0,    0,    0x10, 0,    0x95, 0xCC, 0x64, 0xD0, 0x81, 0,    0x10, 0x80, 0x80,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x20, 0,    0x17, 0,    0xD0, 2,    0xE0, 1,    0x11, 3,    6,    1,    8,    0,    2,    0,
    1,    0,    0,    0,    0,    0x22, 0,    0,    0x2A, 1,    0x22, 2,    0,    0,    0,    3,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0xD0, 2,    0xE0, 1,
    0xB0, 0,    0x10, 0x80, 0,    0x80, 0,    0,    7,    0x80, 7,    0x80, 0,    0,    0x3C, 0,
    0x40, 4,    0,    0x11, 0x32, 0xAA, 0xD2, 0,    0,    0,    0,    0,    0xF,  7,    0,    0};

static const u8 ADV7181D_PDN[] = {0x42, 0xF, 0x24, 0xFF};

/* decompiled */
int
T132B_i2c_read(struct i2c_client *client, unsigned int dev_addr, u8 reg, u8 *dat)
{
	int ret;
	const char *log;
	u8 msg[1];
	struct i2c_msg msgs[2];

	msg[0] = reg;

	msgs[0].addr = (dev_addr >> 1) & 0xFFFF;
	msgs[0].flags = client->flags;
	msgs[0].buf = msg;
	msgs[0].len = 1;
	msgs[0].scl_rate = 350000;
	msgs[0].read_type = 2;

	msgs[1].addr = (dev_addr >> 1) & 0xFFFF;
	msgs[1].flags = client->flags | I2C_M_RD;
	msgs[1].buf = msg;
	msgs[1].len = 1;
	msgs[1].scl_rate = 350000;
	msgs[1].read_type = 0;

	ret = i2c_transfer(client->adapter, msgs, 2);

	if (ret < 0) {
		if (car_struct.car_status.video_decoder_type) {
			printk("mtc-dbg ADV7181D r err\n");
		} else {
			printk("mtc-dbg t132b r err\n");
		}
	} else {
		ret = 0;
		*dat = msg[0];
	}

	return ret;
}

/* decompiled */
int
T132B_i2c_write(struct i2c_client *client, unsigned int dev_addr, u8 reg, u8 dat)
{
	struct i2c_adapter *adap;
	int ret;
	u8 msg[2];
	struct i2c_msg msgs;

	msgs.flags = client->flags;
	msg[0] = reg;
	msg[1] = dat;
	msgs.buf = msg;
	msgs.addr = (dev_addr >> 1) & 0xFFFF;
	msgs.len = 2;
	msgs.read_type = 0;
	msgs.scl_rate = 350000;

	ret = i2c_transfer(client->adapter, &msgs, 1);

	if (ret < 0) {
		if (car_struct.car_status.video_decoder_type) {
			printk("mtc-dbg ADV7181D w err\n");
		} else {
			printk("mtc-dbg T132B w err\n");
		}
	} else {
		ret = 0;
	}

	return ret;
}

/* contains unknown fields */
static void
T132B_UV_Set()
{
	car_struct.car_status.u_value = car_struct.config_data._gap11[0] + 112;
	car_struct.car_status.v_value = car_struct.config_data._gap11[1] - 124;

	T132B_i2c_write(mtc_backview_dev.i2c_client, 0x40u, 12, car_struct.car_status.u_value);
	T132B_i2c_write(mtc_backview_dev.i2c_client, 0x40u, 10, car_struct.car_status.v_value);

	printk("UV set %02x,%02x\n", car_struct.car_status.u_value, car_structcar_status.v_value);
}

/* decompiled */
void
T132B_Write(struct i2c_client *client, u8 *data)
{
	unsigned int addr;

	addr = data[0];

	if (addr != 255) {
		int pos;

		pos = 0;

		do {
			T132B_i2c_write(client, addr, data[pos + 1], data[pos + 2]);
			pos += 3;
			addr = data[pos];
		} while (addr != 255);
	}
}

/* decompiled */
int
IC_WritByte(unsigned int dev_addr, char reg_addr, char data)
{
	return T132B_i2c_write(mtc_backview_dev.i2c_client, dev_addr, reg_addr, data);
}

/* decompiled */
int
IC_ReadByte(unsigned int dev_addr, char reg_addr)
{
	char dat;

	T132B_i2c_read(mtc_backview_dev.i2c_client, dev_addr, reg_addr, &dat);

	return dat;
}

/* decompiled */
void
twdDelay(unsigned int delay)
{
	msleep(delay >> 1);
}

/* decompiled */
int
T132B_Init(struct i2c_client *client, int type)
{
	int result;

	T132B_Write(client, T132B_INIT);
	msleep(20u);

	if (type == 1) {
		if (car_struct.car_status.t132b_yuv) {
			T132B_Page_Write(client, 0x40u, 240, T132B_P0_YUV, T132B_P0_MASK_RGB);
			T132B_UV_Set();
		} else {
			T132B_Page_Write(client, 0x40u, 240, T132B_P0_RGB, T132B_P0_MASK_RGB);
		}

		msleep(100u);
		result = 4;
	} else {
		result = T132B_CVBS_Auto(client);
	}

	return result;
}

int
backview_init()
{
	int result;

	result = i2c_register_driver(0, &mtc_backview_i2c_driver);
	if (result) {
		pr_err("Unable to register Backview driver\n");
	}

	return result;
}

static const struct i2c_device_id backview_id[] = {{"mtc-backview", 0}, {}};

static struct i2c_driver mtc_backview_i2c_driver = {
    .probe = backview_probe,
    .remove = backview_remove,
    .driver{
	.name = "mtc-backview",
    },
    .id_table = backview_id,
};

module_init(backview_init);
