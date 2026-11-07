// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include "nova_matrix_map.h"

/* Bank and channel pin codes and the key id of each matrix position. */
const uint8_t nova_bank_pin_codes[NOVA_BANKS]         = {24, 23, 22, 21, 20, 19};
const uint8_t nova_channel_pin_codes[NOVA_CHANNELS]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const uint8_t nova_key_ids[NOVA_CHANNELS][NOVA_BANKS] = {
    {21, 42, 63, 84, 105, 1}, {22, 43, 64, 85, 106, 32}, {23, 44, 65, 86, 107, 33}, {24, 45, 66, 87, 111, 34}, {25, 46, 67, 88, 115, 53}, {26, 47, 68, 89, 116, 54}, {27, 48, 69, 90, 117, 1}, {28, 49, 70, 91, 118, 74}, {29, 50, 71, 92, 95, 75}, {30, 51, 72, 93, 1, 76}, {31, 52, 73, 94, 97, 1},
};
/* Pin configuration base per GPIO port. */
const uintptr_t nova_pin_config_bases[4] = {0x40018090u, 0x40018148u, 0x4001810cu, 0x4001812cu};

uint16_t nova_hysteresis(uint16_t bank_bits, unsigned bank, uint16_t sample) {
    if (bank >= NOVA_BANKS) return bank_bits;
    const uint16_t bit = (uint16_t)(1u << bank);
    if (sample > 30u) return (uint16_t)(bank_bits | bit);
    if (sample < 20u) return (uint16_t)(bank_bits & (uint16_t)~bit);
    return bank_bits;
}
bool nova_position_present(unsigned bank, unsigned channel) {
    return bank < NOVA_BANKS && channel < NOVA_CHANNELS && nova_key_ids[channel][bank] != NOVA_ABSENT_ID;
}
uintptr_t nova_result_address(unsigned channel) {
    return channel < NOVA_CHANNELS ? 0x4000e09cu + 4u * channel : 0;
}
uintptr_t nova_pin_config_address(uint8_t pin) {
    const unsigned group = (pin >> 5) & 3u;
    return group == 1u ? 0 : nova_pin_config_bases[group] + 4u * (pin & 31u);
}
uintptr_t nova_pin_write10_address(uint8_t pin) {
    return 0x40018010u + 4u * ((pin >> 5) & 3u);
}
uintptr_t nova_pin_write20_address(uint8_t pin) {
    return 0x40018020u + 4u * ((pin >> 5) & 3u);
}
uint32_t nova_pin_mask(uint8_t pin) {
    return (uint32_t)1u << (pin & 31u);
}
uint32_t nova_systick_reload(uint32_t nominal_clock_hz, uint32_t period_argument) {
    return (nominal_clock_hz / 1000000u) * period_argument - 16u;
}
