// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later
#include "g65_rgb_transport.h"
#include <string.h>

/* I2C transport to the G65 LED controller, recovered from the stock
 * application: controller init 0x4cf8/0x4da8, DMA write 0x4cb8, reply read
 * 0x5ef4, start-up handshake 0x3544 and frame packer 0x3ca0. Pin codes are
 * the encoded values the board uses, not package pin numbers. Polled; no
 * flash, retained boot flag or interrupt handler access.
 */
#define I2C_BASE 0x40000000u
#define I2C_ENABLE (I2C_BASE + 0x00u)
#define I2C_SCL_PIN (I2C_BASE + 0x04u)
#define I2C_SDA_PIN (I2C_BASE + 0x08u)
#define I2C_ADDRESS (I2C_BASE + 0x0cu)
#define I2C_DIVIDER (I2C_BASE + 0x10u)
#define I2C_LENGTH (I2C_BASE + 0x18u)
#define I2C_RX_BUFFER (I2C_BASE + 0x1cu)
#define I2C_TX_BUFFER (I2C_BASE + 0x20u)
#define I2C_IRQ_MASK (I2C_BASE + 0x2cu)
#define I2C_STATUS (I2C_BASE + 0x30u)
#define I2C_START (I2C_BASE + 0x34u)
#define I2C_DMA (I2C_BASE + 0x40u)
#define I2C_BUSY (I2C_BASE + 0x44u)
#define I2C_CLEAR (I2C_BASE + 0x48u)
#define STATUS_DONE 0x01u
#define STATUS_ERRORS 0x16u
#define CONTROLLER 0x34u
#define RESET_PIN_A 0x45u
#define RESET_PIN_B 0x41u

static uint32_t read_reg(g65_rgb_transport *s, uintptr_t a) {
    return s->io.read32(a);
}
static void write_reg(g65_rgb_transport *s, uintptr_t a, uint32_t v) {
    s->io.write32(a, v);
}
static uintptr_t pin_config_address(uint8_t pin) {
    static const uintptr_t bases[4] = {0x40018090u, 0x40018148u, 0x4001810cu, 0x4001812cu};
    return bases[(pin >> 5) & 3u] + 4u * (pin & 31u);
}
static void pin_write(g65_rgb_transport *s, uintptr_t offset, uint8_t pin) {
    write_reg(s, 0x40018000u + offset + 4u * ((pin >> 5) & 3u), 1u << (pin & 31u));
}
static bool busy(g65_rgb_transport *s) {
    return (read_reg(s, I2C_BUSY) & 1u) != 0;
}
static void start(g65_rgb_transport *s, bool read) {
    write_reg(s, I2C_CLEAR, 0x1f);
    write_reg(s, I2C_ADDRESS, (CONTROLLER << 1) | (read ? 1u : 0u));
    write_reg(s, I2C_START, 3);
}
/* Wait for the running transfer; true when it completed without error. */
static bool wait_done(g65_rgb_transport *s, uint32_t timeout_ms) {
    const uint32_t begin = s->io.ticks();
    for (;;) {
        const uint32_t status = read_reg(s, I2C_STATUS);
        if (status & STATUS_DONE) {
            write_reg(s, I2C_CLEAR, 0x1f);
            return !(status & STATUS_ERRORS);
        }
        if ((uint32_t)(s->io.ticks() - begin) >= timeout_ms) return false;
    }
}
static void write_frame(g65_rgb_transport *s) {
    write_reg(s, I2C_TX_BUFFER, (uint32_t)(uintptr_t)s->tx);
    write_reg(s, I2C_LENGTH, G65_RGB_FRAME_BYTES);
    start(s, false);
}
/* Controller init 0x4cf8/0x4da8; repeated before every handshake attempt so
 * a transfer left hanging by a booting controller cannot block the next one. */
static void i2c_setup(g65_rgb_transport *s) {
    write_reg(s, I2C_ENABLE, 0);
    write_reg(s, 0x4001402c, 0x1); /* controller clock */
    s->io.delay_units(10);
    write_reg(s, 0xe000e180, 1u << 16); /* polled transport: keep IRQ 16 disabled */
    /* 2 MHz from the 48 MHz core: (48 MHz / 2 MHz) / 4 - 1. */
    write_reg(s, I2C_DIVIDER, 5);
    write_reg(s, I2C_LENGTH, 0x3fff);
    write_reg(s, pin_config_address(0x62), 0x341);
    write_reg(s, pin_config_address(0x63), 0x341);
    write_reg(s, I2C_DMA, 1);
    write_reg(s, I2C_ADDRESS, 0xa8);
    write_reg(s, I2C_SCL_PIN, 0x62);
    write_reg(s, I2C_SDA_PIN, 0x63);
    write_reg(s, I2C_ENABLE, 1);
    write_reg(s, I2C_IRQ_MASK, 0x1f);
    write_reg(s, I2C_CLEAR, 0x1f);
}
/* One handshake attempt (0x3544): an all-off frame must be answered with A5. */
static bool handshake(g65_rgb_transport *s) {
    static const uint8_t black[G65_RGB_CHANNELS][G65_RGB_SLOTS][3];
    ++s->attempts;
    i2c_setup(s);
    g65_rgb_pack(s->tx, black);
    write_frame(s);
    if (!wait_done(s, 20)) return false;
    s->io.delay_ms(2);
    if (busy(s)) return false;
    s->rx[0] = 0;
    write_reg(s, I2C_RX_BUFFER, (uint32_t)(uintptr_t)s->rx);
    write_reg(s, I2C_LENGTH, 1);
    start(s, true);
    const bool replied = wait_done(s, 5);
    s->last_reply      = s->rx[0];
    return replied && s->rx[0] == 0xa5;
}
/* Controller reset pulse 0x45f8: both lines driven through 0x40018010, then
 * back through 0x40018020 (their idle state after the matrix GPIO setup). */
static void reset_pulse(g65_rgb_transport *s, uint32_t ms) {
    pin_write(s, 0x10u, RESET_PIN_A);
    pin_write(s, 0x10u, RESET_PIN_B);
    s->io.delay_ms(ms);
    pin_write(s, 0x20u, RESET_PIN_B);
    pin_write(s, 0x20u, RESET_PIN_A);
}
/* The stock firmware retries forever with growing reset pulses and long
 * pauses; a cold controller can take longer than one attempt. Until it
 * answers, retry every 300 ms (pulsing the reset lines every fourth attempt)
 * without blocking the keyboard for longer than one attempt. */
static void recover(g65_rgb_transport *s) {
    if ((int32_t)(s->io.ticks() - s->next_attempt) < 0) return;
    if (handshake(s)) {
        s->ready = true;
        return;
    }
    if ((s->attempts & 3u) == 0) reset_pulse(s, 20);
    s->next_attempt = s->io.ticks() + 300u;
}
bool g65_rgb_transport_init(g65_rgb_transport *s, const g65_rgb_io *io) {
    if (!s || !io || !io->read32 || !io->write32 || !io->delay_units || !io->delay_ms || !io->ticks) return false;
    memset(s, 0, sizeof(*s));
    s->io     = *io;
    s->usable = true;
    s->ready  = handshake(s);
    if (!s->ready) s->next_attempt = s->io.ticks() + 300u;
    return s->ready;
}
void g65_rgb_pack(uint8_t frame[G65_RGB_FRAME_BYTES], const uint8_t colors[G65_RGB_CHANNELS][G65_RGB_SLOTS][3]) {
    /* Command C0, dimensions, then the controller's key and border levels
     * (stock range 0-4, default 4); QMK scales the channel values instead. */
    frame[0] = 0xc0;
    frame[1] = G65_RGB_CHANNELS;
    frame[2] = G65_RGB_SLOTS;
    frame[3] = 4;
    frame[4] = 4;
    for (unsigned plane = 0; plane < 3; ++plane)
        for (unsigned channel = 0; channel < G65_RGB_CHANNELS; ++channel)
            for (unsigned slot = 0; slot < G65_RGB_SLOTS; ++slot)
                frame[G65_RGB_HEADER_BYTES + plane * G65_RGB_CHANNELS * G65_RGB_SLOTS + channel * G65_RGB_SLOTS + slot] = colors[channel][slot][plane];
}
void g65_rgb_transport_poll(g65_rgb_transport *s) {
    if (!s || !s->ready || !s->active) return;
    const uint32_t status = read_reg(s, I2C_STATUS);
    if (status & STATUS_DONE) {
        write_reg(s, I2C_CLEAR, 0x1f);
        s->active = false;
        if (status & STATUS_ERRORS)
            ++s->errors;
        else
            ++s->completed;
    } else if ((uint32_t)(s->io.ticks() - s->started) >= 10u) {
        ++s->errors;
        s->started = s->io.ticks();
        /* A busy controller may still own the DMA buffer: do not reuse it. */
        if (!busy(s)) s->active = false;
    }
}
bool g65_rgb_transport_submit(g65_rgb_transport *s, const uint8_t colors[G65_RGB_CHANNELS][G65_RGB_SLOTS][3]) {
    if (!s || !colors || !s->usable) return false;
    if (!s->ready) {
        recover(s);
        return false;
    }
    g65_rgb_transport_poll(s);
    if (s->active || busy(s)) {
        ++s->skipped;
        return false;
    }
    g65_rgb_pack(s->tx, colors);
    write_frame(s);
    s->started = s->io.ticks();
    s->active  = true;
    ++s->submitted;
    return true;
}
