/*  BuffText offers an interface to read text file in a fast way.
 *  It also offers file abstraction so other object might be substituted for files
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
#include <BuffText.h>
#include <stdio.h>
#include <string.h>

/*_____________________________________________________
 |
 |  BuffText
 |_____________________________________________________
*/

typedef struct {
    BuffText BuffText;
	char *begin,*end;
} BTString;

static char *BuffTextCheck0(BuffText *this,char *p) {
    this->p = p;
    return (*p)?p:NULL;
}
static long BuffTextSeek(BuffText *this,long dp) {
    int Result;
	int l;
    ThisToThat(BTString,BuffText);
	if (that->end<that->begin) {
	    that->end=that->begin+strlen(that->begin);
	}
    l = 1+that->end-that->begin;
	Result = dp+(this->p-that->begin);
	if (Result<0) {
	    Result = l-((-Result)%(l));
		if (Result==l) Result=0;
	} else {
	    Result = Result%l;
	}
	this->p=that->begin+Result;
    return Result;
}
static void BuffTextClose(BuffText *this) {
	ThisToThat(BTString,BuffText);
	this->p = that->begin;
}

BuffText *BuffTextNew(char *s) {
    static struct BuffText Static = {
        BuffTextCheck0, BuffTextSeek, BuffTextClose
    };
    BTString *New;
	rPush(New);
	New->begin = s;
	New->end = s-1;
	New->BuffText.p = s;
	New->BuffText.Static = &Static;
	return &New->BuffText;
}

/*_____________________________________________________
 |
 |  BTFile
 |_____________________________________________________
*/

typedef struct {
    BuffText BuffText;
	char *bob,*eob,*eof;
	char *label;
	FILE *f;
} BTFile;

static char *BTFileCheck0(BuffText *this,char *p) {
    ThisToThat(BTFile,BuffText);
	this->p = p;
	if (p==that->eob) {
		if (!that->f) { 
			that->f = fopen(that->label,"r");
			if (that->f) {
	            that->eof = that->bob+fread(that->bob,1,that->eob-that->bob,that->f);
			} else {
				that->eof = that->bob;
			}
		} else {
	        that->eof = that->bob+fread(that->bob,1,that->eob-that->bob,that->f);
		}
        *(that->eof) = 0;
		this->p = that->bob;
	}
	return (this->p==that->eof) ? NULL:this->p;
}
static long BTFileSeek(BuffText *this,long dp) {
    int Pos,EndPos,bPos,ePos;
    ThisToThat(BTFile,BuffText);
	if (!that->f) {
		this->p = that->eof = that->eob;
		that->f = fopen(that->label,"r");
	}
	if (that->f) {
	ePos=ftell(that->f);
	bPos=(that->bob-that->eof)+ePos;
    Pos=(this->p-that->eof)+ePos+dp;
	fseek(that->f,SEEK_END,0);
	EndPos=ftell(that->f)+1;
    if (Pos<0) {
	    Pos = EndPos-((-Pos)%EndPos);
		if (Pos==EndPos) Pos=0;
	} else {
	    Pos = Pos%EndPos;
	}
	if (Pos>=bPos && Pos<ePos) {
	    fseek(that->f,ePos,SEEK_SET);
		this->p = that->bob+(Pos-bPos);
	} else {
	    fseek(that->f,Pos,SEEK_SET);
	    that->eof = that->bob+fread(that->bob,1,that->eob-that->bob,that->f);
        *(that->eof) = 0;
	    this->p = that->bob;
    }
	} else {
		this->p = that->eof = that->eob;
		*this->p = 0;
	}
	return Pos;
}

static void BTFileClose(BuffText *this) {
    ThisToThat(BTFile,BuffText);
	if (that->f) {
        fclose(that->f);
		that->f = 0;
	}
	this->p = that->eof = that->eob;
}

BuffText *BTFileNew(char *Name,long BuffSize){
    static struct BuffText Static = {
        BTFileCheck0, BTFileSeek, BTFileClose
    };
    BTFile *New;
	int sName;
	rPush(New);
	New->BuffText.Static = &Static;
	sName = strlen(Name)+1;
	New->f = 0;
	rnPush(New->label,((sName+BuffSize+1+sizeof(void *))&(-sizeof(void *))));
	strcpy(New->label,Name);
	New->bob = New->label+sName;
	New->eob = New->bob+BuffSize;
	*New->eob = 0;
	New->BuffText.p = New->eof = New->eob;
	return &New->BuffText;
}
