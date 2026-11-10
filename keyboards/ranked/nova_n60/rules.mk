# Copyright 2026 patsys
# SPDX-License-Identifier: GPL-2.0-or-later
SRC += scan_tables.c
SRC += keyboards/ranked/common/matrix.c keyboards/ranked/common/nova_acquisition.c keyboards/ranked/common/nova_matrix_map.c
VPATH += keyboards/ranked/common

SRC += rgb/nova_rgb_transport.c rgb/nova_rgb_matrix.c
VPATH += keyboards/ranked/nova_n60/rgb
