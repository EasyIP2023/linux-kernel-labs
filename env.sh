#!/bin/bash

CDIR="$(pwd)"
export PATH="${CDIR}/gcc-arm/bin:$PATH"
export ARCH="arm"
export CROSS_COMPILE="arm-linux-gnueabihf-"

# For Busybox
export CONFIG_PREFIX="$(pwd)/modules/nfsroot"

cd $(pwd)/linux
