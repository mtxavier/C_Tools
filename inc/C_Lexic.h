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

#ifndef _C_Syntax_h_
#define _C_Syntax_h_

typedef enum {
    CS_Eof = 0,
    CS_Switch,
    CS_Case,
    CS_Default,
    CS_Break,
    CS_If,
    CS_Else,
    CS_For,
    CS_Do,
    CS_While,
    CS_Continue,
    CS_Return,
    CS_Goto,
    CS_Etc,
	CS_Defined,

    CS_Inf = '<',
    CS_Sup = '>',
    CS_Set = '=',
    CS_Add = '+',
    CS_Sub = '-',
    CS_Mul = '*',
    CS_Div = '/',
    CS_Mod = '%',
    CS_And = '&',
    CS_Or = '|',
    CS_Not = '!',
    CS_Xor = '^',
    CS_Comp = '~',
    CS_Eoi = ';',
    CS_OpenBrace = '[',
    CS_CloseBrace = ']',
    CS_OpenParenthesis = '(',
    CS_CloseParenthesis = ')',
    CS_OpenUnit = '{',
    CS_CloseCloseUnit = '}',
    CS_Coma = ',',
    CS_Select = '.',
    CS_Altrnt = '?',
    CS_Label = ':',

    CS_Idrct = 'A',
    CS_Diff,
    CS_Inc,
    CS_Dec,
    CS_Eq,
    CS_InfEq,
    CS_SupEq,
    CS_bAnd,
    CS_bOr,
    CS_SAL,
    CS_SAR,
    CS_AddSet,
    CS_AndSet,
    CS_DivSet,
    CS_ModSet,
    CS_MulSet,
    CS_NotSet,
    CS_OrSet,
    CS_SALSet,
    CS_SARSet,
    CS_SubSet,
    CS_XorSet,

    CS_Sizeof = '0',
    CS_ConstChar,
    CS_ConstInt,
    CS_ConstFloat,
    CS_ConstString,
    CS_ConstIdentifier,

    CS_Typedef = 'a',

    CS_Extern,
    CS_Static,

    CS_Auto,
    CS_Volatile,
    CS_Register,
    CS_Const,

    CS_Unsigned,
    CS_Signed,
    CS_Long,
    CS_Short,

    CS_Void,

    CS_Char,
    CS_Int,
    CS_Double,
    CS_Float,
    CS_TypeName,

    CS_Struct,
    CS_Union,
    CS_Enum,

} CSynLexemType;

typedef struct { 
	struct CSynLexem *Static;
	CSynLexemType Type;
} CSynLexem;
struct CSynLexem {
   char *(*Instance)(CSynLexem *this);
   CSynLexem *(*Duplicate)(CSynLexem *this);
} CSynLexemStatic;

/*__________________________________________________
 |
 |__________________________________________________
*/

#include <BuffText.h>
typedef struct _CPreprocEnv_ CPreprocEnv;
CPreprocEnv *CPreprocEnvOpen(char **Path,char *File);
CPreprocEnv *CPreprocEnvStringOpen(char **Path,char *Data);
CSynLexem *CSynPreprocGetLexem(CPreprocEnv *Ev);
BuffText *CPreprocIssueError(CPreprocEnv *Ev,BuffText *Msg);

#endif

