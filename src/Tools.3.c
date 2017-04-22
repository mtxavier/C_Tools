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
#include <Tools.h>

/*______________________
 |
 |______________________
*/

static void ListNullDefault(TlsIdxList *this,void *Elt) {}
static void ListNullSetDefault(TlsIdxList *this,void *Elt) {}
static int ListNullEltNb(TlsIdxList *this) { return 0; }
static int ListNullEltGet(TlsIdxList *this,void *Dest,int idx,int nb) { return 0; }
static int ListNullEltSet(TlsIdxList *this,int idx,void *Elt,int nb) { return 0; }
static int ListNullInsert(TlsIdxList *this,int idx,int nb) { return 0; }
static void ListNullRemove(TlsIdxList *this,int idx,int nb) {}
static void ListNullForEach(TlsIdxList *this,int bRange,int eRange,int step,TlsIdxListIterator *Itr) {}
static TlsIdxList *ListNullClear(TlsIdxList *this) { return this; }
static struct TlsIdxList ListNullStatic = {
	ListNullDefault,ListNullSetDefault,ListNullEltNb,ListNullEltGet,ListNullEltSet,
	ListNullInsert,ListNullRemove,ListNullForEach,ListNullClear
};
TlsIdxList TlsIdxListNull = {&ListNullStatic};

/*______________________
 |
 |______________________
*/

#define cpy(dst,bs,es) { char *p,*q,*e; q=dst; p=bs; e=es; while (p<e) { *q++=*p++; } }

/*______________________
 |
 |______________________
*/

typedef struct {
	TlsIdxList TlsIdxList;
	void *bDefault,*eDefault;
	int EltSize;
	TlsIdxList *Data;
} IdxListSizedElt;

static void SizedDefault(TlsIdxList *this,void *Elt) {
	ThisToThat(IdxListSizedElt,TlsIdxList);
	cpy(Elt,that->bDefault,that->eDefault);
}
static void SizedSetDefault(TlsIdxList *this,void *Elt) {
	ThisToThat(IdxListSizedElt,TlsIdxList);
	cpy(that->bDefault,Elt,Elt+that->EltSize);
}
static int SizedEltNb(TltIdxList *this) {
	int r;
	ThisToThat(IdxListSizedElt,TlsIdxList);
	r = Call(that->Data,EltNb,0);
	return r/that->EltSize;
}
static int SizedEltGet(TlsIdxList *this,void *Dst,int idx,int nb) {
	int r;
	ThisToThat(IdxListSizedElt,TlsIdxList);
    r = Call(that->Data,EltGet,3(Dst,idx*that->EltSize,nb*that->EltSize));
	return r/that->EltSize;
}
static int SizedEltSet(TlsIdxList *this,int idx,void *Src,int nb) {
	int r;
	ThisToThat(IdxListSizedElt,TlsIdxList);
	r = Call(that->Data,EltSet,3(idx*that->EltSize,Src,nb*that->EltSize));
	return r/that->EltSize;
}
static int SizedInsert(TlsIdxList *this,int idx,int nb) {
	int r;
	ThisToThat(IdxListSizedElt,TlsIdxList);
	r = Call(that->Data,EltInsert,2(idx*that->EltSize,nb*that->EltSize));
	return r/that->EltSize;
}
static void SizedRemove(TlsIdxList *this,int idx,int nb) {
	ThisToThat(IdxListSizedElt,TlsIdxList);
	Call(that->Data,Remove,2(that->EltSize*idx,that->EltSize*nb));
}
typedef struct {
	TlsIdxListIterator TlsIdxListIterator;
	int Idx,Step;
	TlsIdxListIterator *Org;
} SizedIterator;
static int SizedIteratorFound(TlsIdxListIterator *this,int Idx,void *Elt) {
	int r;
	ThisToThat(SizedIterator,TlsIdxListIterator);
	r = Call(that->Org,Found,2(that->Idx,Elt));
	that->Idx += that->Step;
	return r;
}
static void SizedForEach(TlsIdxList *this,int bRange,int eRange,int Step,TlsIdxListIterator *Itr) {
	SizedIterator Iter;
	static struct TlsIdxListIterator Static = {&SizedIteratorFound};
	ThisToThat(IdxListSizedElt,TlsIdxList);
    Iter.TlsIdxListIterator.Static = &Static;
	Iter.Org = Itr; Iter.Idx = bRange; Iter.Step = Step;
	Call(that->Data,ForEach,4(bRange*that->EltSize,eRange*that->EltSize,Step*that->EltSize,&Iter));
}
static TlsIdxList *SizedClear(TlsIdxList *this) {
	ThisToThat(IdxListSizedElt,TlsIdxList);
	that->Data = Call(that->Data,Clear,0);
	return this;
}

IdxListSizedElt *TlsSizedEltListNew(void *bDefault,void *eDefault,TlsIdxList *Data) {
	IdxListSizedElt *r;
	static struct TlsIdxList Static = {
		SizedDefault,SizedSetDefault,SizedEltNb,SizedEltGet,SizedEltSet,
		SizedInsert,SizedRemove,SizedForEach,SizedClear
	};
	rPush(r);
	r->TlsIdxList.Static = &Static;
	r->EltSize = eDefault-bDefault;
	rnPush(r->bDefault,TlsRoundUp(int,r->EltSize));
    r->eDefault = r->bDefault+r->EltSize;
	cpy(r->bDefault,bDefault,eDefault);
	r->Data = Data;
	return r;
}

