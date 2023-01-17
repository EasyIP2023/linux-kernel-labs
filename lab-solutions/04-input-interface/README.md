# Lab 4 Input Interface

Directory provides patch for in-tree & out-of-tree compilation of the
device tree and nunchuk kernel module. Device tree now configures pinmux
information on the am3358 ARM Cortex A8 so that pins 17 and 18 utilize
I2C protocol bus.

SPI0_D1 to I2C1_SDA and SPI0_CS0 to I2C1_SCL

**Check for Z/C buttons and joystick x-axis/y-axis data**
```bash
# From embedded compute
$ insmod nunchuk/nunchuk.ko
$ evtest /dev/input/event0
```

**Check Driver Syntax Follows Standards**
```sh
$ ../../linux/scripts/checkpatch.pl --fix nunchuk.c
```
