# Arch = A_i386
Arch = A_x86-64
CC = gcc
CFLAGS = -g  -Iinc -I../inc -Iinc/$(Arch) -Isrc/A_linux
LDFLAGS = -lpthread
# CFLAGS = -g -Iinc -I../inc -Iinc/$(Arch)
vpath  %.c src:src/$(Arch)
vpath  %.h inc:../inc:inc/$(Arch):src/A_linux
vpath  %.o obj:../obj
vpath  %.s src:src/$(Arch)

RawSwitch.o :
RawStack.o : RawSwitch.h A_Stack.h StackEnv.h
RawSwitch.C.o : Classes.h RawSwitch.h A_Stack.h StackEnv.h
Thd.o : Classes.h RawSwitch.h A_Stack.h Thd.h StackEnv.h OsThd.c.h

NativeFrame.o : StackEnv.h Classes.h
Thread.Test.o : Classes.h StackEnv.h
Thread.Test : Thread.Test.o RawSwitch.o Thd.o StackEnv.o \
		NativeFrame.o RawStack.o

all : Thread.Test RawSwitch.o RawSwitch.C.o Thd.o RawStack.o
	mv *.o obj

clean :
	rm obj/*.o

