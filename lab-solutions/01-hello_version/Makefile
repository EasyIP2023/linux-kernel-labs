ifneq ($(KERNELRELEASE),)
obj-m := hello_version.o
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

