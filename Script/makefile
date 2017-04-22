CC = gcc
CFLAGS = -g -Iinc -I../inc 

vpath  %.c src
vpath  %.h inc:../inc
vpath  %.o obj:../obj:../CPreproc/obj

Core_h = Classes.h StackEnv.h
ScrType_h = ScrType.h ScrBuilder.h
ScrDataExpr_h  = ScrDataExpr.h $(ScrType_h)
ScrModule_h = ScrModule.h $(ScrDataExpr_h)
DataExpr_Loc_h = DataInstr.Loc.h DataTgt.Loc.h

ScrType.o : $(Core_h) $(ScrType_h)
ScrDataExpr.o : $(Core_h) $(ScrDataExpr_h) $(DataExpr_Loc_h)
ScrNull.o : $(Core_h) $(ScrDataExpr_h)
ScrModule.o : $(Core_h) $(ScrModule_h) Builder.Loc.h
ScrDataVal.o : $(Core_h) $(ScrDataExpr_h)
ScrDataTgt.o : $(Core_h) $(ScrDataExpr_h) DataTgt.Loc.h
ScrInstruction.o : $(Core_h) $(ScrDataExpr_h) DataInstr.Loc.h
ScrBuilder.o : $(Core_h) ScrBuilder.h 
Parser.o : $(Core_h) $(ScrModule_h) ScrPathExpr.h
Display.o : $(Core_h) $(ScrModule_h)
ScrType.Test.o : $(Core_h) $(ScrModule_h)
ScrPathExpr.o : $(Core_h) $(ScrDataExpr_h) ScrPathExpr.h
ScrMapping.o : $(Core_h) $(ScrDataExpr_h)
ScrBuilder.Loc.o : $(Core_h) $(ScrDataExpr_h) Builder.Loc.h

ScrType.Test: ScrType.Test.o ScrBuilder.o Tools.o ScrType.o ScrDataExpr.o ScrModule.o Parser.o Display.o ScrNull.o  \
		BuffText.o C_Lexic.o C_MacroParse.o C_PrepBuffer.o CtTrace.o List.o Patricia.o StackEnv.o Key.o BigValue.o \
		BigIntExpression.o ScrDataVal.o ScrDataTgt.o ScrInstruction.o ScrPathExpr.o ScrMapping.o ScrBuilder.Loc.o

.PHONY : clean
clean :
	rm obj/*.o

.PHONY : all
all :  ScrDataExpr.o ScrType.o ScrModule.o Parser.o ScrBuilder.o ScrDataVal.o Display.o ScrNull.o \
		ScrType.Test.o ScrType.Test ScrDataTgt.o ScrInstruction.o ScrPathExpr.o ScrMapping.o ScrBuilder.Loc.o
	mv *.o obj

