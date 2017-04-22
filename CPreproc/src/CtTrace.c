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
#include <List.h>
#include <stdarg.h>
#include <BTTools.h>
#include <CtTrace.h>
#include <stdio.h>

BuffText *BTIntDecimal(int x) {
	int l;
	char *s;
	l = (x<=0)? 1:0;
	{
	    int X; 
		X = (x<0)? -x:x; 
		while (X) {l++; X=X/10;} 
	}
    {
		int L; 
		L = l + sizeof(void *); 
		L = L-(L%sizeof(void *)); 
		rnPush(s,L); 
	}
    {
		int X;
		char *p;
		X = (x<0) ? -x:x;
		*s = (x<0) ? '-':'0';
		p = s+l;
		*p-- = 0;
		while (X) {
			*p-- = '0'+(X%10);
			X = X/10;
		}
	}
    return BuffTextNew(s);
}

/*-----------------------------------*/

typedef struct {
	BuffText BuffText;
	struct CatElt {
		BuffText *Text;
		int Length;
	} *Buffer,*Current,*End;
	int BeginPos,Pos,Length;
} Catenated;
static int CatenatedLength(Catenated *that) {
    int Pos,Eobt,BPos;
    Pos = that->Pos;
	BPos = that->BeginPos;
	Eobt = (0!=0);
	while (!Eobt) {
	    if (that->Current->Length==-1) {
		    int l;
			l = Call(that->Current->Text,Seek,1(0));
			that->Current->Length = Call(that->Current->Text,Seek,1(-1-l));
			Pos = BPos+that->Current->Length;
	    } else {
		    Call(that->Current->Text,Seek,1(BPos+that->Current->Length-Pos));
		    Pos = BPos+that->Current->Length;
	    }
		Eobt = (that->Current+1)==(that->End);
		if (!Eobt) {
			BPos = Pos;
			that->Current++;
		}
	}
	that->BeginPos = BPos;
	that->Pos = Pos;
    that->BuffText.p = that->Current->Text->p;
    return Pos;
}
static char *CatenatedCheck0(BuffText *this,char *p) {
	char *r;
	ThisToThat(Catenated,BuffText);
	that->Pos+= (p-this->p);
	r = Call(that->Current->Text,Check0,1(p));
	if (!r) {
		that->Current->Length = that->Pos-that->BeginPos;
		if ((that->Current+1)!=(that->End)) {
			that->Current++;
			that->BeginPos = that->Pos;
			that->Pos = 0;
			r = that->Current->Text->p;
		}
	} 
	this->p = that->Current->Text->p;
	return r;
}
/*!!! To be validated; this requires serious testing !!!!*/
static long CatenatedSeek(BuffText *this,long dp) {
	int nPos,BPos,Pos;
	ThisToThat(Catenated,BuffText);
	nPos = (that->Pos + dp);
	if (nPos<0 || nPos>that->Pos) {
		if (that->Length==-1) {
			that->Length = CatenatedLength(that);
		}
		if (that->Length<=0) { 
			that->Length = nPos = 0;
		} else {
		    if (nPos<0) {
				nPos = that->Length - ((-nPos)%that->Length);
			} else {
				nPos = nPos%that->Length;
			}
		}
	}
	BPos = that->BeginPos;
	Pos = that->Pos;
	if (nPos<=Pos) {
		while (BPos>nPos) {
			Call(that->Current->Text,Seek,1(BPos-Pos));
			Pos = BPos;
			that->Current--;
			BPos = Pos-that->Current->Length;
		}
		Call(that->Current->Text,Seek,1(nPos-Pos));
	} else {
        while (Pos<nPos) {
			if (that->Current->Length==-1) {
				int l;
				l = Call(that->Current->Text,Seek,1(0));
			    l = Call(that->Current->Text,Seek,1(-1-l));
				that->Current->Length = l;
				Pos = BPos+l;
			}
			if (BPos+that->Current->Length>=nPos) {
                Call(that->Current->Text,Seek,1(nPos-Pos));
                Pos = nPos;
			} else {
				Call(that->Current->Text,Seek,1(BPos+that->Current->Length-Pos));
				Pos = BPos+that->Current->Length;
				if (that->Current+1!=that->End) {
					that->Current++;
					BPos = Pos;
				} else {
					nPos = Pos;
				}
			}
		}
	}
    that->Pos = nPos;
	that->BeginPos = BPos;
	this->p = that->Current->Text->p;
	return (that->Pos);
}
static void CatenatedClose(BuffText *this) {
	struct CatElt *p;
	ThisToThat(Catenated,BuffText);
	p = that->Buffer;
	while (p<that->End) { Call(p->Text,Close,0); p->Length=-1; p++; }
	that->Current = that->Buffer;
	this->p = that->Current->Text->p;
}
BuffText *BTCatenate(BuffText *A,...) {
	Catenated *R;
	BuffText *r;
	int Count;
	static struct BuffText Static = {CatenatedCheck0,CatenatedSeek,CatenatedClose};
	if (A) {
		va_list v;
		BuffText *p;
	    Count = 0;
		va_start(v,A);
		p = A;
		while (p) {
			Count++;
			p = va_arg(v,BuffText *);
		}
		va_end(v);
	}
	if (A) {
		va_list v;
		BuffText *p;
	    rPush(R);
	    rnPush(R->Buffer,Count);
	    R->End = R->Current = R->Buffer;
	    R->BuffText.Static = &Static;
	    R->BeginPos = R->Pos = 0;
		R->Length = -1;
		p = A;
		va_start(v,A);
		while (p) {
			R->End->Text = p;
			R->End->Length = -1;
			R->End++;
			p = va_arg(v,BuffText *);
		}
		va_end(v);
		R->BuffText.p = R->Current->Text->p;
		r = &R->BuffText;
	} else {
		r = BuffTextNew("");
	}
	return r;
}

/*________________________________________________
 |
 |________________________________________________
*/

static BuffText *CtPosNullPrint(CtPosition *this,BuffText *Message) {return Message;}
static CtPosition *CtPositionNullMark(CtPosition *this) {return this;}
static struct CtPosition CtPositionNullStatic = {CtPosNullPrint,CtPositionNullMark};
CtPosition CtPositionNull = {&CtPositionNullStatic};

/*------*/

typedef struct {
	CtPosition CtPosition;
	CtPosition *Child;
    int Line,PosLine,Pos;
} PosAtLine;
static BuffText *PosAtLinePrint(CtPosition *this,BuffText *Message) {
	BTList *R;
	BuffText *C;
	ThisToThat(PosAtLine,CtPosition);
	C = Call(that->Child,Print,1(Message));
	R = BTListNew(BuffTextNew("At line "),BTIntDecimal(that->Line),
		BuffTextNew(", pos "),BTIntDecimal(that->PosLine),BuffTextNew(":"),C,0);
	return BTListGetBT(R);
}
static CtPosition *PosAtLineMark(CtPosition *this) {
	CtPosition *Child;
	ThisToThat(PosAtLine,CtPosition);
	Child = Call(that->Child,Mark,0);
	return CtPosAtLine(that->Line,that->PosLine,that->Pos,Child);
}
CtPosition *CtPosAtLine(int Line,int LinePos,int FilePos,CtPosition *Child) {
	PosAtLine *R;
	static struct CtPosition Static = {PosAtLinePrint,PosAtLineMark};
	rPush(R);
	R->CtPosition.Static = &Static;
	R->Line = Line;
	R->PosLine = LinePos;
	R->Pos = FilePos;
	R->Child = Child;
	return &R->CtPosition;
}

/*------*/

typedef struct {
	CtPosition CtPosition;
	CtPosition *Child;
	char *FileName;
} PosInFile;
static BuffText *PosInFilePrint(CtPosition *this,BuffText *Message) {
	BTList *R;
	BuffText *C;
	ThisToThat(PosInFile,CtPosition);
	C = Call(that->Child,Print,1(Message));
	R = BTListNew(BuffTextNew("In file \""),BuffTextNew(that->FileName),BuffTextNew("\":"),C,0);
	return BTListGetBT(R);
}
static CtPosition *PosInFileMark(CtPosition *this) {
	CtPosition *Child;
	char *FileName;
	ThisToThat(PosInFile,CtPosition);
	Child = Call(that->Child,Mark,0);
	FileName = StringDuplicate(that->FileName);
	return CtPosInFile(FileName,Child);
}
CtPosition *CtPosInFile(char *Name,CtPosition *Child){
	PosInFile *R;
	static struct CtPosition Static = {PosInFilePrint,PosInFileMark};
	rPush(R);
	R->CtPosition.Static = &Static;
    R->FileName = Name;
	R->Child = Child;
	return &R->CtPosition;
}

/*------*/

typedef struct {
	CtPosition CtPosition;
	char *Message;
} PosMessage;
static BuffText *PosMessagePrint(CtPosition *this,BuffText *Message) {
	ThisToThat(PosMessage,CtPosition);
	return BTCatenate(BuffTextNew(that->Message),Message,0);
}
static CtPosition *PosMessageMark(CtPosition *this) {
	char *Message;
	ThisToThat(PosMessage,CtPosition);
	Message = StringDuplicate(that->Message);
	return CtPosMessage(Message);
}
CtPosition *CtPosMessage(char *Message) {
	PosMessage *R;
	static struct CtPosition Static = {PosMessagePrint,PosMessageMark};
	rPush(R);
	R->CtPosition.Static = &Static;
	R->Message = Message;
	return &R->CtPosition;
}

/*--------------------------------*/

typedef struct {
	CtPosition CtPosition;
	CtPosition *Children[2];
} StackedPosition;
static BuffText *StackedPositionPrint(CtPosition *this,BuffText *Msg) {
	BuffText *R;
	ThisToThat(StackedPosition,CtPosition);
	R = Call(that->Children[1],Print,1(Msg));
    R = Call(that->Children[0],Print,1(R));
	return R;
}
static CtPosition *StackedPositionMark(CtPosition *this) {
	CtPosition *Children[2];
	ThisToThat(StackedPosition,CtPosition);
	Children[0] = Call(that->Children[0],Mark,0);
	Children[1] = Call(that->Children[1],Mark,0);
	return StackedPositionNew(Children[0],Children[1]);
}
CtPosition *StackedPositionNew(CtPosition *a,CtPosition *b) {
	StackedPosition *R;
	static struct CtPosition Static = {StackedPositionPrint,StackedPositionMark};
	rPush(R);
	R->CtPosition.Static = &Static;
	R->Children[0] = a;
	R->Children[1] = b;
	return &R->CtPosition;
}

/*________________________________________________
 |
 | Position recording
 |________________________________________________
*/

 /* Pop is performed asynchronously because the current position is actually the position of the next lexem ! */
struct FilePosStack {
	CtPosition CtPosition;
	MemStack *Mem;
	List Stack;
	List *Current;
	char *Filename;
    int Pos,LineNum,LineBeg;
};

typedef struct {
	EoxFeedback EoxFeedback;
	CtPosition CtPosition;
	List List;
	char *Filename;
	int Pos,LineNum,LineBeg;
	FilePosStack *Stack;
	int Poped;
} FilePosStacked;

void BuffTextPrint(BuffText *B) {
	char *p;
	p = B->p;
	do {
		printf(p);
		while (*p) p++;
		p = Call(B,Check0,1(p));
	} while (p);
}

static BuffText *PositionStackedPrint(char *Filename,int Line,int LineBeg,int Pos,BuffText *Message) {
	BuffText *R;
	R = BTCatenate(
		BuffTextNew("\""),BuffTextNew(Filename),BuffTextNew("\"("),BTIntDecimal(Line+1),
		BuffTextNew("):"),Message,0
		/* BuffTextNew(", Pos "),BTIntDecimal(Pos-LineBeg),BuffTextNew(": "),Message,0 */
	);
	return R;
}
static CtPosition *PositionStackedMark(char *Filename,int Line,int LineBeg,int Pos) {
	return CtPosAtLine(Line,Pos-LineBeg,Pos,CtPosInFile(Filename,&CtPositionNull));
}
static BuffText *FilePosStackedPrint(CtPosition *this,BuffText *Message) {
	BuffText *R;
	List *n;
	ThisToThat(FilePosStacked,CtPosition);
	if (that->Poped) {
	    R = Message;
	} else {
		R = PositionStackedPrint(that->Filename,that->LineNum,that->LineBeg,that->Pos,Message);
	}
	n = that->List.n;
	if (n!=ListEnd) {
		FilePosStacked *N;
		N = CastBack(FilePosStacked,List,n);
		R = Call(&N->CtPosition,Print,1(R));
	}
	return R;
}
static CtPosition *FilePosStackedMark(CtPosition *this) {
	CtPosition *R;
	List *n;
	ThisToThat(FilePosStacked,CtPosition);
	if (that->Poped) {
		R = &CtPositionNull;
	} else {
	    R = PositionStackedMark(that->Filename,that->LineNum,that->LineBeg,that->Pos);
	}
	n = that->List.n;
	if (n!=ListEnd) {
		FilePosStacked *N;
		CtPosition *C;
		N = CastBack(FilePosStacked,List,n);
		C = Call(&N->CtPosition,Mark,0);
	    if (C!=&CtPositionNull) {
			R = StackedPositionNew(C,R);
	    }
	}
	return R;
}
static void FilePosStackEndOfFile(EoxFeedback *this) {
	ThisToThat(FilePosStacked,EoxFeedback);
	that->Poped = (0==0);
}
static void FilePosStackEndOfLine(EoxFeedback *this,int LineNumber,int LinePos) {
	ThisToThat(FilePosStacked,EoxFeedback);
	that->LineNum = LineNumber;
	that->LineBeg = LinePos;
	that->Pos = LinePos;
}
static void FilePosStackPosInFile(EoxFeedback *this,int NewPos) {
	ThisToThat(FilePosStacked,EoxFeedback);
	that->Pos = NewPos;
}
FilePosStacked *PositionStackFileNew(FilePosStack *Stack,char *Filename) {
	FilePosStacked *R;
	static struct {
		struct EoxFeedback EoxFeedback;
		struct CtPosition CtPosition;
	} Static = {
		 {FilePosStackEndOfFile,FilePosStackEndOfLine,FilePosStackPosInFile}
		,{FilePosStackedPrint,FilePosStackedMark}
	};
	rPush(R);
	R->EoxFeedback.Static = &Static.EoxFeedback;
	R->CtPosition.Static = &Static.CtPosition;
	R->Filename = Filename;
	R->Stack = Stack;
	R->LineNum = 0;
	R->LineBeg = R->Pos = 0;
	R->Poped = (0!=0);
	R->List.n = ListEnd;
	return R;
}

EoxFeedback *FilePosStackPush(FilePosStack *Stack,char *Filename) {
	FilePosStacked *R;
	FilePosStackNextLexem(Stack);
	mEnter(Stack->Mem);
    mIn(Stack->Mem,Filename = StringDuplicate(Filename));
	mIn(Stack->Mem,R = PositionStackFileNew(Stack,Filename));
	R->List.n = Stack->Stack.n;
	Stack->Stack.n = &R->List;
	return &R->EoxFeedback;
}
void FilePosStackNextLexem(FilePosStack *Stack) {
	List *n;
	n = Stack->Stack.n;
	if (n!=ListEnd) {
		int end;
	    FilePosStacked *N;
		do {
		    N = CastBack(FilePosStacked,List,n);
			n = n->n;
			end = (!N->Poped)||(n==ListEnd);
			Stack->Filename = N->Filename;
			Stack->Pos = N->Pos;
			Stack->LineBeg = N->LineBeg;
			Stack->LineNum = N->LineNum;
			Stack->Current = n;
			if (N->Poped) {
				BuffText *Msg;
				Stack->Stack.n = n;
				Stack->Filename = "";
				mLeave(Stack->Mem);
			}
		} while (!end);
	}
}

static BuffText *FilePosStackPrint(CtPosition *this,BuffText *Message) {
	BuffText *R;
	ThisToThat(FilePosStack,CtPosition);
	R = PositionStackedPrint(that->Filename,that->LineNum,that->LineBeg,that->Pos,Message);
	if (that->Current != ListEnd) {
		FilePosStacked *F;
		F = CastBack(FilePosStacked,List,that->Current);
		R = Call(&F->CtPosition,Print,1(R));
	}
	return R;
}
static CtPosition *FilePosStackMark(CtPosition *this) {
	CtPosition *R;
	ThisToThat(FilePosStack,CtPosition);
	R = PositionStackedMark(that->Filename,that->LineNum,that->LineBeg,that->Pos);
	if (that->Current != ListEnd) {
		FilePosStacked *F;
		CtPosition *E;
		F = CastBack(FilePosStacked,List,that->Current);
		E = Call(&F->CtPosition,Mark,0);
		if (E!=&CtPositionNull) R = StackedPositionNew(E,R);
	}
	return R;
}

FilePosStack *FilePosStackNew(void) {
	FilePosStack *R;
	MemStack *Mem;
	static struct CtPosition Static = {FilePosStackPrint,FilePosStackMark};
	Mem = rFork(512);
    mPush(Mem,R);
	R->Mem = Mem;
	R->CtPosition.Static = &Static;
	R->Stack.n = ListEnd;
	R->Current = R->Stack.n;
	R->Filename = "";
	R->Pos = R->LineNum = R->LineBeg = 0;
	return R;
}

CtPosition *FilePosStackGetPosition(FilePosStack *this) {
	return &this->CtPosition;
}


