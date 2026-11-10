// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

/* Development diagnostics, only with RANKED_DIAGNOSTICS = yes (post_rules.mk):
 * a raw HID bootloader request that needs no key, and read-only LED and sensor
 * queries. Normal builds enter the bootloader only by key (Bootmagic, QK_BOOT). */
#ifdef RANKED_DIAGNOSTICS
#    include "quantum.h"
#    include "nova_acquisition.h"
#    ifdef RGB_MATRIX_ENABLE
#        include "g65_rgb_transport.h"
#    endif

/* Bootloader entry over raw HID that does not depend on reading keys: the
 * factory bootloader has no recovery pin, so a development build that cannot
 * scan the keys would otherwise have no way back. The request is VIA's
 * custom-set-value layout on the keyboard channel:
 *   [0x07, 0x00, 0xB0, 'R', 'Y', 'B', 'L']
 */
static bool is_bootloader_request(const uint8_t *data, uint8_t length) {
    static const uint8_t request[] = {0x07, 0x00, 0xB0, 'R', 'Y', 'B', 'L'};
    return length >= sizeof(request) && memcmp(data, request, sizeof(request)) == 0;
}

/* LED controller status, read-only: [0x08, 0x00, 0xB1] answers with
 * g65_rgb_status() from byte 3 on. */
static bool lighting_status(uint8_t *data, uint8_t length) {
#    ifdef RGB_MATRIX_ENABLE
    if (length < 15 || data[0] != 0x08 || data[1] != 0x00 || data[2] != 0xB1) return false;
    g65_rgb_status(&data[3]);
    return true;
#    else
    return false;
#    endif
}

/* Raw analog samples, read-only: [0x08, 0x00, 0xB2, bank] answers with the
 * 14 channel samples of that bank (little-endian 16-bit) from byte 4 on. */
static bool sensor_samples(uint8_t *data, uint8_t length) {
    if (length < 32 || data[0] != 0x08 || data[1] != 0x00 || data[2] != 0xB2 || data[3] >= NOVA_BANKS) return false;
    for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel) {
        const uint16_t sample     = nova_matrix_sample(data[3], channel);
        data[4 + 2 * channel]     = (uint8_t)sample;
        data[4 + 2 * channel + 1] = (uint8_t)(sample >> 8);
    }
    return true;
}

#    if defined(VIA_ENABLE)
#        include "via.h"
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    if (is_bootloader_request(data, length)) bootloader_jump();
    if (!lighting_status(data, length) && !sensor_samples(data, length)) data[0] = id_unhandled;
}
#    elif defined(RAW_ENABLE)
#        include "raw_hid.h"
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (is_bootloader_request(data, length)) bootloader_jump();
    if (lighting_status(data, length) || sensor_samples(data, length)) raw_hid_send(data, length);
}
#    endif
#endif // RANKED_DIAGNOSTICS
