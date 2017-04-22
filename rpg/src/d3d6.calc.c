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

#include <Classes.h>
#include <StackEnv.h>
#include <d3d6.h>
#include <stdio.h>

/*____________________________________________________________
 |
 |
 |____________________________________________________________
*/

main() {
	int Score,kept;
	EnvOpen(4096,4096);
	printf("\n");
	for (kept=1;kept<6;kept++) {
		printf("\n------------------------\nKeeping %d dices:\n\n",kept);
	for (Score = -15; Score<=15; Score++) {
		int i,range,faces;
		double cumul;
		d3d6PopRepart *Rep;
		faces = 6; range = ((faces-1)*kept);
		rOpen
		Rep = d3d6ScoreCount(6,kept,Score);
		printf("\nScore %+dd (Population %.0f):\n    ",Score,Rep->population);
        for (i=0;i<=range;i++) printf("%6.2f ",(Rep->repart[i]*100.)/Rep->population);
		printf("\n    ");
		cumul = Rep->population;
		for (i=0;i<=range;i++) { printf("%6.2f",(cumul*100.)/Rep->population); cumul-=Rep->repart[i];}
		printf("\n    ");
		/* for (i=0;i<=range;i++) { printf("%6d  ",d3d6Check(Score,i)); }
		printf("\n"); */
		rClose
	}
	}
	EnvClose();
}

