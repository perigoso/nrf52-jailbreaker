/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2020 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define BIT(x) (1 << (x))

#define REG_SET(reg, mask, value) (reg = (reg & ~mask) | value)