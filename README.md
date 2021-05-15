# nrf42-jailbreaker

Just a simple firmware running on a stm32 bluepill (and a couple external mosfets), allowing us to glitch nrf52 chips.

The device is triggerable through serial, exposed through usb cdc

### Command List
```
generic
 h    print this
 r    reset target
 g    glitch target(includes reset)

config
 w    glitch pulse width (format: command%d, 125ns steps, default 4/500ns)
 l    reset pulse width (format: command%d, 125ns steps, default 800000/100ms)
 d    time between reset and glitch (format: command%d, 125ns steps, default 504000/63ms)
```
## hardware

~~A external n mosfet needs to be connected to glitch the device~~ maybe we dont? why not connect the ios directly, a pushpull to power the device, and a open drain to glitch dec1, to be investigated

an external debugger is needed to connect to the target and extract the firmware, the script will work around a jlink and openocd

### firmware

building the firmware

you will need **arm-none-eabi**

`git clone https://github.com/perigoso/nrf52-jailbreaker`

`cd nrf52-jailbreaker/fw`

`mkdir build && cd build`

`meson .. --cross-file ../crossfile_bluepill.txt`

`ninja`

you can flash with a connected jlink with

`ninja flash`