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

#ifndef _d3d6_h_
#define _d3d6_h_

/*______________________________________________________
 |
 |  Sum of 3 6 sided dices among 3+|Score|.
 |  If score is positive, keep the highest, 
 |  keep the lowest if it is negative.
 |  The final score is a number in the range [0..15]
 |______________________________________________________
*/

/*_______________________________________________________________
 |
 | Give statistical data (to evaluate an action expected result).
 |
 | d3d6ScoreCount: for a given score, gives the number of 
 |     possible draws (-> in pop), and the population of the various
 |     scores (-> in repart[16]).
 |     This is very slow computation and shouldn't be used for big
 | values of |score| or as part of regular real time computation (use tables instead).
 |
 | d3d6Check: for a given score and a value [0..15], gives the
 |     0x10000 * probability of obtaining that value or above.
 | d3d6Check keeps a table for scores in the range [-20..20]. 
 | For now, the score value is clipped to these values.
 |
 |_______________________________________________________________
*/

typedef struct {
	int sides,kept,score;
	double population;
	double *repart,*e;
} d3d6PopRepart;

d3d6PopRepart *d3d6ScoreCount(int sides,int kept,int Score);
int d3d6Check(int Score,int target);

/*______________________________________________________
 |
 | make a measure (throw the dices).
 | 
 |______________________________________________________
*/

int d3d6Throw(int Score);
int dnd6Throw(int Score,int kept);

#endif
