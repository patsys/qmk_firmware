// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include "ry72xx_usb_io.h"

typedef struct {
    void *context;
    uint8_t (*read8)(void *, uintptr_t);
    uint32_t (*read32)(void *, uintptr_t);
    void (*write32)(void *, uintptr_t, uint32_t);
    void (*delay_units)(void *, uint32_t);
    void (*delay_ms)(void *, uint32_t);
    void (*delay_short)(void *, uint32_t);
} ry72xx_usb_bus;
typedef struct {
    const uint8_t *data;
    uint16_t       length;
    uint8_t       *out_data;
    uint16_t       out_capacity;
} ry72xx_usb_control_reply;
typedef struct {
    void *context;
    const uint8_t *(*descriptor)(void *, uint8_t type, uint8_t index, uint16_t interface_number, uint16_t *length);
    void (*configured)(void *, uint8_t configuration);
    bool (*control)(void *, const ry72xx_usb_setup *, ry72xx_usb_control_reply *);
    void (*control_complete)(void *, const ry72xx_usb_setup *, const uint8_t *, uint16_t);
    void (*packet)(void *, unsigned endpoint, const uint8_t *, uint16_t);
    void (*frame)(void *);
    void (*event)(void *, unsigned raw_event);
    void (*power)(void *, bool suspended);
} ry72xx_usb_callbacks;
typedef enum { RY72XX_EP0_IDLE, RY72XX_EP0_IN_DATA, RY72XX_EP0_OUT_DATA, RY72XX_EP0_IN_STATUS, RY72XX_EP0_OUT_STATUS, RY72XX_EP0_STALL } ry72xx_usb_phase;
typedef struct ry72xx_usb_device {
    ry72xx_usb_bus           bus;
    ry72xx_usb_callbacks     callbacks;
    ry72xx_usb_setup         setup;
    ry72xx_usb_control_reply reply;
    ry72xx_usb_phase         phase;
    uint16_t                 transferred, total;
    uint16_t                 in_state, out_state;
    uint8_t                  address, pending_address, configuration;
    uint8_t                  small_reply[2];
    bool                     apply_address, need_zlp, remote_wakeup;
    volatile bool            suspended;
    bool                     wakeup_requested;
    bool                     saw_frame;
    uint32_t                 frame_events, suspend_events, resume_events, reset_events, aux_events;
    uint8_t                  last_aux_flags;
    volatile bool            ready[6];
} ry72xx_usb_device;

/* Clock, pin and NVIC setup belong to the platform. This initializes the
 * controller registers after the USB clock is enabled. */
void ry72xx_usb_device_init(ry72xx_usb_device *, const ry72xx_usb_bus *, const ry72xx_usb_callbacks *);
void ry72xx_usb_device_irq(ry72xx_usb_device *);
void ry72xx_usb_device_aux_irq(ry72xx_usb_device *);
bool ry72xx_usb_device_send(ry72xx_usb_device *, unsigned endpoint, const uint8_t *, size_t);
bool ry72xx_usb_device_configured(const ry72xx_usb_device *);
/* Host permission, configured suspend, >=5ms idle and one pulse per suspend
 * are mandatory. Caller serializes USB IRQ access; delay_ms keeps time alive. */
bool ry72xx_usb_device_remote_wakeup(ry72xx_usb_device *, uint32_t suspended_ms);
