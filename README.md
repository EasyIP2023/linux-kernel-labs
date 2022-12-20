# Bootlin Linux Kernel Practice

**Useful Links**
- [Updating kernel and bootloader on BeagleBone Black Wireless](https://krinkinmu.github.io/2020/07/05/beaglebone-software-update.html)

**tftp server**
```sh
$ sudo apt install tftpd-hpa
# Update to TFTP_DIRECTORY variable
# TFTP_DIRECTORY="/home/vince/linux-kernel-labs/tftp-server-files"
$ mkdir -p $HOME/linux-kernel-labs/tftp-server-files
$ sudo vim /etc/default/tftpd-hpa
$ sudo /etc/init.d/tftpd-hpa restart
```

**Download + Extract Latest Cross Compiler Tools**
Latest toolchain can be found at [linaro.org](https://releases.linaro.org/components/toolchain/binaries/)
```
$ tar xvf ~/Downloads/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz -C $(pwd)
$ mv gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf gcc-arm
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
# Should now see 4 folders (usr,bin,etc,sbin) and a file (linuxrc)
```

Populating rootfs:
Busybox init will first look for `/etc/init.d/rcS` script, if it can’t find that then
it will look for `/etc/inittab`. Inittab file will mount the virtual filesystem using
fstab. Also, it will have the command for getting login prompt and shell.
```
$ cd modules/nfsroot
$ mkdir -p dev lib usr/lib proc sys root etc
$ sudo mknod dev/console c 5 1
$ sudo mknod dev/null c 1 3
$ sudo mknod dev/zero c 1 5
$ sudo chown -v vince:vince dev/*

$ cat >> etc/inittab
null::sysinit:/bin/mount -a
null::sysinit:/bin/hostname -F /etc/hostname
null::respawn:/bin/cttyhack /bin/login root
null::restart:/sbin/reboot
[CTRL-D]

$ cat >> etc/fstab
proc  /proc proc  defaults  0 0
sysfs /sys  sysfs defaults  0 0
[CTRL-D]

$ cat >> etc/hostname
great_grand_embedded
[CTRL-D]

$ cat >> etc/passwd
root::0:0:root:/root:/bin/sh
[CTRL-D]
```

Installing server to watch
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

Reset environment variables
```
env default -f -a
saveenv
```

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

**Building kernel**
```sh
$ git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux
$ cd linux
$ git remote add stable https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable
$ git fetch stable
# Choose stable branch to develop off of

$ . env.sh
$ make omap2plus_defconfig
# Add kernel config symbols
# CONFIG_USB_GADGET=y
# CONFIG_USB_MUSB_HDRC=y
# CONFIG_USB_MUSB_GADGET=y
# CONFIG_USB_MUSB_DSPS=y
# CONFIG_AM335X_PHY_USB=y
# CONFIG_USB_ETH=y
# CONFIG_USB_ETH=y
# CONFIG_KERNEL_LZO=y
$ make -j$(($(nproc)/2))
$ ls arch/${ARCH}/boot
$ ls arch/${ARCH}/boot/dts/*.dtb

# Copy over required files
$ cp -a arch/${ARCH}/boot/zImage $HOME/linux-kernel-labs/tftp-server-files
$ cp -a arch/${ARCH}/boot/dts/am335x-boneblack*.dtb $HOME/linux-kernel-labs/tftp-server-files
```
