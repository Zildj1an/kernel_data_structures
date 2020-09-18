MODULE	 = datas

obj-m += $(MODULE).o
MY_CFLAGS += -g -DDEBUG
ccflags-y += ${MY_CFLAGS}
CC += ${MY_CFLAGS}

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all: $(MODULE)

%.o: %.c
	@echo "  CC      $<"
	@$(CC) -c $< -o $@

$(MODULE):
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
