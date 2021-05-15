/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2020 Rafael Silva <perigoso@riseup.net>
 */

#include "util/types.h"

#include "tusb.h"

void _putchar(char character)
{
	tud_cdc_write_char(character);
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if ( dtr )
  {
    // Terminal connected
  }else
  {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}