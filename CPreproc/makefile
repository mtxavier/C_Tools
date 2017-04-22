CC = gcc
CFLAGS = -g -O2 -Iinc -I../inc 

vpath  %.c src
vpath  %.h inc:../inc
vpath  %.o obj:../obj

BigIntExpression.o : Classes.h BigValue.h StackEnv.h
CtTrace.o : CtTrace.h StackEnv.h Classes.h
C_Lexic.o : C_Preproc.h Classes.h StackEnv.h
C_Lexic.Test.o : C_Preproc.h Classes.h StackEnv.h
C_PrepBuffer.o : C_Preproc.h C_Util.h Classes.h StackEnv.h
C_MacroParse.o : C_Preproc.h C_Lexic.h BigValue.h StackEnv.h Classes.h
C_MacroParse.Test.o : StackEnv.h Classes.h C_Lexic.h BigValue.h
C_Lexic.Test : C_Lexic.Test.o C_Lexic.o C_PrepBuffer.o C_MacroParse.o \
	StackEnv.o BuffText.o List.o Patricia.o Key.o Debug.o CtTrace.o \
	BigValue.o BigIntExpression.o 
C_MacroParse.Test : C_MacroParse.Test.o C_Lexic.o StackEnv.o BuffText.o List.o Patricia.o Key.o \
	C_PrepBuffer.o C_MacroParse.o BigIntExpression.o BigValue.o CtTrace.o  CtTrace.o

all : C_Lexic.Test C_MacroParse.Test
	mv *.o obj

clean :
	rm obj/*.o

