obj-m += i2c_driver.o

DIR=/lib/modules/$(shell uname -r)/build

all:
	make -C $(DIR) M=$(shell pwd) modules

clean:
	make -C $(DIR) M=$(shell pwd) clean
