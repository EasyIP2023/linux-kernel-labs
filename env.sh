#!/bin/bash

CDIR="$(pwd)"
export PATH="${CDIR}/gcc-arm/bin:$PATH"
export ARCH="arm"
export CROSS_COMPILE="arm-linux-gnueabihf-"
