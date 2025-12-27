CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined

OBJS = rpsd.o rpsd_thread.o

all: rpsd

rpsd: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

rpsd.o: rpsd.c rpsd_thread.h rpsd_thread.o
	$(CC) $(CFLAGS) -c $<


rpsd_thread.o: rpsd_thread.c rpsd_thread.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *.o rpsd 
