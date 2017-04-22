#include "BuffText.h"
#include "Classes.h"
#include "List.h"
#include "string.h"
#include "stdlib.h"

typedef struct _BTEditStatic_ BTEditStatic;
typedef struct _BTEdit_ BTEdit;

struct _BTEditStatic_ {
    long (*Insert)(BTEdit *this,long Offset,long size,void *data);
    long (*Remove)(BTEdit *this,long Offset,long size);
    BuffText *(*Open)(BTEdit *this);
};

struct _BTEdit_ {
    BTEditStatic *Static;
};

typedef struct _BTEditNode_ BTEditNode;
struct _BTEditNode_ {
    BTEditNode *Children[2];
    long Size,LeftSize;
};

typedef struct {
    BTEdit BTEdit;
    BTEditNode Root;
} BTEditTree;

#define BTEditNodeGranularity 512
#define BTEditNodeSize (BTEditNodeGranularity-sizeof(BTEditNode))

typedef struct {
    BTEditNode Node;
    char Data[BTEditNodeSize];
} BTEMemoryNode;

BTEMemoryNode *BTEMemoryNodeAllocate(void) {
    BTEMemoryNode *Result;
    Result = malloc(sizeof(BTEMemoryNode));
    Result->Node.Children[0] = Result->Node.Children[1] = &Result->Node;
    return Result;
}

typedef struct {
    BTEditNode *Node;
    long Offset;
} NodePosition;

NodePosition *GetOffsetNode(NodePosition *Result,BTNode *Root,long Offset){
    BTEditNode *r;
    long o;
    int end;
    Result->Node = Root;
    Result->Offset = Offset-BTEditNodeSize;
    r1 = Root;
    r = r1->Children[1];
    end = (r==r1);
    o = 0;
    while ((!end)&&(r!=r1)){
        BTEditNode *r1;
        o+=r->LeftSize;
        end =((o<Offset)&&(o+r->Size>Offset))||(o==Offset);
        if (end) {
            Result->Node = r;
            Result->Offset = o;
        } else {
            r1=r;
            if (o>Offset) {
                o-=r->LeftSize;
                r=r1->Children[0];
            } else {
                o+=r->Size;
                r=r1->Children[1];
            }
            end = (r1==r);
            if (end && o==Offset) {
                /* End of file  */
                Result->Node = r;
                Result->Offset = o-r->Size;
            }
        }
    }
    return Result;
}

void InsertNodeInTree(BTEditNode *Root,BTEditNode *n,long Offset) {
    BTEditNode *p,*q;
    p = Root;
    q = p->Children[1];
    n->LeftSize = 0;
    n->Children[0] = n->Children[1] = n;
    if (p==q) {
        Root->Children[1] = n;
    } else {
        long o;
        int end;
        o = Offset;
        do {
            end = ((q->LeftSize<o)&&(q->LeftSize+q->Size>o))||(q->LeftSize==o);
            if (end) {
                n->LeftSize=q->LeftSize;
                q->LeftSize+=n->Size;
                if (q->Children[0]!=q) {
                    n->Children[0] = q->Children[0];
                }
                q->Children[0] = n;
            } else {
                p=q;
                if (q->LeftSize>o) {
                    q=p->Children[0];
                    p->LeftSize+=n->Size;
                } else {
                    q=p->Children[1];
                    o-=p->LeftSize+p->Size;
                }
            }
        } while ((p!=q)&&(!end));
    }
}

BTEditNode *ExtractNodeFromTree(BTEditNode *Root,long Offset) {
    BTEditNode *p,*q,*r;
    long o;
    int end;
    p = Root;
    r = q = p->Children[1];
    o = Offset;
    do {
        end = ((r->LeftSize<o)&&(r->LeftSize+r->Size>o))||(r->LeftSize==o);
        if (!end) {
            p = r;
            if (r->LeftSize>o) {
                r=r->Children[0];
            } else {
                o-=r->LeftSize+r->Size;
                r=r->Children[1];
            }
        }
    } while (!end && p!=r);
    if (end) {
        int i;
        o = Offset; 
        while (q!=r) {
            if(q->LeftSize>o) {
                q->LeftSize-=r->Size;
                q=q->Children[0];
            } else {
                o-=q->LeftSize+q->Size;
                q=q->Children[1];
            }
        }
        i = p->Children[0]==r ? 0:1;
        if (r->Children[0]==r) {
            if (r->Children[1]==r) {
                p->Children[i] = p;
            } else {
                p->Children[i] = r->Children[1];
                r->Children[1]=r;
            }
        } else {
            SynNode *q0;
            q0=r->Children[0];
            q=q0->Children[1];
            if (q!=q0) {
                while (q->Children[1]!=q) {
                   q0=q;
                   q=q0->Children[1];
                }
                q0->Children[1] = q0;
                q->LeftSize = r->LeftSize;
                q->Children[0] = r->Children[0];
            }
            if (r->Children[1]==r) 
                q->Children[1]=q;
            else
                q->Children[1] = r->Children[1];
            r->Children[0] = r->Children[1] = r;
            p->Children[i] = q;
        }
    } else {
        r = Root;
    }
    return r;
}

long BTEditTreeInsert(BTEdit *this,long Offset,long size,void *data) { 
    NodePosition Pos;
    BTEMemoryNode *n;
    ThisToThat(BTEditTree,BTEdit);
    GetOffsetNode(&Pos,&that->Root,Offset);
    n = CastBack(BTEMemoryNode,Node,Pos.Node);
    Pos.Offset=Offset-Pos.Offset;
    if (size+r->Size<=BTEditNodeSize) {
        BTEditNode *x;
        x = ExtractNodeFromTree(&that->Root,Offset-Pos.Offset);
        memcpy(n->Data+Pos.Offset+size,n->Data+Pos.Offset,
            Pos.Node->Size-Pos.Offset);
        memcpy(n->Data+Pos.Offset,data,size);
        x->Size+=size;
        InsertNodeInTree(&that->Root,x,Offset-Pos.Offset);
    } else {
        long s,o;
        BTEMemoryNode *Last;
        Last = BTEMemoryNodeAllocate();
        s = size;
        if (Pos.Node != &that->Root) {
            o = (Pos.Offset+s)%BTEditNodeSize;
            memcpy(Last->Data+o,n->Data+Pos.Offset,Pos.Node->Size-Pos.Offset);
        }
        memcpy(Last->Data,data+s-o,o);
        Last->Node.Size = o + Pos.Node->Size-Pos.Offset;
        s-=o;
        while (s>BTEditNodeSize-r->Size) {
            Last->Node.Children[0] = BTEMemoryNodeAllocate();
            Last->Node.Children[0]->Children[1] = &Last->Node;
            Last = CastBack(BTEMemoryNode,Node,Last->Node.Children[0]);
            s-=BTEditNodeSize;
            memcpy(Last->Data,data+s,BTEditNodeSize);
            Last->Node.Size = BTEditNodeSize;
        }
        /* Compute size */
        if (Pos.Node != &that->Root) {
            BTEditNode *x;
            x = ExtractNodeFromTree(&that->Root,Offset-Pos.Offset);
            memcpy(n->Data+Pos.Offset,data,BTEditNodeSize-Pos.Offset);
            x->Size=BTEditNodeSize;
            InsertNodeInTree(&that->Root,x,Offset-Pos.Offset);
        }
        /* Insert in Tree */ {
            BTEditNode *N,*P;
            long o;
            o = (Offset-Pos.Offset)+BTEditNodeSize;
            P = &Last->Node;
            N = P->Children[1];
            while (P!=N) {
                N->Children[0] = N;
                P->Children[1] = P;
                InsertNodeInTree(&that->Root,P,o);
                o+=BTEditNodeSize;
                P = N;
                N = P->Children[1];
            }
            InsertNodeInTree(&that->Root,P,o);
        }
    }
    return size;
}

