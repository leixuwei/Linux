ROOT_PATH=$(shell pwd)
CONF=$(ROOT_PATH)/conf
PLUGIN=$(ROOT_PATH)/plugin
WWWROOT=$(ROOT_PATH)/wwwroot
CC=g++
FLAGS=-D_STDOUT_   #-D_DEBUG_
LDFLAGS=-lpthread  #-static
bin=httpd
src=httpd.c main.c

obj=$(shell echo $(src) |  sed 's/\.c/\.o/g')

$(bin):$(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -c $< $(FLAGS)

.PHONY:clean
clean:
	rm *.o $(bin)

.PHONY:dubug
debug:

	echo $(obj)
	echo $(src)
	echo $(ROOT_PATH)
