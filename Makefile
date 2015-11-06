MODULE_NAME = elevator

PWD := $(shell pwd)


#KDIR := /lib/modules/$(shell uname -r)/build

KDIR := /lib/modules/4.2.0/build

ccflags-y += -I$(src)/include

obj-y := elevator_syscalls.o

$(MODULE_NAME)-objs += elevator_module.o 
$(MODULE_NAME)-objs += elevator_passenger.o
$(MODULE_NAME)-objs += module_data.o

obj-m := $(MODULE_NAME).o


default:

	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:

	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
