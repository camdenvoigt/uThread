CC = gcc
CFLAGS = -I.
DEPS = uThread.h
OBJ = main.o uThread.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

uThread: $(OBJ)
	$(CC) -o -pthread $@ $^ $(CFLAGS)

debug: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -g

clean: 
	rm uThread debug $(OBJ)