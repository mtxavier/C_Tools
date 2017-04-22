CC = gcc
#CFLAGS = -g -Iinc -I../inc  -I/usr/include/freetype2
CFLAGS = -g -O2 -Iinc -I../inc  -I/usr/include/freetype2
LDFLAGS = -lGLEW -lGLU -lGL -lglut -lSDL -lfreetype -lrt -lpng -lgif -ltiff -ljpeg -lncurses

vpath  %.c src src/OpenGL
vpath  %.h inc:../inc
vpath  %.o obj:../obj

Gui_h = Geo.h Gui.h Gui.local.h
Core_h = Classes.h StackEnv.h Tools.h
Gui_Term_h = Geo.h Gui/GuiKeyMap.h Gui.Term.h GuiWidget.h Gui.Core.h

GuiImgLoad.o : Classes.h StackEnv.h Tools.h $(Gui_h)
Field.o : Classes.h StackEnv.h $(Gui_h) GeoField.h
Gui.o : Classes.h StackEnv.h $(Gui_h)
GuiField.o : Classes.h StackEnv.h $(Gui_h) GeoField.h GuiText.h
GuiFont.o : Classes.h StackEnv.h $(Gui_h) GuiText.h
GuiStyle.o : Classes.h StackEnv.h $(Gui_h) GuiText.h
GuiPalette.o : Classes.h StackEnv.h $(Gui_h)
GuiLibrary.o : Classes.h StackEnv.h $(Gui_h) List.h
GuiSdl.o : GuiKeyMap.h Classes.h StackEnv.h $(Gui_h) List.h GeoField.h
GuiFreeType.o : Classes.h StackEnv.h $(Gui_h) List.h GuiText.h
Posx.Time.o : GuiTime.h
GeoText.o : Classes.h StackEnv.h GeoText.h $(Gui_h)
GuiOgl21.o : Classes.h StackEnv.h $(Gui_h) GeoField.h List.h Tools.h

Curse.test.o : $(Core_h) $(Gui_Term_h)
Gui.Core.o : $(Core_h) Gui.Core.h
Gui.Term.o : $(Gui_Term_h) $(Core_h)
GuiWidget.o : $(Gui_Term_h) $(Core_h)
Curse.Term.o : $(Core_h) $(Gui_Term_h)
Gui.Term.Widget.o : $(Core_h) $(Gui_Term_h)
Input.Test.o : $(Gui_Term_h) $(Core_h)
Curse.test : StackEnv.o Curse.Term.o
Input.Test : Core.o Gui.Core.o Gui.Term.o Curse.Term.o GuiWidget.o Gui.Term.Widget.o

first.test.o : Classes.h StackEnv.h $(Gui_h)
Text.test.o : Classes.h StackEnv.h GeoText.h $(Gui_h)
Sdl.first.test.o : $(Gui_h)
Sdl.Text.test.o : $(Gui_h)
Ogl.first.test.o : $(Gui_h)
Ogl.Text.test.o : $(Gui_h)
Field.Test.o : Classes.h StackEnv.h GeoField.h $(Gui_h)
GuiLibrary.Test.o : Classes.h StackEnv.h $(Gui_h)
GuiOgl21.test : StackEnv.o GuiOgl21.test.o

Sdl.first.test: Sdl.first.test.o StackEnv.o List.o Gui.o GuiSdl.o GuiLibrary.o GuiPalette.o Geo.Shape.o Posx.Time.o GuiImgLoad.o first.test.o
Sdl.Text.test : Sdl.Text.test.o StackEnv.o List.o Gui.o GuiSdl.o Field.o GuiPalette.o GuiFont.o Geo.Shape.o\
 GuiStyle.o GuiField.o GuiLibrary.o GuiFreeType.o Posx.Time.o GuiImgLoad.o Text.test.o
Ogl.first.test: Ogl.first.test.o StackEnv.o List.o Gui.o GuiOgl21.o GuiLibrary.o GuiPalette.o Geo.Shape.o Posx.Time.o GuiImgLoad.o first.test.o Tools.o Patricia.o Key.o
Ogl.Text.test : Ogl.Text.test.o StackEnv.o List.o Gui.o GuiOgl21.o Field.o GuiPalette.o GuiFont.o Geo.Shape.o\
 GuiStyle.o GuiField.o GuiLibrary.o GuiFreeType.o Posx.Time.o GuiImgLoad.o Text.test.o Tools.o Patricia.o Key.o
GuiPalette.Test : GuiPalette.Test.o StackEnv.o GuiPalette.o
GuiLibrary.Test : GuiLibrary.Test.o GuiLibrary.o StackEnv.o List.o Gui.o GuiImgLoad.o GuiPalette.o Geo.Shape.o

Field.Test : Field.Test.o Field.o StackEnv.o List.o

Ogl : Ogl.first.test Ogl.Text.test GuiOgl21.test
	mv *.o obj

all : Sdl.first.test Field.Test GuiField.o GuiStyle.o GuiPalette.o GuiPalette.Test GuiOgl21.o\
 GuiFont.o Sdl.Text.test GuiLibrary.o GuiLibrary.Test GuiFreeType.o Geo.Shape.o\
 GeoText.o Text.test.o first.test.o Curse.test Input.Test
	mv *.o obj

clean :
	rm obj/*.o

