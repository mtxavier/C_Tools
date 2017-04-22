CC = gcc
CFLAGS = -O2 -I../../inc -I../../inc/A_i386 -I../../../inc
vpath  %.c src
vpath  %.h ../../inc:../../../inc:../../inc/A_i386
vpath  %.s src

RawSwitch.C.s : RawSwitch.h Classes.h A_Stack.h
	$(CC) -S $(CFLAGS) -o RawSwitch.C.s RawSwitch.C.c
RawSwitch.s : RawSwitch.h Classes.h A_Stack.h RawSwitch.Modele.c
	$(CC) -S $(CFLAGS) -o RawSwitch.s RawSwitch.Modele.c

all : RawSwitch.s RawSwitch.C.s

clean : 

