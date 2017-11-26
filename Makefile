CC = gcc
CFLAGS = -I. -pthread -fstack-protector-all
DEPS = uThread.h
OBJ = main.o uThread.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

uThread: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

debug: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -g

clean:
	rm uThread debug $(OBJ)
