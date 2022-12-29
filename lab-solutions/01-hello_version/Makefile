MODULE_NAME = hello_version

ifneq ($(KERNELRELEASE),)
obj-m := $(MODULE_NAME).o
else
KDIR := $(PWD)/../../linux
endif

all:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	$(RM) $(PWD)/*.o
	$(RM) $(PWD)/*.ko
	$(RM) $(PWD)/*.order
	$(RM) $(PWD)/*.symvers
	$(RM) $(PWD)/*.mod*
	$(RM) $(PWD)/.mod*
	$(RM) $(PWD)/.Mod*
	$(RM) $(PWD)/.$(MODULE_NAME)*
