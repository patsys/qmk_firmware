# Copyright 2026 patsys
# SPDX-License-Identifier: GPL-2.0-or-later
ifeq ($(strip $(MCU)),RY7203)
    PLATFORM_KEY = ry72xx
    MCU_ARCH = cortex-m0
    MCU_FAMILY = RY72XX
endif
