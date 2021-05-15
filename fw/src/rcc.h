/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2020 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#include "util/types.h"

struct rcc_clock_tree_t {
	u32 sys_clock_freq;
	u32 ahb_clock_freq;
	u32 apb1_clock_freq;
	u32 apb2_clock_freq;
	u32 apb1_tim_clock_freq;
	u32 apb2_tim_clock_freq;
	u32 adc_clock_freq;
};

void rcc_init(u32 clock);
void rcc_update_clock_tree();
struct rcc_clock_tree_t rcc_get_clock_tree();
