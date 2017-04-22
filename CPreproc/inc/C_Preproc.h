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

#ifndef _C_Preproc_
#define _C_Preproc_ 

#include <BTTools.h>
#include <C_Lexic.h>

/*_____________________________________________________
 |
 |   String Construct Sequence:
 |
 |   1st) Create a void construct:
 |      SC = StringConstructNew();
 |   2nd) Get a pointer on the clear buffer:
 |      b = StringConstructNext(Sc,0);
 |   End of the current buffer is at 
 |      e = StringConstructGetEnd(SC);
 |   Do your Read/Write between b and e; to improve the
 | size of the buffer:
 |      b = StringConstructNext(SC,e);
 |      e = StringContsructGetEnd(SC);
 |   3rd) Signal the current end of construction:
 |      b = StringConstructNext(SC,b);
 |   You can then use the functions:
 |      s = StringConstructGetString(SC);  -> Copy the content into a String.
 |      l = StringConstructCurrentLength(SC);
 |      B = BTStringConstructNew(SC); -> Make the BuffText point on the content; no copy is performed.
 |   4th) Editing : restart the edition at a given position:
 |      p = StringConstructLast(SC,p);
 |      e = StringConstructGetEnd(SC);
 |_____________________________________________________
*/

typedef struct _StringConstruct_ StringConstruct;
StringConstruct *StringConstructNew(void);

int  StringConstructCurrentLength(StringConstruct *this);
char *StringConstructGetString(StringConstruct *this);

char *StringConstructNext(StringConstruct *this,char *e);
char *StringConstructGetEnd(StringConstruct *SC);

char *StringConstructLast(StringConstruct *this,char *e);
void StringConstructClose(StringConstruct *this);
BuffText *BTStringConstructNew(StringConstruct *S);

StringConstruct *StringConstructStringNew(char *s);

/*_____________________________________________________
 |
 |_____________________________________________________
*/

typedef enum {
   CLTBlank,
   CLTIdentifier,
   CLTSeparator,
   CLTChar,
   CLTNumber,
   CLTFloat,
   CLTString,
   CLTEndFile
} CLexemType;

typedef struct {
    CLexemType Type;
    union {
        char *Identifier,*Separator,*Number,*Float;
	    StringConstruct *String;
    } Token;
} CPreprocLexem;


/*_____________________________________________________
 |
 |_____________________________________________________
*/

#include <CtTrace.h>

typedef struct _BTConstructor_ BTConstructor;
typedef struct {
    BuffText *(*Instantiate)(BTConstructor *this);
} BTConstructorStatic;
struct _BTConstructor_ {
    BTConstructorStatic *Static;
};

BTConstructor *BTConstrCPIIFileNew(char *Name,EoxFeedbackConstr *Feedback); /* Does not keep track of line number */
BTConstructor *BTConstructorNull(void);

/*_____________________________________________________
 |
 |_____________________________________________________
*/

typedef struct _BTStack_ BTStack;
BuffText *BTStack2BuffText(BTStack *this);
BTStack *BTStackNew(void);
BTStack *BTStackInsertBT(BTStack *R,BTConstructor *B);

BTStack *BTStackInsertLocalBT(BTStack *R,BTConstructor *B);
BTStack *BTStackRemoveLocalBT(BTStack *R);

/*__________________________________________________
 |
 |__________________________________________________
*/

int CPreprocNextIdIsDefined(CPreprocEnv *Ev);
int MacroExpressionParseBool(CPreprocEnv *Ev);

CPreprocLexem *CPreprocGetNextLexem(CPreprocEnv *Ev);
BuffText *CPreprocEnvGetStream(CPreprocEnv *t);

CPreprocEnv *CPreprocEnvEnterLocalText(CPreprocEnv *Ev,BTConstructor *T);
CPreprocEnv *CPreprocEnvLeaveLocalText(CPreprocEnv *Ev);

/*__________________________________________________
 |
 |__________________________________________________
*/

CPreprocEnv *CPreprocEnvNew(char **Path);
CPreprocEnv *CPreprocEnvInsertText(CPreprocEnv *Ev,BTConstructor *T);

#endif

