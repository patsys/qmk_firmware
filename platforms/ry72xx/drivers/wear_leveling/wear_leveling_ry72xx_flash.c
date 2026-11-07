// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "wear_leveling_internal.h"
#include "ry72xx_clock.h"
#include "ry72xx_flash_guards.h"

/*
 * The stock application maps flash page IDs to 512-byte pages and exposes a
 * 32-bit word-programming command at 0x40013000. Only F8-FF are assigned to
 * this driver. Their erased state and metadata permission bits were checked
 * by a verified read-only full-flash probe; E0-F7 contain preserved data.
 */
#define RY72XX_FLASH_CONTROLLER 0x40013000u
#define RY72XX_FLASH_METADATA 0x10000600u
#define RY72XX_FLASH_PAGE_SIZE 512u
#define RY72XX_FLASH_FIRST_PAGE 0xF8u
#define RY72XX_FLASH_LAST_PAGE 0xFFu
#define RY72XX_FLASH_PAGE_COUNT (RY72XX_FLASH_LAST_PAGE - RY72XX_FLASH_FIRST_PAGE + 1u)
#define RY72XX_FLASH_BASE_ADDRESS (RY72XX_FLASH_FIRST_PAGE * RY72XX_FLASH_PAGE_SIZE)
#define RY72XX_FLASH_BYTES (RY72XX_FLASH_PAGE_COUNT * RY72XX_FLASH_PAGE_SIZE)
#define RY72XX_FLASH_TIMEOUT (1u << 20)
#define RY72XX_FLASH_PAGE_GATE_OFFSET 0xD0u

STATIC_ASSERT(WEAR_LEVELING_BACKING_SIZE == RY72XX_FLASH_BYTES, "Wear-leveling size must be exactly pages F8-FF");
STATIC_ASSERT(BACKING_STORE_WRITE_SIZE == 4, "Nova flash programming uses 32-bit words");
STATIC_ASSERT(sizeof(backing_store_int_t) == RY72XX_FLASH_WRITE_BYTES, "Nova backing store entries must be 32 bits");
STATIC_ASSERT(RY72XX_FLASH_BYTES == RY72XX_FLASH_BACKING_BYTES, "Nova flash guard range must match F8-FF");

static volatile uint32_t *const flash_registers = (volatile uint32_t *)RY72XX_FLASH_CONTROLLER;
static bool controller_ready;
static bool write_unlocked;


static uint32_t irq_disable(void) {
    uint32_t primask;
    __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(primask) : : "memory");
    return primask;
}

static void irq_restore(uint32_t primask) {
    if ((primask & 1u) == 0) {
        __asm__ volatile("cpsie i" : : : "memory");
    }
}

static bool flash_configure(void) {
    const uint32_t hz = ry72xx_clock_effective_hz();
    if (hz < 1500000u || hz > 48000000u) {
        return false;
    }

    const uint32_t mhz = hz / 1000000u;
    flash_registers[0x14u / 4u] = 0x207u;
    flash_registers[0x18u / 4u] = 0x0Bu;
    flash_registers[0x20u / 4u] = 0x40u;
    flash_registers[0x24u / 4u] = 0x137u;
    flash_registers[0x28u / 4u] = (0x144u * mhz) / 0x30u;
    flash_registers[0x30u / 4u] = 0x104u;
    flash_registers[0x38u / 4u] = 0x114u;
    flash_registers[0x3Cu / 4u] = (0x34BC0u * mhz) / 0x30u;
    flash_registers[0x40u / 4u] = (0x15F900u * mhz) / 0x30u;
    flash_registers[0x4Cu / 4u] = 6u;
    __asm__ volatile("dsb" : : : "memory");
    return true;
}

static bool page_allowed(uint32_t page) {
    if (page < RY72XX_FLASH_FIRST_PAGE || page > RY72XX_FLASH_LAST_PAGE) {
        return false;
    }
    const uint32_t group = page >> 5;
    const uint32_t bit   = 1u << (page & 31u);
    const volatile uint32_t *const allow = (const volatile uint32_t *)(RY72XX_FLASH_METADATA + 0x50u + group * 4u);
    const volatile uint32_t *const gate  = (const volatile uint32_t *)(RY72XX_FLASH_CONTROLLER + RY72XX_FLASH_PAGE_GATE_OFFSET + group * 4u);
    /* The page write gate is set one-hot immediately before each controller
     * operation. Its readback is not a valid precondition (it has read as all
     * ones in the running application), so the info-flash permission bitmap is
     * the authoritative page check here.
     */
    (void)gate;
    return (*allow & bit) != 0u;
}

/* The first STATUS read after the start bit returns 0x1: busy (bit 1) rises one
 * read later and both bits clear at completion (measured on hardware). Polling
 * bit 1 alone can report completion before the operation has begun. */
static bool wait_ready(void) {
    for (uint32_t attempt = 0; attempt < RY72XX_FLASH_TIMEOUT; ++attempt) {
        if ((flash_registers[0x10u / 4u] & 3u) == 0u) {
            __asm__ volatile("dsb" : : : "memory");
            return true;
        }
    }
    return false;
}

static bool erase_page(uint32_t page) {
    if (!page_allowed(page)) {
        return false;
    }

    const uint32_t primask = irq_disable();
    volatile uint32_t *const gate = (volatile uint32_t *)(RY72XX_FLASH_CONTROLLER + RY72XX_FLASH_PAGE_GATE_OFFSET + (page >> 5) * 4u);
    const uint32_t bit = 1u << (page & 31u);
    *gate = bit;
    flash_registers[0x04u / 4u] = page << 7;
    flash_registers[0x00u / 4u] = 2u;
    flash_registers[0x10u / 4u] |= 1u;
    const bool ok = wait_ready();
    /* Clear only this page's gate bit: the register aliases a page bitmap, so
     * writing zero would also clear unrelated bits. */
    *gate &= ~bit;
    irq_restore(primask);
    if (!ok) {
        return false;
    }

    const volatile uint32_t *const contents = (const volatile uint32_t *)(page * RY72XX_FLASH_PAGE_SIZE);
    for (uint32_t word = 0; word < RY72XX_FLASH_PAGE_SIZE / sizeof(uint32_t); ++word) {
        if (contents[word] != UINT32_MAX) {
            return false;
        }
    }
    return true;
}

bool backing_store_init(void) {
    controller_ready = flash_configure();
    write_unlocked  = false;
    return controller_ready;
}

bool backing_store_unlock(void) {
    if (!controller_ready) {
        return false;
    }
    write_unlocked = true;
    return true;
}

bool backing_store_lock(void) {
    write_unlocked = false;
    return true;
}

bool backing_store_erase(void) {
    if (!controller_ready || !write_unlocked) {
        return false;
    }
    for (uint32_t page = RY72XX_FLASH_FIRST_PAGE; page <= RY72XX_FLASH_LAST_PAGE; ++page) {
        if (!erase_page(page)) {
            return false;
        }
    }
    return true;
}

bool backing_store_write(uint32_t address, backing_store_int_t value) {
    if (!controller_ready) {
        return false;
    }
    if (!write_unlocked) {
        return false;
    }
    if (!ry72xx_flash_address_valid(address)) {
        return false;
    }

    const uint32_t page = RY72XX_FLASH_FIRST_PAGE + address / RY72XX_FLASH_PAGE_SIZE;
    const uint32_t word = (address % RY72XX_FLASH_PAGE_SIZE) / sizeof(value);
    if (!page_allowed(page)) {
        return false;
    }

    volatile uint32_t *const contents = (volatile uint32_t *)(RY72XX_FLASH_BASE_ADDRESS + address);
    /* Refuse non-erased cells; QMK's log appends and only rewrites after erase. */
    if (*contents != UINT32_MAX) {
        return false;
    }

    const uint32_t primask = irq_disable();
    volatile uint32_t *const gate = (volatile uint32_t *)(RY72XX_FLASH_CONTROLLER + RY72XX_FLASH_PAGE_GATE_OFFSET + (page >> 5) * 4u);
    const uint32_t bit = 1u << (page & 31u);
    *gate = bit;
    flash_registers[0x08u / 4u] = ~value;
    flash_registers[0x04u / 4u] = (page << 7) | word;
    flash_registers[0x00u / 4u] = 1u;
    flash_registers[0x10u / 4u] |= 1u;
    const bool ok = wait_ready();
    /* Bit-preserving gate clear, as for erase. */
    *gate &= ~bit;
    irq_restore(primask);
    const bool verified = ok && *contents == ~value;
    return verified;
}

bool backing_store_read(uint32_t address, backing_store_int_t *value) {
    if (value == NULL || !ry72xx_flash_address_valid(address)) {
        return false;
    }
    const volatile uint32_t *const contents = (const volatile uint32_t *)(RY72XX_FLASH_BASE_ADDRESS + address);
    *value = ~*contents;
    return true;
}
