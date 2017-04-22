#ifndef _Key_h_
#define _Key_h_


/*___________________________________________________________
 |                                                           |
 | Key definition:                                           |
 | a key is a character sequence of the following syntax     |
 |     0..7F                                                 |
 |     80..FF Key Key                                        |
 | In the second syntax, the first Key is the Head and the   |
 | second the Tail.  As a convention we will say that for    |
 | an atomic Key A, A = Head(A) = Queue(A).                  |
 | (Trivial) useful properties:                              |
 |    Length of a Key is always even.                        |
 |    A key contains always one char below 80 more than      |
 |       it contains char up 80.
 |___________________________________________________________|
*/

/*___________________________________________________________
 |                                                           |
 | KeyCmp returns the first divergent char position so to end|
 | the comparison, you should compare those char.            |
 | KeyCpy returns the char position following the copy. You  |
 | can use this to compose Keys.                             |
 |___________________________________________________________|
*/


char *K256Tail(char *Key);
char *K256Head(char *Key);
int  K256Cmp(const char *ref,const char *Key);
char *K256Cpy(char *dst,const char *Key);
int  K256Length(const char *Key);

/*____________________________________________________________
 |
 | Keys and C types
 | Quotation rules:
 |     Keys 0..7f are quoted with '-' followed with 2 digits hexa.
 |     Keys 80+Key Key1 Key2 are quoted |KeyKey1Key2
 | Examples:
 |      -07
 |      -6e
 |      |07-2b|64-7c-3b
 |____________________________________________________________
*/

#define KeyQuoteAtom ')'
#define KeyQuoteChain '('

char *K2String(char *S,char *Key);
char *String2K(char *Key,char *S);
char *QuoteKey(char *S,char *Key);
char *UnquoteKey(char *Key,char *S);
char *Int2K(char *K,int i);
int  K2Int(char *K);
char *P2K(char *K,void *p);
void *K2P(char *K);

#include <Patricia.h>

void K256NodeInit(PatriciaNod *Root);
PatriciaNod *K256NodeSeek(PatriciaNod *Root,char *s);
PatriciaNod *K256NodeSeekOrInsert(PatriciaNod *Root,char *s,PatriciaNod *NewNod);
#define K256NodeForEach PatriciaForEach

#endif
