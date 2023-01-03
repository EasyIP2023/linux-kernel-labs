# Lab 2 Device Model

Directory provides patch for in-tree & out-of-tree compilation of the
device tree and nunchuk kernel module.

**Create NFS Rootfs nunchuk Directory**
```sh
$ mkdir -p $HOME/linux-kernel-labs/modules/nfsroot/root/nunchuk/
```

**Device Tree Location**
```sh
$ find /sys/firmware/devicetree -name "*joystick*"
/sys/firmware/devicetree/base/ocp/interconnect@48000000/segment@0/target-module@2a000/i2c@0/joystick@52
# Run bellow to check the whole structure of the loaded device tree
$ dtc -I fs /sys/firmware/devicetree/base/ > /tmp/dts 2>/dev/null
```

**Check Driver Syntax Follows Standards**
```sh
$ ../../linux/scripts/checkpatch.pl --fix nunchuk.c
```
