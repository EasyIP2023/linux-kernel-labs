# Bootlin Linux Kernel Practice

**Useful Links**

- [Bootlin Training Material](https://github.com/bootlin/training-materials)
- [Updating kernel and bootloader on BeagleBone Black Wireless](https://krinkinmu.github.io/2020/07/05/beaglebone-software-update.html)
- [Creating Rootfs for BeagleBone Black](https://embedjournal.com/custom-rfs-beaglebone-black/)

**tftp server**

```sh
$ sudo apt install tftpd-hpa
$ mkdir -p $(pwd)/tftp-server-files
$ sudo sed -i "s@/srv/tftp@$(pwd)/tftp-server-files@g" \
              /etc/default/tftpd-hpa
$ sudo /etc/init.d/tftpd-hpa restart
```

**Cross Compiler tools may be found in docker container**

[Underview Kernel Dev (arm-gnueabihf)](https://github.com/under-view/docker-builds/blob/master/containers/kern-devel/arm-gnueabihf/Dockerfile)

**Creating NFS root filesystem**

Could also just generate one with yocto.

```sh
$ mkdir -p "$(pwd)/modules/nfsroot"


# Checkout to stable branch if so choose
$ git clone -b 1_37_stable git://git.busybox.net/busybox.git ; cd busybox


$ make defconfig
$ make menuconfig
# Networking Utilities -> tc (8.3 kb) | CONFIG_TC=n
# Settings -> Build static binary (no shared libs) | CONFIG_STATIC=y
# Settings -> (./_install) Destination path for 'make install' | CONFIG_PREFIX="../modules/nfsroot"


# Taken from: https://github.com/mirror/busybox/issues/104#issuecomment-2993736445
$ cat >> busybox-stable.patch << EOF
From 12baa36320ecd27cb8bbdb3ff029259bcd571d7c Mon Sep 17 00:00:00 2001
From: UlinKot <denis2005991@gmail.com>
Date: Sat, 21 Jun 2025 21:22:16 +0300
Subject: [PATCH] build fix: error occurring on non-i386, x86_64 systems when
 using `sha1_process_block64`

---
 libbb/hash_md5_sha.c | 2 ++
 1 file changed, 2 insertions(+)

diff --git a/libbb/hash_md5_sha.c b/libbb/hash_md5_sha.c
index 57a8014..75a61c3 100644
--- a/libbb/hash_md5_sha.c
+++ b/libbb/hash_md5_sha.c
@@ -1313,7 +1313,9 @@ unsigned FAST_FUNC sha1_end(sha1_ctx_t *ctx, void *resbuf)
 	hash_size = 8;
 	if (ctx->process_block == sha1_process_block64
 #if ENABLE_SHA1_HWACCEL
+# if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
 	 || ctx->process_block == sha1_process_block64_shaNI
+# endif
 #endif
 	) {
 		hash_size = 5;
-- 
2.49.0
EOF

$ git apply busybox-stable.patch
$ make -j $(nproc)
$ make install
$ cd .. ; rm -rf busybox
```

Populating rootfs:

Busybox upon init will first look for `/etc/init.d/rcS` script, if it can’t find that then
it will look for `/etc/inittab`. Inittab file will mount the virtual filesystem's using
fstab. Also, it will have the command for getting login prompt and shell.

```sh
$ mkdir -pv modules/nfsroot/{dev,lib,usr/lib,proc,sys,root,etc,tmp}


# mknod /dev/<device file name> type major minor
# types:
# b      create a block (buffered) special file
# c, u   create a character (unbuffered) special file
# p      create a FIFO
# major minor device numbers associated with character device can be found here
# https://www.kernel.org/doc/html/latest/admin-guide/devices.html
# major: typically indicates the family of devices (Bootlin Slides)
# minor: allows drivers to distinguish the various devices they manage (Bootlin Slides)
$ sudo mknod modules/nfsroot/dev/console c 5 1
$ sudo mknod modules/nfsroot/dev/null c 1 3
$ sudo mknod modules/nfsroot/dev/zero c 1 5
$ sudo chown -v $USER:$USER modules/nfsroot/dev/*


$ cat >> modules/nfsroot/etc/inittab << EOF
null::sysinit:/bin/mount -a
null::sysinit:/bin/hostname -F /etc/hostname
null::respawn:/bin/cttyhack /bin/login root
null::restart:/sbin/reboot
EOF

$ cat >> modules/nfsroot/etc/fstab << EOF
proc  /proc proc  defaults  0 0
sysfs /sys  sysfs defaults  0 0
tmpfs /tmp  tmpfs defaults,nodev,nosuid,size=1G 0  0
EOF

$ cat >> modules/nfsroot/etc/hostname << EOF
great_embedded
EOF

$ cat >> modules/nfsroot/etc/passwd << EOF
root::0:0:root:/root:/bin/sh
EOF


$ cp -raL /usr/arm-linux-gnueabihf/lib/* modules/nfsroot/lib
$ cp -raL /usr/arm-linux-gnueabihf/lib/* modules/nfsroot/usr/lib
```

```sh
# Checkout to stable branch if so choose
$ git clone https://git.kernel.org/pub/scm/utils/dtc/dtc.git ; cd dtc
$ meson setup --prefix="$(pwd)/../modules/nfsroot" \
              --cross-file="$(pwd)/../arm-linux-gnueabihf-meson-cross-file.txt" \
              -Dtests="false" \
              build
$ ninja -C build
$ ninja -C build install
$ cd .. ; rm -rf dtc
```

```sh
# Checkout to stable branch if so choose
$ git clone https://gitlab.freedesktop.org/libevdev/evtest.git ; cd evtest
$ ./autogen.sh --prefix="$(pwd)/../modules/nfsroot" --host="${CROSS_COMPILE}"
$ make -j$(nproc)
$ make install
$ cd .. ; rm -rf evtest
```

```sh
$ pkgname="ncurses" ; pkgver=6.5
$ wget https://invisible-mirror.net/archives/${pkgname}/${pkgname}-${pkgver}.tar.gz{,.asc}
$ tar xfv "${pkgname}-${pkgver}.tar.gz" ; cd "${pkgname}-${pkgver}"
$ mkdir -p build ; cd build


# For odd reasons configure script errors out if external variables set
$ unset CC CPP CXX
$ cross_compile="${CROSS_COMPILE%?}"
$ ../configure --prefix="$(pwd)/../../modules/nfsroot" \
               --with-pkg-config-libdir="$(pwd)/../../modules/nfsroot/lib/pkgconfig" \
               --target="${cross_compile}" \
               --host="${cross_compile}" \
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
               --disable-stripping \
               --with-build-cc="gcc -D_GNU_SOURCE"
$ make -j$(nproc)
$ make install


$ cd ../../ ; rm -rf "${pkgname}-${pkgver}"*
$ sed -i "s#${pkgname}-${pkgver}/build/../../##g" \
         $(pwd)/modules/nfsroot/lib/pkgconfig/*.pc
$ unset pkgname pkgver cross_compile
$ source /etc/profile.d/compile-env.sh
$ cd $(pwd)/modules/nfsroot/lib/pkgconfig
$ ln -sf ncursesw.pc ncurses.pc
```

```sh
$ git clone https://github.com/sf-refugees/ninvaders.git ; cd ninvaders
$ git apply "$(pwd)/../patches/0001-cross-compile-ninvaders.patch"
$ export PKG_CONFIG_PATH="$(pwd)/../modules/nfsroot/lib/pkgconfig:$PKG_CONFIG_PATH"


$ cmake -G Ninja \
        -S "$(pwd)" \
        -B "$(pwd)/build" \
        -DCMAKE_INSTALL_PREFIX="$(pwd)/../modules/nfsroot"
$ cmake --build "$(pwd)/build" -j$(nproc)
$ cmake --build "$(pwd)/build" --target install


$ cd .. ; rm -rf ninvaders
$ unset PKG_CONFIG_PATH
```

**Installing NFS Server**

```
$ sudo apt install nfs-kernel-server
$ echo "<path to>/linux-kernel-labs/modules/nfsroot 192.168.0.100(rw,no_root_squash,no_subtree_check)" | sudo tee -a /etc/exports
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
# From host machine
$ sudo cat >> /etc/systemd/network/70-bb-usb-ether.network << EOF
[Match]
Name=enxf8dc7a000001
MACAddress=f8:dc:7a:00:00:01

[Network]
Address=192.168.0.1/24
Gateway=192.168.0.1
EOF
$ sudo systemctl restart systemd-networkd
$ echo "check if net copy" > tftp-server-files/testing.txt


# From u-boot run
ping $serverip
# Check if everything works
tftp 0x81000000 testing.txt
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
# CONFIG_INPUT_JOYDEV=y
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
