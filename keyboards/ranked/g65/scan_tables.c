// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include "nova_matrix_map.h"

/* Bank and channel pin codes and the key id (row * 21 + column of the stock
 * key table) of each matrix position; 1 marks the unused position. */
const uint8_t nova_bank_pin_codes[NOVA_BANKS]         = {24, 23, 22, 21, 20};
const uint8_t nova_channel_pin_codes[NOVA_CHANNELS]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const uint8_t nova_key_ids[NOVA_CHANNELS][NOVA_BANKS] = {
    {21, 42, 63, 84, 105}, {22, 43, 64, 85, 106}, {23, 44, 65, 86, 107}, {24, 45, 66, 87, 111}, {25, 46, 67, 88, 114}, {26, 47, 68, 89, 115}, {27, 48, 69, 90, 1}, {28, 49, 70, 91, 116}, {29, 50, 71, 92, 117}, {30, 51, 72, 93, 118}, {31, 52, 73, 94, 119}, {32, 53, 74, 95, 98}, {33, 54, 76, 96, 56}, {34, 75, 77, 97, 35},
};
