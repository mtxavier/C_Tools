/*  Fondement Michtam
 *  Copyright (C) 2011 Xavier Lacroix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <StackEnv.h>
#include <Classes.h>
#include <Gui.h>
#include <stdio.h>


main () {
	GuiRgbPalette *Palette;
	GuiRGBA col;
	int i,j;
	EnvOpen(4096,4096);
    Palette = GuiFixed256Palette();
	printf("\n");
	for (i=0;i<256;i++) {
		Call(Palette,GetRGBA,2(&col,i));
		j = Call(Palette,GetColor,1(&col));
		printf("%d -> {%d,%d,%d | %d} -> %d\n",i,col.r,col.g,col.b,col.a,j);
	}
	printf("\n");
	EnvClose();
}

