// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdint.h>
#include <stdbool.h>

/* LED controller frame: 5-byte header and three 14 x 8 color planes. */
enum { G65_RGB_CHANNELS = 14, G65_RGB_SLOTS = 8, G65_RGB_HEADER_BYTES = 5, G65_RGB_FRAME_BYTES = G65_RGB_HEADER_BYTES + 3 * G65_RGB_CHANNELS * G65_RGB_SLOTS };
typedef struct {
    uint32_t (*read32)(uintptr_t address);
    void (*write32)(uintptr_t address, uint32_t value);
    void (*delay_units)(uint32_t units);
    void (*delay_ms)(uint32_t ms);
    uint32_t (*ticks)(void);
} g65_rgb_io;
typedef struct {
    g65_rgb_io io;
    _Alignas(16) uint8_t tx[G65_RGB_FRAME_BYTES];
    _Alignas(16) uint8_t rx[4];
    uint32_t started, submitted, completed, errors, skipped;
    uint32_t attempts, next_attempt;
    bool     usable, ready, active;
    uint8_t  last_reply;
} g65_rgb_transport;

bool g65_rgb_transport_init(g65_rgb_transport *state, const g65_rgb_io *io);
bool g65_rgb_transport_submit(g65_rgb_transport *state, const uint8_t colors[G65_RGB_CHANNELS][G65_RGB_SLOTS][3]);
void g65_rgb_transport_poll(g65_rgb_transport *state);
#ifdef RANKED_DIAGNOSTICS
/* Diagnostics for the raw HID status request (g65.c): ready, attempts,
 * completed frames, errors and the last handshake reply. */
void g65_rgb_status(uint8_t out[12]);
#endif
void g65_rgb_pack(uint8_t frame[G65_RGB_FRAME_BYTES], const uint8_t colors[G65_RGB_CHANNELS][G65_RGB_SLOTS][3]);
