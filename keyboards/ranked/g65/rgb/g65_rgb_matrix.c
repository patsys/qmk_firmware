// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later
#ifdef RGB_MATRIX_ENABLE
#    include "rgb_matrix.h"
#    include "timer.h"
#    include "wait.h"
#    include "g65_rgb_transport.h"
#    include <string.h>
#    include "ry72xx_delay.h"

/* Controller (channel, slot) of each RGB Matrix LED: the 69 keys at
 * (matrix column, matrix row + 1), then the 40-LED border ring from the stock
 * table at 0x11cc9 (slots 0, 6 and 7). */
static const uint8_t led_slots[RGB_MATRIX_LED_COUNT][2] = {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}, {12, 1}, {13, 1}, {13, 5}, {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {7, 2}, {8, 2}, {9, 2}, {10, 2}, {11, 2}, {12, 2}, {12, 5}, {0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3}, {6, 3}, {7, 3}, {8, 3}, {9, 3}, {10, 3}, {11, 3}, {13, 2}, {12, 3}, {13, 3}, {0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 4}, {7, 4}, {8, 4}, {9, 4}, {10, 4}, {11, 4}, {12, 4}, {13, 4}, {11, 5}, {0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {7, 5}, {8, 5}, {9, 5}, {10, 5}, {0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7}, {8, 7}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 6}, {7, 6}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}};

static g65_rgb_transport transport;
static uint8_t           colors[G65_RGB_CHANNELS][G65_RGB_SLOTS][3];
static uint32_t          read32(uintptr_t a) {
    return *(volatile uint32_t *)a;
}
static void write32(uintptr_t a, uint32_t v) {
    *(volatile uint32_t *)a = v;
}
static void delay_ms(uint32_t ms) {
    wait_ms(ms);
}
static void init(void) {
    const g65_rgb_io io = {read32, write32, ry72xx_delay_units, delay_ms, timer_read32};
    memset(colors, 0, sizeof(colors));
    g65_rgb_transport_init(&transport, &io);
}
static void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 0 || index >= RGB_MATRIX_LED_COUNT) return;
    uint8_t *led = colors[led_slots[index][0]][led_slots[index][1]];
    led[0]       = r;
    led[1]       = g;
    led[2]       = b;
}
static void set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < RGB_MATRIX_LED_COUNT; ++i)
        set_color(i, r, g, b);
}
static void flush(void) {
    g65_rgb_transport_submit(&transport, colors);
}
#    ifdef RANKED_DIAGNOSTICS
void g65_rgb_status(uint8_t out[12]) {
    out[0] = transport.usable;
    out[1] = transport.ready;
    out[2] = (uint8_t)transport.attempts;
    out[3] = transport.last_reply;
    for (unsigned i = 0; i < 4; ++i) {
        out[4 + i] = (uint8_t)(transport.completed >> (8 * i));
        out[8 + i] = (uint8_t)(transport.errors >> (8 * i));
    }
}
#    endif
const rgb_matrix_driver_t rgb_matrix_driver = {init, set_color, set_color_all, flush};
#endif
