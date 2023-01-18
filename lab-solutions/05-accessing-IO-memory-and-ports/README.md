# Lab 5 Accessing I/O Memory and Ports

Directory provides patch for in-tree & out-of-tree compilation of the
device tree and serial-uart kernel module. Add device tree for uart
devices and configure pinmux information on the am3358 ARM Cortex A8
processor so that pins 21,22 (uart 2) and pins 11,13 (uart 4) on
Expansion Header P9 are utilized as uart pins.

**Create NFS rootfs serial-uart folder**
```sh
$ mkdir -p "${CDIR}/modules/nfsroot/root/serial-uart"
```

**Check Driver Syntax Follows Standards**
```sh
$ ../../linux/scripts/checkpatch.pl --fix nunchuk.c
```
