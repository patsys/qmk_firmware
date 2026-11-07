// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* RY7203 USB endpoint FIFO primitives. Endpoint buffers hold one byte per
 * 32-bit word. Callers own endpoint readiness and serialization with the USB
 * IRQ; nothing here touches flash or other peripherals.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t (*ry72xx_usb_read8_fn)(void *context, uintptr_t address);
typedef void (*ry72xx_usb_write32_fn)(void *context, uintptr_t address, uint32_t value);
typedef struct {
    void                 *context;
    ry72xx_usb_read8_fn   read8;
    ry72xx_usb_write32_fn write32;
} ry72xx_usb_io;

enum { RY72XX_USB_IRQ = 4, RY72XX_USB_EP0_PACKET_BYTES = 8, RY72XX_USB_INTERRUPT_PACKET_BYTES = 64, RY72XX_USB_LAST_ENDPOINT = 5, RY72XX_USB_RAW_IN_ENDPOINT = 2, RY72XX_USB_RAW_OUT_ENDPOINT = 3, RY72XX_USB_RAW_REPORT_BYTES = 32 };

typedef struct {
    uint8_t  request_type;
    uint8_t  request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} ry72xx_usb_setup;

/* Submit only when the endpoint is ready (FIFO geometry for endpoints 0..5,
 * EP0 packet size 8). Success means the stores were issued, not that the host
 * acknowledged a transfer.
 */
bool ry72xx_usb_fifo_submit(const ry72xx_usb_io *io, unsigned endpoint, const uint8_t *source, size_t length);

/* Read one EP0 OUT packet from the low byte of each FIFO word. The caller
 * provides the received length and serializes access with the USB ISR.
 */
bool ry72xx_usb_ep0_receive(const ry72xx_usb_io *io, uint8_t *destination, size_t length);

/* Read an OUT packet of the given length from the endpoint 3 FIFO (Raw HID).
 * It does not clear the IRQ or invoke command handlers.
 */
bool ry72xx_usb_vendor_receive(const ry72xx_usb_io *io, uint8_t *destination, size_t length);

/* Read the eight SETUP bytes from word-spaced setup registers and decode USB
 * little-endian fields. This does not acknowledge the SETUP event.
 */
bool ry72xx_usb_read_setup(const ry72xx_usb_io *io, ry72xx_usb_setup *setup);
