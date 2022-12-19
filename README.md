# Bootlin Linux Kernel Practice

**Useful Links**
- [Updating kernel and bootloader on BeagleBone Black Wireless](https://krinkinmu.github.io/2020/07/05/beaglebone-software-update.html)


**tftp server**
```sh
$ sudo apt install tftpd-hpa
# Update to TFTP_DIRECTORY variable
# TFTP_DIRECTORY="/home/vince/linux-kernel-labs/tftp-server-files"
$ sudo vim /etc/default/tftpd-hpa
$ sudo /etc/init.d/tftpd-hpa restart
```

**Creating NFS root filesystem**
```sh
$ git clone git://git.busybox.net/busybox.git
$ cd busybox/
$ export ARCH=arm
$ export CROSS_COMPILE=arm-linux-gnueabi-
$ make defconfig
# CONFIG_STATIC=y (Settings > Build static binary (no shared libs))
# CONFIG_PREFIX="$HOME/linux-kernel-labs/modules/nfsroot" (Settings > Destination path for 'make install')
$ make -j $(nproc)
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

```
$ sudo apt install nfs-kernel-server
$ sudo echo "$HOME/linux-kernel-labs/modules/nfsroot 192.168.1.100(rw,no_root_squash,no_subtree_check)" >> /etc/exports
```

Restarting NFS server
```sh
$ sudo exportfs -r
$ sudo /etc/init.d/nfs-kernel-server restart
```

**Uboot Environment Variables**
```
setenv ethact usb_ether
setenv ipaddr 192.168.1.100
setenv serverip 192.168.1.1
setenv usbnet_devaddr f8:dc:7a:00:00:02
setenv usbnet_hostaddr f8:dc:7a:00:00:01
# Gets stored inbetween the Master Boot Record (MBR) and
# the first partition of the eMMC on the BeagleBone Black
saveenv
```

```
# From u-boot run
ping 192.168.1.1

# From host machine
# Use NetWork Manager CLI to configure interface for board
$ nmcli con add type ethernet ifname enxf8dc7a000001 ip4 192.168.1.1/24

# Check if everything works
# From u-boot
tftp 0x81000000 zImage
```

Set boot arguments
```
setenv kern_load_addr 0x81000000
setenv dtb_load_addr 0x82000000
setenv bootargs console=ttyO0,115200n8 root=/dev/nfs rw ip=192.168.1.100:::::usb0 g_ether.dev_addr=f8:dc:7a:00:00:02 g_ether.host_addr=f8:dc:7a:00:00:01 nfsroot=192.168.1.1:/home/vince/linux-kernel-labs/modules/nfsroot,nfsvers=3,proto=tcp
setenv bootcmd 'tftp ${kern_load_addr} zImage; tftp ${dtb_load_addr} am335x-boneblack.dtb; bootz ${kern_load_addr} - ${dtb_load_addr}'
saveenv
```
