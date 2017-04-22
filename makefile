CC = gcc
#CFLAGS = -g -O2 -Iinc
CFLAGS = -g -Iinc
LDFLAGS = -lpthread -lrt
vpath  %.c src:src/A_linux
vpath  %.s src
vpath  %.h inc
vpath  %.o .

obj/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

obj/StackEnv.o : StackEnv.h List.h Classes.h
obj/Patricia.o : Patricia.h
obj/Key.o : Key.h Patricia.h
obj/List.o : List.h StackEnv.h
obj/BigValue.o : StackEnv.h
obj/BuffText.o : StackEnv.h Classes.h
obj/Geo.Shape.o : StackEnv.h Classes.h Geo.h 
obj/Debug.o : Classes.h StackEnv.h
obj/Browser.o : StackEnv.h Classes.h Browser.h Browser.cmn.c
obj/Browser.zip.o : StackEnv.h Classes.h Tools.h Browser.h
obj/Geo.o : StackEnv.h Classes.h Tools.h List.h Geo.h
obj/Geo.Dyna3d.o : StackEnv.h Classes.h Tools.h List.h Geo.h TlsSched.h
obj/TlsSched.o : StackEnv.h Classes.h Tools.h List.h TlsSched.h Patricia.h
obj/TlsThdActor.o : StackEnv.h Classes.h TlsSched.h Thd.h

obj/Tools.1.o : Classes.h StackEnv.h List.h
obj/Tools.2.o : Classes.h StackEnv.h List.h
obj/Tools.4.o : Classes.h StackEnv.h List.h
obj/Tools.5.o : Classes.h StackEnv.h List.h
obj/Tools.IntVal.o : Tools.h Classes.h StackEnv.h List.h
obj/Tools.UTF8.o : Tools.h
TOOLS_O = obj/Tools.1.o obj/Tools.2.o obj/Tools.4.o obj/Tools.5.o obj/Tools.UTF8.o obj/Tools.IntVal.o obj/TlsSched.o
obj/Tools.o : $(TOOLS_O)
	ld -i -o obj/Tools.o $(TOOLS_O)

CORE_O = obj/StackEnv.o obj/Patricia.o obj/List.o obj/BigValue.o obj/Browser.o obj/Tools.o obj/BuffText.o
obj/Core.o : $(CORE_O)
	ld -i -o obj/Core.o $(CORE_O)

obj/List.test.o : List.h StackEnv.h Classes.h
obj/Tools.4.Test.o : Classes.h StackEnv.h Tools.h
obj/Tools.5.Test.o : Classes.h StackEnv.h Tools.h
obj/Tools.tmp.o : Classes.h StackEnv.h Tools.h Tools.1.c Tools.2.c Tools.4.c
obj/Key.Test.o : Key.h
obj/Browser.test.o : StackEnv.h Classes.h Browser.h
obj/StackEnv.Test.o : StackEnv.h List.h Classes.h
obj/BigValue.Test.o : StackEnv.h BigValue.h
obj/Tools.Test.o : Classes.h StackEnv.h Tools.h
obj/TlsBuffer.Test.o : Classes.h StackEnv.h Tools.h
obj/Tls.Sched.Test.o : Classes.h StackEnv.h Tools.h TlsSched.h
obj/Geo.Bndry.Test.o : Classes.h StackEnv.h Tools.h Geo.h List.h
obj/Basic.Test.o : Tools.h

List.Test : obj/List.Test.o obj/List.o obj/StackEnv.o
	gcc -g -o List.Test obj/List.Test.o obj/List.o obj/StackEnv.o
BigValue.Test : obj/BigValue.Test.o obj/StackEnv.o obj/BigValue.o obj/List.o
	gcc -g -o BigValue.Test obj/BigValue.Test.o obj/StackEnv.o obj/BigValue.o obj/List.o
StackEnv.Test : obj/StackEnv.Test.o obj/StackEnv.o
	gcc -g -o StackEnv.Test obj/StackEnv.Test.o obj/StackEnv.o
Key.Test : obj/Key.Test.o obj/Key.o obj/Patricia.o
	gcc -g -o Key.Test obj/Key.Test.o obj/Key.o obj/Patricia.o
Tls.Sched.Test : obj/Core.o obj/Tls.Sched.Test.o
	gcc -g -o Tls.Sched.Test obj/Tls.Sched.Test.o obj/Core.o
Geo.Bndry.Test : obj/Core.o obj/Geo.Bndry.Test.o obj/Geo.o
	gcc -g -o Geo.Bndry.Test obj/Geo.Bndry.Test.o obj/Core.o obj/Geo.o
TLSBUFFER_TEST = obj/TlsBuffer.Test.o obj/Tools.o obj/StackEnv.o obj/List.o obj/Patricia.o
TlsBuffer.Test : $(TLSBUFFER_TEST)
	gcc -g -o TlsBuffer.Test $(TLSBUFFER_TEST)
TOOLS_TEST = obj/Tools.Test.o obj/StackEnv.o obj/List.o obj/Tools.o obj/Patricia.o
Tools.Test : $(TOOLS_TEST)
	gcc -g -o Tools.Test $(TOOLS_TEST)
WordCount.Test : obj/Patricia.o obj/StackEnv.o obj/List.o
TOOLS_4_TEST = obj/Tools.4.Test.o obj/StackEnv.o obj/List.o obj/Tools.o obj/Patricia.o
Tools.4.Test : $(TOOLS_4_TEST)
	gcc -g -o Tools.4.Test $(TOOLS_4_TEST)
TOOLS_5_TEST = obj/Tools.5.Test.o obj/StackEnv.o obj/List.o obj/Tools.o obj/Patricia.o
Tools.5.Test : $(TOOLS_5_TEST)
	gcc -g -o Tools.5.Test $(TOOLS_5_TEST)
Tools.tmp : obj/Tools.tmp.o obj/StackEnv.o obj/List.o
	gcc -g -o Tools.tmp obj/Tools.tmp.o obj/StackEnv.o obj/List.o
Browser.test : obj/Browser.test.o obj/Core.o
	gcc -g -lrt -o Browser.test obj/Browser.test.o obj/Core.o

.PHONY : all
all : obj/List.o obj/StackEnv.o obj/Patricia.o obj/Key.o obj/Debug.o obj/BigValue.o obj/Browser.o \
obj/Browser.zip.o obj/Geo.Shape.o obj/BuffText.o obj/Tools.o obj/Geo.o obj/TlsSched.o obj/TlsThdActor.o \
obj/Geo.Dyna3d.o \
BigValue.Test StackEnv.Test Key.Test TlsBuffer.Test Tools.Test Tls.Sched.Test Geo.Bndry.Test\
Tools.4.Test Tools.5.Test Tools.tmp List.Test Browser.test Basic.Test \
WordCount.Test

.PHONY : clean
clean:
	rm obj/*.o
