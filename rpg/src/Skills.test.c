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

#include <stdio.h>
#include <Classes.h>
#include <StackEnv.h>
#include <RpgCore.h>

main() {
    RpgSkillPool *dp;
    int i,j,k;
    EnvOpen(4096,4096);
    dp = RpgSkillPoolNew();
    for (i=5;i<19;i++) {
        int repart[24];
        printf("\nLvl %2d:",i);
    Call(dp,Init,2(0,i));
    for (k=0;k<24;k++) {repart[k]=0;}
        for (k=0;k<1000;k++) { repart[RpgSkillPoolThrow(dp)]++; }
        j = 1000;
        for (k=3;k<19;k++) { j-=repart[k]; printf("%4d",j); }
    }
    printf("\n");
    EnvClose();
}


