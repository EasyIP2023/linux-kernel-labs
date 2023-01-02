#!/bin/bash

CDIR="$(pwd)"
export PATH="${CDIR}/gcc-arm/bin:$PATH"
export ARCH="arm"
export CROSS_COMPILE="arm-linux-gnueabihf-"
export CC="${CROSS_COMPILE}gcc"
export CXX="${CROSS_COMPILE}g++"
export CPP="${CXX}"
