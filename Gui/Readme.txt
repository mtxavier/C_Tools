This is part of the Fondement Michtam project

This is intended to be the Gui part.

Various backend libraries might be supported in the end, but for the moment, only Sdl is supported.
(The reason why is because no image loader for various image format has been written yet, so we use the facilities offered by Sdl).

The Gui is fairly limited for the moment.
Images are supposed to be abstractly described instead of procedurally generated.
(This is so that that Gui has at least some purpose instead of yet another architecture independant interface).

For the moment only 2d primitives are proposed.
    * Use the various GuiOutput creation primitives or use your own to describe your image.
    * Send it to a gui engine so it can be drawn.
    * The Gui Engine is our back-end. For the moment, we only have the Sdl engine.

"Gui.h" is supposed to tells exactly what primitives are available for the application, and thus what a backend has to implement.

Gui offers services of its own:
    * Interface to the freetype library. So various fonts are available to the application.
    * Textbox and log windows. For the moment, I haven't found a good software interface for it, however. Since the various options are very numerous, it is hard to setup. (Something like a style sheet is definitely needed).

---------------------------------------------

On linux:

Compilation:

This is not as straight forward as the other modules (that's part of the genre, really). You will need a numbers of outside libraries:
   * OpenGl ->  GLU, glut
   * Sdl -> SDL, SDL_image
   * The freetype library
   * We also use the font "/usr/share/fonts/X11/misc/7x14.pcf.gz" in GuiTTFontLoad
As a side note, I also use a iso_8859-1.png image in the data directory, as an alternate font source. But I can't remember where that image comes from, so I can't give credit where credit is due.

Dependencies: as usual, you should first build the previous directory before building this one.
   * in the previous directory type
       make all
   * Then in this directory, type
       make all
Various test application are created. Use 'q' or 'Q' to quit the applications. You might notice that to test a gui api, graphic backend is not always needed. [Actually managing various internal structures is often the more delicate problem].

