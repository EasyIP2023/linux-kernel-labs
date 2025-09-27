MODULE_NAME = hello_version

ifneq ($(KERNELRELEASE),)
	obj-m := $(MODULE_NAME).o
else
	KSRC ?= $(PWD)/linux
endif

# Bootlin Explanation
# The module Makefile is initially interpreted with KERNELRELEASE undefined, so
# it calls the kernel Makefile, passing the module directory in the M variable
# The kernel Makefile knows how to compile a module, and thanks to the M
# variable, knows where the Makefile for our module is. This module Makefile is
# then interpreted with KERNELRELEASE defined, so the kernel sees the obj-m
# definition.

all:
	$(MAKE) -C $(KSRC) M=$$PWD

modules_install:
	$(MAKE) -C $(KSRC) M=$$PWD modules_install

clean:
	$(MAKE) -C $(KSRC) M=$$PWD clean
