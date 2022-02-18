# -- makefile --

# kernel object
KOBJ = ./cfn_c641x.ko

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
# Comment/uncomment the following line to disable/enable debugging
DEBUG = y

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DFPGAENDPOINT_DEBUG # "-O" is needed to expand inlines
else
	DEBFLAGS = -O2
endif

CFLAGS += $(DEBFLAGS)
CFLAGS += -I$(LDDINC)

ifneq ($(KERNELRELEASE),)
	cfn_c641x_drv-objs := main.o # pipe.o access.o
	obj-m := cfn_c641x_drv.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC=$(PWD)/../include modules
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
