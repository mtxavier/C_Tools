CC = gcc
CFLAGS = -g -Iinc -I../inc
LDFLAGS = -lpthread -lrt

vpath  %.c src
vpath  %.h inc:../inc
vpath  %.o obj:../obj:../Thread/obj

LstSched.o : StackEnv.h Classes.h Tools.h Schedule.h Thd.h List.h
Schedule.Test.o : StackEnv.h Classes.h Schedule.h

Schedule.Test : LstSched.o Core.o Thd.o RawStack.o RawSwitch.o

.PHONY : clean
clean :
	rm obj/*.o

.PHONY : all
all :  LstSched.o Schedule.Test
	mv *.o obj

