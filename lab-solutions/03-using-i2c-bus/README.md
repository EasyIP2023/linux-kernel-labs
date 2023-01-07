# Lab 3 Using I2C Bus

Directory provides patch for in-tree & out-of-tree compilation of the
device tree and nunchuk kernel module. Device tree now configures pinmux
information on the am3358 ARM Cortex A8 so that pins 17 and 18 utilize
I2C protocol bus.

SPI0_D1 to I2C1_SDA and SPI0_CS0 to I2C1_SCL

**Using i2cdetect**

Should see the i2c register address to communicate.

```bash
$ i2cdetect -l
i2c-1	i2c       	OMAP I2C adapter                	I2C adapter
i2c-2	i2c       	OMAP I2C adapter                	I2C adapter
i2c-0	i2c       	OMAP I2C adapter                	I2C adapter
```

```bash
$ i2cdetect -F 1
Functionalities implemented by bus #1
I2C                              yes
SMBus quick command              no
SMBus send byte                  yes
SMBus receive byte               yes
SMBus write byte                 yes
SMBus read byte                  yes
SMBus write word                 yes
SMBus read word                  yes
SMBus process call               yes
SMBus block write                yes
SMBus block read                 no
SMBus block process call         no
SMBus PEC                        yes
I2C block write                  yes
I2C block read                   yes
```

```bash
$ i2cdetect -r 1
i2cdetect: WARNING! This program can confuse your I2C bus
Continue? [y/N] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- 52 -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

**Check Driver Syntax Follows Standards**
```sh
$ ../../linux/scripts/checkpatch.pl --fix nunchuk.c
```
