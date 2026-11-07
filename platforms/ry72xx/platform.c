// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdbool.h>
#include "ry72xx_delay.h"
#include <stdint.h>
#include "keyboard.h"
#include "bootloader.h"
#include "ry72xx_clock.h"
#ifdef RGB_MATRIX_ENABLE
#    include "rgb_matrix.h"
#endif
extern bool          ry72xx_usb_keyboard_init(void);
extern void          ry72xx_usb_keyboard_pre_task(void);
extern bool          ry72xx_usb_keyboard_suspended(void);
extern void          ry72xx_usb_keyboard_suspend_task(void);
extern void          ry72xx_usb_keyboard_task(void);
void                 platform_setup(void) {
    __asm__ volatile("cpsid i" ::: "memory");
    /* Clear inherited enabled/pending external IRQs before initializing ours. */
    *(volatile uint32_t *)0xe000e180u = 0xffffffffu;
    *(volatile uint32_t *)0xe000e280u = 0xffffffffu;
    ry72xx_clock_setup();
    if (!ry72xx_clock_calibrate(ry72xx_delay_units, ry72xx_delay_long)) {
        bootloader_jump();
    }
    /* Delays and SysTick assume 48 MHz; fall back to the bootloader rather than
     * run at another rate. */
    if (ry72xx_clock_effective_hz() != RY72XX_NOMINAL_CLOCK_HZ) {
        bootloader_jump();
    }
}
void protocol_setup(void) {}
void protocol_pre_init(void) {
    // QMK initializes SysTick at the start of keyboard_init(). Bootmagic and
    // other standard initialization hooks may wait on that timer. External
    // IRQs were cleared during platform_setup; USB IRQ4 is enabled only after
    // its device state is initialized in protocol_post_init().
    __asm__ volatile("cpsie i" ::: "memory");
}
void protocol_post_init(void) {
    /* The status of this bounded sequence is ignored, as in the stock firmware. */
    (void)ry72xx_shared_startup_enable(ry72xx_delay_units, ry72xx_delay_long);
    if (!ry72xx_usb_keyboard_init()) {
        bootloader_jump();
    }
    __asm__ volatile("cpsie i" ::: "memory");
}
void protocol_pre_task(void) {
    ry72xx_usb_keyboard_pre_task();
}
void protocol_keyboard_task(void) {
    if (ry72xx_usb_keyboard_suspended()) {
        ry72xx_usb_keyboard_suspend_task();
#ifdef RGB_MATRIX_ENABLE
        /* Finish the asynchronous black frame while acquisition is paused. */
        rgb_matrix_task();
#endif
        __asm__ volatile("wfi");
        return;
    }
    keyboard_task();
}
void protocol_post_task(void) {
    ry72xx_usb_keyboard_task();
}
