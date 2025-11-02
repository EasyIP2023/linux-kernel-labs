# Lab 7: Sleeping and handling interrupts

Directory provides patch for in-tree & out-of-tree compilation of the
device tree and serial-uart kernel module. Add device tree for uart
devices and configure pinmux information on the am3358 ARM Cortex A8
processor so that pins 21,22 (uart 2) and pins 11,13 (uart 4) on
Expansion Header P9 are utilized as uart pins.

**Compile and Install Driver**

```
$ make -C lab-solutions/08-locking
$ INSTALL_MOD_PATH="$(pwd)/modules/nfsroot" make -C lab-solutions/08-locking modules_install
```

**Compiling and Install Userspace IOCTL Examples**

```sh
$ make -C lab-solutions/08-locking/userspace
$ cp -av lab-solutions/08-locking/userspace/build/serial-* modules/nfsroot/root
```

**Driver Usage**

```s
$ modprobe serial-uart
$ echo "cool beans fire nice" > /dev/serial-481a8000
$ ./serial-get-counter /dev/serial-481a8000
$ ./serial-reset-counter /dev/serial-481a8000
```

**Check Driver Syntax Follows Standards**

```sh
$ ../../linux/scripts/checkpatch.pl --fix nunchuk.c
```

**References**

[Lammertbies Serial Uart](https://www.lammertbies.nl/comm/info/serial-uart)
