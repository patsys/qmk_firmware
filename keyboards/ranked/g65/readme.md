# Ranked G65

Wired 65% ISO keyboard (69 keys) with per-key RGB (plus a 40-LED border ring)
and an RY7203 Cortex-M0 controller, on the same platform as the Ranked Nova N60.
The switches are read through the controller's ADC; on the tested unit they
switch between two levels and give no travel information.

* Keyboard Maintainer: [patsys](https://github.com/patsys)
* Hardware Supported: Ranked G65 ISO PCB (`3532:a0c2`) with its factory bootloader v1.0.7
* Hardware Availability: [Ranked](https://ranked.gg)

Make example for this keyboard (after setting up your build environment):

    make ranked/g65:default

Flashing example for this keyboard:

    make ranked/g65:default:flash

Flashing uses the separately distributed `ry72xx-flasher` host tool; see the
[platform readme](../../../platforms/ry72xx/readme.md).

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

The analog scan (5 banks x 14 channels) and its fixed press/release thresholds
are shared with the Nova N60 (`keyboards/ranked/common`). The LED controller
is driven over I2C; RGB Matrix supports solid color, breathing, cycle-all,
horizontal and vertical rainbow waves and a moving rainbow chevron. Settings are stored with QMK wear-leveling in pages F8-FF; the stock
firmware's settings pages (D0-EC) are left untouched.

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Esc) and plug in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available (Fn + B in the default keymap)

Development builds made with `RANKED_DIAGNOSTICS=yes` also accept the raw HID
request `07 00 B0 52 59 42 4C` ("RYBL"), which needs no key, and answer
read-only LED (`08 00 B1`) and sensor (`08 00 B2 <bank>`) queries. Normal builds
contain none of these.

There is no physical reset button and the factory bootloader has no recovery
pin. While the stock firmware is still installed, `ry72xx-flasher --enter-stock`
switches it into the bootloader.
