CC = gcc
CFLAGS = -I. -pthread -fstack-protector-all
DEPS = uThread.h
OBJ = main.o uThread.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

uThread: main.o uThread.o
	$(CC) -o $@ $^ $(CFLAGS)

memleaks: memoryleaks.o uThread.o
	$(CC) -o $@ $^ $(CFLAGS) -g

debug: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -g

clean:
	rm uThread debug $(OBJ)
