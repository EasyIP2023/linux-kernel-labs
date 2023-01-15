# Bootlin Linux Kernel Practice

**Useful Links**
- [Bootlin Training Material](https://github.com/bootlin/training-materials)
- [Updating kernel and bootloader on BeagleBone Black Wireless](https://krinkinmu.github.io/2020/07/05/beaglebone-software-update.html)
- [Creating Rootfs for BeagleBone Black](https://embedjournal.com/custom-rfs-beaglebone-black/)

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
```sh
$ tar xvf ~/Downloads/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz -C $(pwd)
$ mv gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf gcc-arm
$ cp -rav gcc-arm/include/* gcc-arm/lib/gcc/arm-linux-gnueabihf/7.5.0/plugin/include/
# ncurses make install uses strip command.
# Simple fix to ensure we are using arm-linux-gnueabihf- implementation
# and not the system implementation
$ cd gcc-arm/bin
$ ln -sfv arm-linux-gnueabihf-strip strip
```

**Enter Dev Environment**
```sh
$ . ./env.sh
```

**Creating NFS root filesystem**
```sh
$ mkdir -p "${CDIR}/tmp" ; cd "${CDIR}/tmp"
# Checkout to stable branch if so choose
$ git clone git://git.busybox.net/busybox.git ; cd busybox
$ make defconfig
# CONFIG_STATIC=y (Settings > Build static binary (no shared libs))
# CONFIG_PREFIX="$HOME/linux-kernel-labs/modules/nfsroot" (Settings > Destination path for 'make install')
$ make -j $(nproc)
$ make install
# Should now see 4 folders (usr,bin,etc,sbin) and a file (linuxrc)
```

Populating rootfs:

Busybox upon init will first look for `/etc/init.d/rcS` script, if it can’t find that then
it will look for `/etc/inittab`. Inittab file will mount the virtual filesystem's using
fstab. Also, it will have the command for getting login prompt and shell.

```sh
$ cd ${CDIR}/modules/nfsroot
$ mkdir -p dev lib usr/lib proc sys root etc tmp
# mknod /dev/<device file name> type major minor
# types:
# b      create a block (buffered) special file
# c, u   create a character (unbuffered) special file
# p      create a FIFO
$ sudo mknod dev/console c 5 1
$ sudo mknod dev/null c 1 3
$ sudo mknod dev/zero c 1 5
$ sudo chown -v $USER:$USER dev/*

$ cat >> etc/inittab
null::sysinit:/bin/mount -a
null::sysinit:/bin/hostname -F /etc/hostname
null::respawn:/bin/cttyhack /bin/login root
null::restart:/sbin/reboot
[CTRL-D]

$ cat >> etc/fstab
proc  /proc proc  defaults  0 0
sysfs /sys  sysfs defaults  0 0
tmpfs /tmp  tmpfs defaults,nodev,nosuid,size=1G 0  0
[CTRL-D]

$ cat >> etc/hostname
great_embedded
[CTRL-D]

$ cat >> etc/passwd
root::0:0:root:/root:/bin/sh
[CTRL-D]

$ cd $CDIR
$ cp -ra ${CDIR}/gcc-arm/arm-linux-gnueabihf/libc/lib/* ${CDIR}/modules/nfsroot/lib
$ cp -ra ${CDIR}/gcc-arm/arm-linux-gnueabihf/libc/lib/* ${CDIR}/modules/nfsroot/usr/lib/
```

```sh
$ mkdir -p "${CDIR}/tmp" ; cd "${CDIR}/tmp"
# Checkout to stable branch if so choose
$ git clone https://git.kernel.org/pub/scm/utils/dtc/dtc.git
$ cd dtc
$ meson setup --prefix="${CDIR}/modules/nfsroot" \
              --cross-file="${CDIR}/arm-linux-gnueabihf-meson-cross-file.txt" \
              build
$ ninja -C build
$ ninja -C build install
```

```sh
$ mkdir -p "${CDIR}/tmp" ; cd "${CDIR}/tmp"
# Checkout to stable branch if so choose
$ git clone https://gitlab.freedesktop.org/libevdev/evtest.git ; cd evtest
$ ./autogen.sh --prefix="${CDIR}/modules/nfsroot" --host="${COMPILER_PREFIX}"
$ make -j$(nproc)
$ make install
```

```sh
$ mkdir -p "${CDIR}/tmp" ; cd "${CDIR}/tmp"
$ pkgname="ncurses" ; pkgver=6.4
$ wget https://invisible-mirror.net/archives/${pkgname}/${pkgname}-${pkgver}.tar.gz{,.asc}
$ tar xfv "${pkgname}-${pkgver}.tar.gz" ; cd "${pkgname}-${pkgver}"
$ mkdir -p build ; cd build
# For odd reasons configure script errors out if external variables set
$ unset CC CPP CXX
$ ../configure --prefix="${CDIR}/modules/nfsroot" \
               --target="${COMPILER_PREFIX}" \
               --host="${COMPILER_PREFIX}" \
               --with-shared \
               --with-normal \
               --without-debug \
               --without-ada \
               --with-cxx-binding \
               --with-cxx-shared \
               --enable-ext-colors \
               --enable-ext-mouse \
               --enable-overwrite \
               --enable-pc-files \
               --with-build-cc="gcc -D_GNU_SOURCE"
$ make -j$(nproc)
$ make install
$ unset pkgname pkgver
# Remember to re-execute environment script
$ . env.sh
```

```sh
$ mkdir -p "${CDIR}/tmp" ; cd "${CDIR}/tmp"
$ git clone https://github.com/sf-refugees/ninvaders.git
$ git apply "${CDIR}/patches/0001-cross-compile-ninvaders.patch"
$ cd ninvaders
$ cmake -G Ninja \
        -S "$(pwd)" \
        -B "$(pwd)/build" \
        -DCMAKE_INSTALL_PREFIX="${CDIR}/modules/nfsroot"
$ cmake --build "$(pwd)/build" -j$(nproc)
$ cmake --build "$(pwd)/build" --target install
```

**Installing NFS Server**
```
$ sudo apt install nfs-kernel-server
$ sudo echo "$HOME/linux-kernel-labs/modules/nfsroot 192.168.0.100(rw,no_root_squash,no_subtree_check)" >> /etc/exports
```

**Restarting NFS server**
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
setenv ipaddr 192.168.0.100
setenv serverip 192.168.0.1
setenv usbnet_devaddr f8:dc:7a:00:00:02
setenv usbnet_hostaddr f8:dc:7a:00:00:01
# Gets stored inbetween the Master Boot Record (MBR) and
# the first partition of the eMMC on the BeagleBone Black
saveenv
```

```
# From u-boot run
ping 192.168.0.1

# From host machine
# Use NetWork Manager CLI to configure interface for board
$ nmcli con add type ethernet ifname enxf8dc7a000001 ip4 192.168.0.1/24

# Check if everything works
# From u-boot
tftp 0x81000000 zImage
```

Set boot arguments
```
setenv kern_load_addr 0x81000000
setenv dtb_load_addr 0x82000000
setenv bootargs console=ttyO0,115200n8 root=/dev/nfs rw ip=192.168.0.100:::::usb0 g_ether.dev_addr=f8:dc:7a:00:00:02 g_ether.host_addr=f8:dc:7a:00:00:01 nfsroot=192.168.0.1:/home/vince/linux-kernel-labs/modules/nfsroot,nfsvers=3,proto=tcp
setenv bootcmd 'tftp ${kern_load_addr} zImage; tftp ${dtb_load_addr} am335x-boneblack.dtb; bootz ${kern_load_addr} - ${dtb_load_addr}'
saveenv
```

**Building kernel**
```sh
$ git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux
$ cd linux
$ git remote add stable https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable
$ git fetch stable
# Choose stable branch to develop from

$ make omap2plus_defconfig
# Add kernel config symbols
# CONFIG_USB_GADGET=y
# CONFIG_USB_MUSB_HDRC=y
# CONFIG_USB_MUSB_GADGET=y
# CONFIG_USB_MUSB_DSPS=y
# CONFIG_NOP_USB_XCEIV=y
# CONFIG_AM335X_PHY_USB=y
# CONFIG_USB_ETH=y
# CONFIG_ROOT_NFS=y
# CONFIG_KERNEL_LZO=y
$ make -j$(($(nproc)/2))
$ ls arch/${ARCH}/boot
$ ls arch/${ARCH}/boot/dts/*.dtb

# Copy over required files
$ cp -a arch/${ARCH}/boot/zImage $HOME/linux-kernel-labs/tftp-server-files
$ cp -a arch/${ARCH}/boot/dts/*am335x-*boneblack*.dtb $HOME/linux-kernel-labs/tftp-server-files
```

**Kernel Modules**
```sh
$ sudo INSTALL_MOD_PATH="${CDIR}/modules/nfsroot" make modules_install
$ sudo chown -Rv $USER:$USER ${CDIR}/modules/nfsroot/lib/modules

# From embedded compute
$ modinfo hello_version
$ modprobe hello_version who="TRY ME NOW"
# Or
$ insmod /lib/modules/$(uname -r)/kernel/drivers/misc/hello_version.ko who="TRY ME NOW"
$ lsmod || cat < /proc/modules
$ rmmod hello_version

# Way to find parameters for a kernel module
$ ls /sys/module/hello_version/parameters/
# Can change parameter if module has write permissions
$ echo 0 > /sys/module/usb_storage/parameters/delay_use
```
