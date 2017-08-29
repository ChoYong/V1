# Set compiler to use
CC=g++
CFLAGS=-g -I. -fpermissive
LDFLAGS_TET=
LDFLAGS_CHAT=-lpthread
DEPS_TET=CTetris.h Tetris.h Matrix.h
DEPS_CHAT=bool.h colors.h log.h llist2.h
OBJS_TET=Main.o CTetris.o Tetris.o Matrix.o ttymodes.o
OBJS_CHAT=chatsrv.o llist2.o log.o
DEBUG=0

all:: chatsrv.exe Main.exe

Main.exe: $(OBJS_TET)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS_TET)

chatsrv.exe: $(OBJS_CHAT)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS_CHAT)

%.o: %.c $(DEPS_CHAT)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS_TET)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -f chatsrv.exe Main.exe 
	rm -f *.o *~ *.stackdump
