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

#ifndef _CtTrace_
#define _CtTrace_

typedef struct {
	struct CtPosition *Static;
} CtPosition;
struct CtPosition {
	BuffText *(*Print)(CtPosition *this,BuffText *Message);
	CtPosition *(*Mark)(CtPosition *this);
};

extern CtPosition CtPositionNull;
CtPosition *CtPosInFile(char *Filename,CtPosition *Child);
CtPosition *CtPosAtLine(int Line,int LinePos,int FilePos,CtPosition *Child);
CtPosition *CtPosMessage(char *Message);
CtPosition *StackedPositionNew(CtPosition *a,CtPosition *b);


/*_________________________________________________
 |
 | EoxFeedback
 |_________________________________________________
*/

typedef struct {
	struct EoxFeedback *Static;
} EoxFeedback;
struct EoxFeedback {
	void (*EndOfFile)(EoxFeedback *this);
	/* 
	     Those only give a rough indication of the cursor position;
         directly after a Check0 call, this indication should 
		 however be pretty accurate.
		 Linecount starts at 0.
	 */
	void (*EndOfLine)(EoxFeedback *this,int NewLineNumber,int Pos);
	void (*PosInFile)(EoxFeedback *this,int NewPos); 
};
typedef struct {
	struct EoxFeedbackConstr *Static;
} EoxFeedbackConstr;
struct EoxFeedbackConstr {
	EoxFeedback *(*Instantiate)(EoxFeedbackConstr *this);
};

extern EoxFeedback EoxFeedbackNull;
extern EoxFeedbackConstr EoxFeedbackConstrNull;
EoxFeedbackConstr *EoxFeedbackQuote(EoxFeedback *Const);

/*________________________________________________
 |
 | Position recording
 |________________________________________________
*/

typedef struct FilePosStack FilePosStack;
FilePosStack *FilePosStackNew(void);
CtPosition *FilePosStackGetPosition(FilePosStack *Stack);
EoxFeedback *FilePosStackPush(FilePosStack *Stack,char *Filename);
void FilePosStackNextLexem(FilePosStack *Stack);

#endif

