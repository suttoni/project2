TARG = elevator_mod
elevator_mod-objs := elevater_syscalls.o elevator_module.o elevator_passenger.o
obj-m := $(TARG).o

KDIR :=/lib/modules/4.2.0/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
