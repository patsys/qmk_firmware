# Copyright 2026 patsys
# SPDX-License-Identifier: GPL-2.0-or-later
# Development diagnostics (g65.c): build with RANKED_DIAGNOSTICS=yes.
ifeq ($(strip $(RANKED_DIAGNOSTICS)), yes)
    OPT_DEFS += -DRANKED_DIAGNOSTICS
    ifneq ($(strip $(VIA_ENABLE)), yes)
        RAW_ENABLE = yes
    endif
endif
