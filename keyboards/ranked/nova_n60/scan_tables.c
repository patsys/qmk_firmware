// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include "nova_matrix_map.h"

/* Bank and channel pin codes and the key id of each matrix position. */
const uint8_t nova_bank_pin_codes[NOVA_BANKS]         = {24, 23, 22, 21, 20, 19};
const uint8_t nova_channel_pin_codes[NOVA_CHANNELS]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const uint8_t nova_key_ids[NOVA_CHANNELS][NOVA_BANKS] = {
    {21, 42, 63, 84, 105, 1}, {22, 43, 64, 85, 106, 32}, {23, 44, 65, 86, 107, 33}, {24, 45, 66, 87, 111, 34}, {25, 46, 67, 88, 115, 53}, {26, 47, 68, 89, 116, 54}, {27, 48, 69, 90, 117, 1}, {28, 49, 70, 91, 118, 74}, {29, 50, 71, 92, 95, 75}, {30, 51, 72, 93, 1, 76}, {31, 52, 73, 94, 97, 1},
};
