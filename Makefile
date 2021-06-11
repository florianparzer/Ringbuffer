#Options
#-Wall		Turns on most compiler warnings
#-Wextra	Turns on more compiler warnings
#-pedantic	Turns on even more compiler warnings
#-Werror	Treats compiler warnings as errors

OPTIONS = -Wall -Wextra -pedantic
all: ringbuffer

ringbuffer: sender.o receiver.o
	gcc $(OPTIONS) -o sender sender.o -lrt -lpthread
	gcc $(OPTIONS) -o receiver receiver.o -lrt -lpthread

sender.o: sender.c buff.h
	gcc $(OPTIONS) -c sender.c

receiver.o: receiver.c buff.h
	gcc $(OPTIONS) -c receiver.c

clean:
	rm -f sender
	rm -f receiver
	rm -f *.o
