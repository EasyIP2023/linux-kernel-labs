# Bootlin Linux Kernel Practice

**Useful Links**
- [Updating kernel and bootloader on BeagleBone Black Wireless](https://krinkinmu.github.io/2020/07/05/beaglebone-software-update.html)


**Creating NFS root filesystem**
```sh
$ git clone git://git.busybox.net/busybox.git
$ cd busybox/
$ export ARCH=arm
$ export CROSS_COMPILE=arm-linux-gnueabi-
$ make defconfig
# CONFIG_STATIC=y (Settings > Build static binary (no shared libs))
# CONFIG_PREFIX="$HOME/linux-kernel-labs/modules/nfsroot" (Settings > Destination path for 'make install')
$ make -j 8
$ make install
```

```
# Add to file
#!/bin/sh
# mount -t proc none /proc
# mount -t sysfs none /sys
# exec sh

$ vim ${NFSROOT}/etc/init.d/rcS
```

**Restarting NFS server**
```sh
sudo /etc/init.d/nfs-kernel-server restart
```
