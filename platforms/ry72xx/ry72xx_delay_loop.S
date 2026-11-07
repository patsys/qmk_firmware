// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

.syntax unified
.cpu cortex-m0
.thumb
.section .text.ry72xx_delay48_loop,"ax",%progbits
.global ry72xx_delay48_loop
.type ry72xx_delay48_loop, %function
.thumb_func
/* Calibrated busy-wait at 48 MHz: one iteration is a subtract, 25 NOPs and a
 * branch. Fixed encodings keep the cycle count independent of the assembler. */
ry72xx_delay48_loop:
1: .inst.n 0x1e40 /* subs r0,r0,#1 */
.rept 25
    .inst.n 0xbf00 /* architectural NOP, not MOV r8,r8 */
.endr
    bne 1b
    bx lr
.size ry72xx_delay48_loop, .-ry72xx_delay48_loop
