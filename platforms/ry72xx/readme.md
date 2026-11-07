# RY7203 bare-metal platform

Platform support for the RY7203 Cortex-M0 (r0p0, CPUID `0x410CC200`) used in the
Ranked Nova N60. No public register reference or compatible ChibiOS HAL exists,
so the platform has its own register backend: clock, SysTick timebase, delays,
USB device controller and the flash controller used for EEPROM wear-leveling.
It reuses the Apache-licensed ChibiOS ARMv6-M startup code only. QMK's normal
main loop drives the platform and protocol hooks.

The register sequences were worked out by observing the stock firmware's
behaviour on the device and measuring the peripherals, and were implemented
independently. Registers whose meaning is not documented are written as plain
addresses in the order the hardware requires.

## Bootloader and flashing

The board keeps its factory USB HID bootloader. Supported revisions are
`ry72xx_boot_usb_v1.0.5` and `v1.0.7` (newer units ship with v1.0.7). The
bootloader is entered through the standard QMK paths: Bootmagic or `QK_BOOT`
call `bootloader_jump()`, which sets the retained request `0x5AC396C5` at
`0x4001400C` and resets.

Flashing needs the external [`ry72xx-flasher`](https://github.com/patsys/ry72xx-flasher)
host tool (like `dfu-util` for DFU boards); `qmk flash` and `make <keyboard>:<keymap>:flash` call it. It writes only
the application pages, backs up and reads back the whole flash, and supports
both bootloader revisions. Do not send a hex file to an unrelated MCU tool.

## Memory

- Application vectors start at `0x2000`; the bootloader forwards interrupts there.
  Application flash ends at `0x109FF` (117 pages of 512 bytes).
- SRAM is allocated within `0x20000000..0x20001C4F`, the stock application's
  initial stack pointer; this is a safe window, not the physical SRAM size.
- Bootloader v1.0.7 verifies the XOR of the application words against the
  info-flash metadata before starting it; the flasher sets the reserved vector
  entry at `0x201C` so the check passes.
- QMK EEPROM: wear-leveling on flash pages F8-FF (4 KiB at `0x1F000..0x1FFFF`),
  which the info-flash permission bitmap opens for writing. Stock settings
  (D5-DF), other data pages (E0-F7) and the info-flash metadata are never written.

## Clock

The core runs at 48 MHz from the RC oscillator, using the factory trim entry for
48 MHz from the info flash. SysTick and the calibrated delays assume 48 MHz;
startup falls back to the bootloader if another rate results.

## Status

Typing, lighting, NKRO, Raw HID/VIA, wake from suspend and flash-backed settings
persistence have been tested on Nova N60 units. Mouse, media and console reports
are not implemented yet and are rejected at build time.
