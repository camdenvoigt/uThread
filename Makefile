CC = gcc
CFLAGS = -I. -pthread -fstack-protector-all
DEPS = uThread.h
OBJ = main.o memoryleaks.o bufferoverflow.o uThread.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: main.o uThread.o
	$(CC) -o $@ $^ $(CFLAGS)

memleaks: memoryleaks.o uThread.o
	$(CC) -o $@ $^ $(CFLAGS) -g

bufferoverflow: bufferoverflow.o uThread.o
	$(CC) -o $@ $^ $(CFLAGS) -g

clean:
	rm uThread memleaks bufferoverflow $(OBJ)
