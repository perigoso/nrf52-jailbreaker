project('nrf52-jailbreaker', 'c')

if not meson.is_cross_build()
    error('Cross-compiling is required.')
endif

sources = [
    'src/startup.c',
    'src/exceptions.c',
	'src/main.c',
	'src/rcc.c',
	'src/systick.c',
	'src/flash.c',
	'src/gpio.c',
	'src/usb.c',
	'src/usb_descriptors.c',
	'src/cdc.c',
	'external/printf/printf.c',
	'external/tinyusb/src/tusb.c',
	'external/tinyusb/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c',
	'external/tinyusb/src/device/usbd.c',
	'external/tinyusb/src/device/usbd_control.c',
	'external/tinyusb/src/common/tusb_fifo.c',
	'external/tinyusb/src/class/cdc/cdc_device.c',
]

include_dirs = [
    'src',
	'external/cmsis-5/CMSIS/Core/Include',
	'external/cmsis-dfp-stm32f1/Include',
	'external/printf',
	'external/tinyusb/src',
]

git = find_program('git')
version = run_command(git, 'describe', '--abbrev=7', '--always', '--long', '--match', 'v*')

defines = [
	'PROJECT_NAME="' + meson.project_name() + '"',
	'BUILD_VERSION=0',
    'STM32F103xB',
	'EXTERNAL_CLOCK_VALUE=8000000UL',
	'PROJECT_VERSION="' + version.stdout().split('\n')[0] + '"',
]

c_flags = [
	'-Os',
	'-nostdlib',
	'-nostartfiles',
	'-ffunction-sections',
	'-fdata-sections',
	'-march=armv7-m',
	'-mtune=cortex-m3',
	'-mthumb',
	'-mfloat-abi=soft',
	'-ffreestanding',
]

ld_flags = [
	'-lm',
	'-lc',
	'-lgcc',
	'-lnosys',
	'-nostdlib',
	'-nostartfiles',
	'-fdata-sections',
	'-ffunction-sections',
	'--specs=nano.specs',
	'--specs=nosys.specs',
	'-Wl,--gc-sections',
	'-march=armv7-m',
	'-mtune=cortex-m3',
	'-mthumb',
    '-L../linker',
    '-Tstm32f103cb.ld'
]

foreach define : defines
c_flags += '-D' + define
endforeach

compiler = meson.get_compiler('c')

include = include_directories(include_dirs)

size = find_program('arm-none-eabi-size')
objcopy = find_program('arm-none-eabi-objcopy')

printf = find_program('echo')

jlinkexe = find_program('JLinkExe')

exe = executable(meson.project_name() + '.elf', sources,
        c_args: c_flags,
        link_args: ld_flags,
        include_directories: include)

bin = custom_target('bin',
        input: exe,
        output: exe.full_path().split('/')[-1].split('.elf')[0] + '.bin',
        command: [objcopy, '-O', 'binary', '--only-section=.isr_vector', '--only-section=.text', '--only-section=.ARM', '--only-section=.ram.text', '--only-section=.data', '@INPUT@', '@OUTPUT@'],
        build_by_default: true,
        depends: exe)

hex = custom_target('hex',
        input: exe,
        output: exe.full_path().split('/')[-1].split('.elf')[0] + '.hex',
        command: [objcopy, '-O', 'ihex', '@INPUT@', '@OUTPUT@'],
		build_by_default: true,
        depends: exe)

run_target('size',
        command: [size, exe.full_path(), '-B'],
        depends: exe)

run_target('flash',
        command: [jlinkexe, '-device', 'STM32F103CB', '-if', 'SWD', '-JTAGConf', '-1,-1', '-speed', 'auto', '-CommandFile', '../flash.jlink'],
        depends: hex)
