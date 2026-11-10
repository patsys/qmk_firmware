# Ranked Nova N60

Wired 60% ISO keyboard (62 keys) with analog Hall-effect switches, per-key RGB
and an RY7203 Cortex-M0 controller.

* Keyboard Maintainer: [patsys](https://github.com/patsys)
* Hardware Supported: Ranked Nova N60 ISO PCB with its factory bootloader (v1.0.5 or v1.0.7)
* Hardware Availability: [Ranked product page](https://ranked.gg/products/nova-n60-mechanical-gaming-keyboard-iso-de)

Make example for this keyboard (after setting up your build environment):

    make ranked/nova_n60:default

Flashing example for this keyboard:

    make ranked/nova_n60:default:flash

Flashing uses the separately distributed `ry72xx-flasher` host tool; see the
[platform readme](../../../platforms/ry72xx/readme.md).

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

The analog keys use fixed press/release thresholds. Settings are stored with
QMK wear-leveling in a reserved 4 KiB flash region; flashing new firmware keeps
the stock firmware's own settings pages untouched. All RGB Matrix effects are
enabled, including the reactive ones.

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Esc) and plug in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available (Fn + B in the default keymap)

There is no physical reset button. While the stock firmware is still installed,
`ry72xx-flasher --enter-stock` switches it into the bootloader.
