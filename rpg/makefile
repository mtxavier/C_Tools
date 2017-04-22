CC = gcc
CFLAGS = -g -Iinc -I../inc -I../Schedule/inc
M4FLAGS = -I../m4inc
LDFLAGS = -lpthread -lrt

vpath  %.c src
vpath  %.h inc:../inc
vpath  %.o .:..:../Schedule
vpath  %.html doc
vpath  %.htm4 doc
vpath  %.m4 ../m4inc


Core_h = Classes.h StackEnv.h Tools.h
m4core = LexFR.m4 core.m4 HTML.m4

obj/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.html: %.htm4 
	m4 $(M4FLAGS) $< >doc/$@

Items.html : Items.htm4
obj/Skills.o : RpgCore.h Browser.h $(Core_h)
obj/Skills.test.o : RpgCore.h $(Core_h)
obj/d3d6.o : d3d6.h $(Core_h)
obj/d3d6.calc.o : d3d6.h $(Core_h)
obj/RpgCore.o : RpgCore.h Browser.h Tools.h $(Core_h)
obj/RpgAltrnCore.o : RpgCore.h Tools.h $(Core_h)
d3d6.calc : obj/d3d6.o obj/Core.o
Skills.test : obj/Skills.o obj/RpgCore.o obj/Core.o
Cmbt.test : obj/Skills.o obj/RpgCore.o obj/Core.o
RpgAltrn.Test : obj/RpgCore.o obj/Skills.o obj/RpgAltrnCore.o obj/Core.o

.PHONY : clean
clean :
	rm obj/*.o
	rm doc/*.html

.PHONY : all
all : d3d6.calc Items.html Skills.test RpgAltrn.Test Cmbt.test

