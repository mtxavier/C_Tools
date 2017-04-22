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
#include <Schedule.h>
#include <stdio.h>

/*---------------------------------*/

typedef struct {
	SchActor SchActor;
	int ColorTiming[8];
	int Color;
	char *Label;
	int Date;
} myFlag;

static int FlagPerform(SchActor *this,int TimeLeft) {
	static char *Message[] = {
		"%s : Orange Light on\n",
		"%s : Red Light off\n",
		"%s : Green Light on\n",
		"%s : Orange Light off\n",
		"%s : Orange Light on\n",
		"%s : Green Light off\n",
		"%s : Red Light on\n",
		"%s : Orange Light off\n"
	};
	int bd,ed,cd;
	ThisToThat(myFlag,SchActor);
	bd = that->Date; ed = cd = bd + TimeLeft;
	if (ed>=that->ColorTiming[that->Color]) {
		cd = that->ColorTiming[that->Color];
		printf(Message[that->Color],that->Label);
        if (that->Color>=7) {
			that->Color = 0;
			that->Date = 0;
		} else {
			that->Color++;
			that->Date = cd;
		}
	} else {
		that->Date = ed;
	}
	return (ed-cd);
}

static SchActor *FlagNew(char *Label,int *ColorTiming,int Org) {
    static struct SchActor Static = {FlagPerform};
	myFlag *r;
	int i,dat;
    rPush(r);
	r->SchActor.Static = &Static;
	r->Label = Label;
	dat = 0;
	for (i=0;i<8;i++) {
		dat+=ColorTiming[i];
		r->ColorTiming[i] = dat;
	}
	r->Color = 0;
	r->Date = Org%ColorTiming[7];
	for (i=0;i<8;i++) {
		if (ColorTiming[i]<=r->Date) { r->Color = i; }
	}
	return (&r->SchActor);
}


/*---------------------------------*/

main() {
	int i;
	SchStage *TheStage;
	SchActor *Actors[4];
	int Timings[] = {267,18,83,18,267,18,83,18};
    EnvOpen(4096,4096);
	TheStage = SchRoundRobin("Feu");
	Actors[0] = FlagNew("Feu 1",Timings,0);
	Actors[1] = FlagNew("    Feu 2",Timings,300);
	Actors[2] = FlagNew("        Feu 3",Timings,100);
	Actors[3] = FlagNew("            Feu 4",Timings,200);
	Call(TheStage,EnterActor,1(Actors[0]));
	Call(TheStage,EnterActor,1(Actors[1]));
	Call(TheStage,EnterActor,1(Actors[2]));
	Call(TheStage,EnterActor,1(Actors[3]));
	for (i=0;i<200;i++) {
		Call(&TheStage->SchActor,Perform,1(10));
	}
	EnvClose();
}

