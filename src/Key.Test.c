#include <stdio.h>
#include "Key.h"
#include "Patricia.h"

char Const0[]="Un  essai.";

PatriciaNod *KeyBrowse(PatriciaNod *f,void *Clos) {
    char Chaine[64];
    printf("\n(");
    QuoteKey(Chaine,f->Key);
    printf(Chaine);
    return 0;
}

main() {
    char Chaine0[100];
    char Chaine1[100];
    char Chaine2[100];
    char *P,*cnst;
    int a,b;
    cnst = Const0;
    P2K(Chaine0,cnst);
    QuoteKey(Chaine1,Chaine0);
    P=K2P(Chaine0);
    printf("\n%x %x\n(%s\n",cnst,P,Chaine1);
    a=-543234;
    Int2K(Chaine0,a);
    b=K2Int(Chaine0);
    String2K(Chaine2,Const0);
    K2String(Chaine0,Chaine2);
    printf("%d %d\n %s\n",a,b,Chaine0);
    {
        PatriciaNod Root;
        PatriciaNod Leaf[15];
        int i;
        K256NodeInit(&Root);
        i=0;
        K256NodeSeekOrInsert(&Root,"\x1",Leaf+0);
        K256NodeSeekOrInsert(&Root,"\x2",Leaf+1);
        K256NodeSeekOrInsert(&Root,"\x3",Leaf+2);
        K256NodeSeekOrInsert(&Root,"\x4",Leaf+3);
        PatriciaForEach(&Root,KeyBrowse,0);
        printf("\n");
    }
}

