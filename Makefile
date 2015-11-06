TARG = elevator_mod
elevator_mod-objs := module_data.o elevator_module.o elevator_passenger.o
obj-m := $(TARG).o
obj-y := elevater_syscalls.o

KDIR :=/lib/modules/4.2.0/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
