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

/*____________________________________________________________________________________________
 |
 | Sum on n d6 between m.
 | Depending on Score, we keep the highest (score>0)
 | or the lowest (score<0) values.
 | Repart is an array containing the number of dices having obtained the value given by idx+1.
 |____________________________________________________________________________________________
*/

static int d3d6KeepNonM(int Score,int kept,int *Repart,int *Re) {
	int r,nb;
	int *p,*ep,rng,n;
	r = 0;
	if (Score>=0) {
		p = Re-1; ep = Repart-1; rng = Re-Repart;
		while (kept>0 && p>ep) {
			n = *p;
			if (n>kept) {
                r += rng*kept; kept = 0;
			} else {
			    r += rng*n; kept-=n;
			}
			p--; rng--;
		}

	} else {
		p = Repart; ep = Re; rng = 1;
		while (kept>0 && p<ep) {
			n = *p;
			if (n>kept) {
				r += rng*kept; kept = 0;
			} else {
				r += rng*n; kept-=n;
			}
			p++; rng++;
		}
	}
	return r;
}

/*_____________________________
 |
 | Parcourt
 |_____________________________
*/

typedef struct {
	double *scores;
	double weight;
	double total;
	int Kept;
	int left;
	int *bp,*ep,*cp;
	int *Repart,*Rend;
    int Score;
	int (*Val)(int Score,int Kept,int *Repart,int *Rend);
} ScoreRecord;

static void rScoreCount(ScoreRecord *record) {
	if ((record->left==0)||(record->cp+1==record->ep)) {
        *record->cp = record->left;
		record->scores[record->Val(record->Score,record->Kept,record->Repart,record->Rend)-record->Kept]+=record->weight;
		*record->cp = 0;
	} else {
		int left,l;
		double weight,combin,a,b;
		int *p;
		left = record->left;
        p = record->cp;
		weight = record->weight;
		record->cp++;
		combin = 1.;
		for (l=0;l<=left;l++) {
			*p = l;
			record->left = left-l;
			record->weight = weight * combin;
			rScoreCount(record);
			a = (left-l); b = (l+1);
			combin = (combin*a)/b;
		}
		record->weight = weight;
		record->cp = p;
		record->left = left;
	}
}

d3d6PopRepart *d3d6ScoreCount(int sides,int kept,int Score) {
	int i;
	d3d6PopRepart *r;
	ScoreRecord R;
	rPush(r);
	r->population = 0.;
	r->sides = sides;
	r->kept = kept;
	r->score = Score;
	rnPush(r->repart,((sides*kept)+1-kept));
	r->e = r->repart+((sides*kept)+1-kept);
	rOpen
	R.Kept = kept;
	R.scores = r->repart;
	rnPush(R.Repart,sides);
	R.Rend = R.Repart+sides;
	R.left = Score>=0 ? Score+R.Kept:R.Kept-Score;
	for (i=0;i<=((sides*kept)-kept);i++) R.scores[i] = 0.;
	R.total = 1.; for (i=R.left;i>0;i--) R.total = R.total*sides;
	R.weight = 1.;
	R.bp = R.Repart; R.ep = R.bp+sides; R.cp = R.bp;
	for (i=0;i<sides;i++) R.Repart[i] = 0;
	R.Score = Score;
	R.Val = d3d6KeepNonM;
	rScoreCount(&R);
	r->population = R.total;
	rClose
	return r;
}

/*____________________________________________________________
 |
 | Probabilité d'obtenir un score donné ([.0 , 1.]*0x10000)
 | Score : [-20,20]
 | target : [0,15]
 |____________________________________________________________
*/

int d3d6Check(int Score,int target) {
	static struct {
        int Inited;
	    int stat[16];	
	} precalc[41];
	static int Inited = (0!=0);
	if (!Inited) {
		Inited = (0==0);
		int i;
		for (i=0;i<41;i++) { precalc[i].Inited = (0!=0);}
	}
	Score+=20;
	if (Score<0) Score = 0;
	if (Score>40) Score = 40;
	if (!precalc[Score].Inited) {
		d3d6PopRepart *r;
		int *p;
		double Cumul,*pop,*pe;
		rOpen
		precalc[Score].Inited = (0==0);
		r = d3d6ScoreCount(6,3,Score-20);
		p = precalc[Score].stat;
        Cumul = r->population; 
		pop = r->repart; pe = r->e;
		while (pop<pe) {
			*p = ((Cumul*65536.)/r->population);
			Cumul -= *pop;
			p++; pop ++;
		}
		rClose
	}
	return precalc[Score].stat[target];
}

/*___________________________________________________
 |
 | Tirage en lui même
 |___________________________________________________
*/
/*

#include <Dices.h>

int dnd6Throw(int Score,int kept) {
	int Repart[6];
    int N;
	static Dice *d6;
	static int inited = (0!=0);
	if (!inited) { inited = (0==0); d6 = UniformDice0New(6);}
	{ int *p,*ep; for (p=Repart,ep=p+6;p<ep;p++) {*p=0;} }
	N = (Score>=0) ? Score+kept:kept-Score;
	while (N) { Repart[Call(d6,Throw,0)]++; N--;}
    return d3d6KeepNonM(Score,kept,Repart,Repart+6);
}

int d3d6Throw(int Score) {
	int Repart[6];
    int N;
	static Dice *d6;
	static int inited = (0!=0);
	if (!inited) { inited = (0==0); d6 = UniformDice0New(6);}
	{ int *p,*ep; for (p=Repart,ep=p+6;p<ep;p++) {*p=0;} }
	N = (Score>=0) ? Score+3:3-Score;
	while (N) { Repart[Call(d6,Throw,0)]++; N--;}
    return d3d6KeepNonM(Score,3,Repart,Repart+6);
}
*/
