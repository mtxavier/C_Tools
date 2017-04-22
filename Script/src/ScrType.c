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
#include <ScrType.h>

/*_____________________________________
 |
 | Browser
 |_____________________________________
*/

static void StructBrowserNullAdd(ScrTypeStructBrowser *this,int num,char *Label,ScrType *Type) {}
static ScrType *StructBrowserNullEnd(ScrTypeStructBrowser *this) { return &ScrTypeNull;}
static void StructBrowserNullMap(ScrTypeStructBrowser *this,int offset,int num,char *Label,ScrType *Type) {}
static ScrType *StructBrowserNullMapEnd(ScrTypeStructBrowser *this,char *Label,int align,int Size) { return &ScrTypeNull; }
static struct ScrTypeStructBrowser StructBrowserNullStatic = { 
	StructBrowserNullAdd,StructBrowserNullEnd,StructBrowserNullMap,StructBrowserNullMapEnd
};
ScrTypeStructBrowser ScrTypeStructBrowserNull = {&StructBrowserNullStatic};

/*-------*/

static ScrTypeThrou *ScrTypeThrouNew(void);
static ScrType *BrowserNullAtom(ScrTypeBrowser *this,ScrTypeBase Atom) { return &ScrTypeNull; }
static ScrTypeStructBrowser *BrowserNullStruct(ScrTypeBrowser *this,char *Label,ScrType *Base,int FieldNb) {
	return &ScrTypeStructBrowserNull;
}
static ScrType *BrowserNullFn(ScrTypeBrowser *this,ScrType *Result,ScrType *Params) { return &ScrTypeNull; }
static ScrType *BrowserNullPtr(ScrTypeBrowser *this,ScrType *On) { return &ScrTypeNull; }
static ScrType *BrowserNullArray(ScrTypeBrowser *this,int EltNb,ScrType *Of) { return &ScrTypeNull; }
static ScrType *BrowserNullList(ScrTypeBrowser *this,ScrType *Of) { return &ScrTypeNull; }
static ScrType *BrowserNullLabeled(ScrTypeBrowser *this,char *Label,ScrType *Actual) { return &ScrTypeNull; }
static ScrTypeThrou *BrowserNullThrou(ScrTypeBrowser *this) { return ScrTypeThrouNew(); }
static struct ScrTypeBrowser BrowserNullStatic = {
	BrowserNullAtom,BrowserNullStruct,BrowserNullFn,
	BrowserNullPtr,BrowserNullArray,BrowserNullList,BrowserNullLabeled,
    BrowserNullThrou
};
ScrTypeBrowser ScrTypeBrowserNull = {&BrowserNullStatic};

/*_____________________________________
 |
 |
 |_____________________________________
*/

ScrTypeBase ScrTypeActualBase(ScrType *t) {
	ScrTypeBase r;
	r = Call(t,Base,0);
    while (r==ScbLabeled) {
		t = Call(t,Label.Base,0);
		r = Call(t,Base,0);
	}
	return r;
}

/*______________________________________
 |
 | ScrType
 |______________________________________
*/

static void TypeNullBrowse(ScrType *this,ScrTypeBrowser *Tgt) {}
static ScrTypeBase TypeNullBase(ScrType *this) { return ScbVoid;}
static ScrType *TypeNullGetPtr(ScrType *this) { return &ScrTypeNull; }
static ScrType *TypeNullPtrTarget(ScrType *this) { return this;}
static int TypeNullStructEltNb(ScrType *this) { return 0;}
static ScrType *TypeNullStructElt(ScrType *this,char **Label,int Idx) { *Label=""; return 0;}
static int TypeNullStructEltNum(ScrType *this,char *Label) { return -1; }
static char *TypeNullStructEltLabel(ScrType *this,int Idx) { return "";}
static ScrType *TypeNullFnResult(ScrType *this) { return this;}
static ScrType *TypeNullFnParam(ScrType *this) { return this;}
static int TypeNullArrayEltNb(ScrType *this) { return 0;}
static ScrType *TypeNullArrayEltType(ScrType *this) { return this;}
static ScrType *TypeNullListEltType(ScrType *this) {return this;}
static char *TypeNullLabelLabel(ScrType *this) { return "";}
static ScrType *TypeNullLabelBase(ScrType *this) { return this; }

static struct ScrType TypeNull = {
    &TypeNullBrowse, &TypeNullBase, &TypeNullGetPtr,
	{&TypeNullPtrTarget},
	{&TypeNullStructEltNb,&TypeNullStructElt,&TypeNullStructEltNum},
	{&TypeNullFnResult,&TypeNullFnParam},
	{&TypeNullArrayEltNb,&TypeNullArrayEltType},
	{&TypeNullListEltType},
	{&TypeNullLabelLabel,&TypeNullLabelBase}
};
ScrType ScrTypeNull = {&TypeNull};

    /*_______________________
	 |
	 | Throu
	 |_______________________
	*/

static void TypeThrouBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ThisToThat(ScrTypeThrou,ScrType);
	Call(that->Child,Browse,1(Tgt));
}
static ScrTypeBase TypeThrouBase(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Base,0);
}
static ScrType *TypeThrouGetPtr(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,GetPtr,0);
}
static ScrType *TypeThrouPtrTarget(ScrType *this) { 
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Ptr.Target,0);
}
static int TypeThrouStructEltNb(ScrType *this) { 
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Struct.EltNb,0);
}
static ScrType *TypeThrouStructElt(ScrType *this,char **Label,int Idx) { 
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Struct.Elt,2(Label,Idx));
}
static int TypeThrouStructEltNum(ScrType *this,char *Label) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Struct.EltNum,1(Label)); 
}
static ScrType *TypeThrouFnResult(ScrType *this) { 
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Fn.Result,0);
}
static ScrType *TypeThrouFnParam(ScrType *this) { 
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Fn.Param,0);
}
static int TypeThrouArrayEltNb(ScrType *this) { 
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Array.EltNb,0);
}
static ScrType *TypeThrouArrayEltType(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Array.EltType,0);
}
static ScrType *TypeThrouListEltType(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,List.EltType,0);
}
static char *TypeThrouLabelLabel(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Label.Label,0);
}
static ScrType *TypeThrouLabelBase(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return Call(that->Child,Label.Base,0);
}

static ScrTypeThrou *ScrTypeThrouNew(void) {
	ScrTypeThrou *r;
	static struct ScrType Static = {
		TypeThrouBrowse, TypeThrouBase, TypeThrouGetPtr,
	    {TypeThrouPtrTarget},
	    {TypeThrouStructEltNb,TypeThrouStructElt,TypeThrouStructEltNum},
	    {TypeThrouFnResult,TypeThrouFnParam},
	    {TypeThrouArrayEltNb,TypeThrouArrayEltType},
	    {TypeThrouListEltType},
	    {TypeThrouLabelLabel,TypeThrouLabelBase}
	};
	rPush(r);
	r->ScrType.Static = &Static;
	r->Child = &ScrTypeNull;
	return r;
}

/*_________________________________________
 |
 | Raw type only contains raw information
 | (without mapping details)
 |_________________________________________
*/

typedef struct RawType RawType;
typedef struct {
	ScrTypeThrou ScrTypeThrou;
	RawType *Parent;
	List/*<RawElt.Ptr>*/ Ptr;
} RawElt;
struct RawType {
	MemStack *Mem;
	ScrTypeBrowser ScrTypeBrowser;
	ScrTypeStructBrowser ScrTypeStructBrowser;
	List/*<TypeStruct.List>*/ Struct;
	ScrType *Int,*Char,*String,*Void,*Path;
	/* ToDo: include other structures to avoid lists, pointers and arrays duplication... */
};

static RawElt *RawEltNew(RawType *this,ScrType *t);

/*______________________
 |
 | Raw Elt
 |______________________
*/


    /*____________________________________________________________________________
	 |
	 | Atomic Type:
	 |     * Fixed Size
	 |     * Mapping on Ptr has no meaning
	 |     * Mapped as a structure with a single element "return" of type parent
     |     * Mapped as the identity function
	 |     * Mapped as an Array with one single element
	 |     * Mapped as a List of one single element
	 |     * Mapping on Label has no meaning
	 |____________________________________________________________________________
	*/

extern ScrType ScrTypeVoidPtr;

static int strsup(char *p,char *q) {
	while ((*p==*q)&&(*p)) { p++; q++;}
	return (*p==*q)?0:((*p>*q)?1:-1);
}
static void TypeAtomBrowse(ScrType *this,ScrTypeBrowser *Tgt) { 
	ScrTypeBase t;
	t = Call(this,Base,0);
	Call(Tgt,Atom,1(t));
}
static ScrType *TypeAtomGetPtr(ScrType *this) { return &ScrTypeVoidPtr; }
static int TypeAtomStructEltNb(ScrType *this) { return 1;}
static ScrType *TypeAtomStructElt(ScrType *this,char **Label,int Idx) { 
	if (!Idx) {*Label="return";} else { *Label="";}
	return (Idx==0)?this:&ScrTypeNull; 
}
static int TypeAtomStructEltNum(ScrType *this,char *Label) { return (strsup(Label,"return")==0)?0:-1; }
static ScrType *TypeAtomFnResult(ScrType *this) { return this;}
static ScrType *TypeAtomFnParam(ScrType *this) { return this; }
static int TypeAtomArrayEltNb(ScrType *this) { return 1; }
static ScrType *TypeAtomArrayEltType(ScrType *this) { return this; }
static ScrType *TypeAtomListEltType(ScrType *this) { return this;}

static ScrTypeBase TypeVoidBase(ScrType *this) { return ScbVoid; }
static struct ScrType TypeVoid = {
	TypeAtomBrowse, TypeVoidBase, TypeAtomGetPtr,
	{TypeNullPtrTarget},
	{TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	{TypeAtomFnResult,TypeAtomFnParam},
	{TypeAtomArrayEltNb,TypeAtomArrayEltType},
	{TypeAtomListEltType},
	{TypeNullLabelLabel,TypeNullLabelBase}
};
ScrType ScrTypeVoid = {&TypeVoid};

static ScrTypeBase TypeVoidPtrBase(ScrType *this) { return ScbPtr; }
static ScrType *TypeVoidPtrTarget(ScrType *this) { return &ScrTypeVoid; }
static struct ScrType TypeVoidPtr = {
	TypeAtomBrowse, TypeVoidPtrBase, TypeAtomGetPtr,
	{TypeVoidPtrTarget},
	{TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	{TypeAtomFnResult,TypeAtomFnParam},
	{TypeAtomArrayEltNb,TypeAtomArrayEltType},
	{TypeAtomListEltType},
	{TypeNullLabelLabel,TypeNullLabelBase}
};
ScrType ScrTypeVoidPtr = {&TypeVoidPtr};

static ScrTypeBase TypePathBase(ScrType *this) { return ScbPath; }
static struct ScrType TypePath = {
	TypeAtomBrowse, TypePathBase, TypeAtomGetPtr,
	{TypeNullPtrTarget},
	{TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	{TypeAtomFnResult,TypeAtomFnParam},
	{TypeAtomArrayEltNb,TypeAtomArrayEltType},
	{TypeAtomListEltType},
	{TypeNullLabelLabel,TypeNullLabelBase}
};
ScrType ScrTypePath = {&TypePath};

static ScrTypeBase TypeCharBase(ScrType *this) { return ScbChar;}
static struct ScrType TypeChar = {
	TypeAtomBrowse, TypeCharBase, TypeAtomGetPtr,
	{TypeNullPtrTarget},
	{TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	{TypeAtomFnResult,TypeAtomFnParam},
	{TypeAtomArrayEltNb,TypeAtomArrayEltType},
	{TypeAtomListEltType},
	{TypeNullLabelLabel,TypeNullLabelBase}
};
ScrType ScrTypeChar = {&TypeChar};

static ScrTypeBase TypeIntBase(ScrType *this) { return ScbInt;}
static struct ScrType TypeInt = {
	TypeAtomBrowse,TypeIntBase, TypeAtomGetPtr,
	{TypeNullPtrTarget},
	{TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	{TypeAtomFnResult,TypeAtomFnParam},
	{TypeAtomArrayEltNb,TypeAtomArrayEltType},
	{TypeAtomListEltType},
	{TypeNullLabelLabel,TypeNullLabelBase}
};
ScrType ScrTypeInt = {&TypeInt};

static ScrTypeBase TypeStringBase(ScrType *this) { return ScbString;}
static struct ScrType TypeString = {
	TypeAtomBrowse, TypeStringBase, TypeAtomGetPtr,
	{TypeNullPtrTarget},
	{TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	{TypeAtomFnResult,TypeAtomFnParam},
	{TypeAtomArrayEltNb,TypeAtomArrayEltType},
	{TypeAtomListEltType},
	{TypeNullLabelLabel,TypeNullLabelBase}
};
ScrType ScrTypeString = {&TypeString};


    /*-------------------*/

static ScrType *RawTypeAtom(ScrTypeBrowser *this,ScrTypeBase Atom) {
	ScrType *r;
	ThisToThat(RawType,ScrTypeBrowser);
	switch (Atom) {
	case ScbInt: r = that->Int; break;
	case ScbChar: r = that->Char; break;
	case ScbString: r = that->String; break;
	case ScbPath: r = that->Path; break;
	default: r = that->Void;
	}
	return r;
}

    /*___________________
	 |
	 | PtrType
	 |___________________
	*/

static void TypePtrBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ThisToThat(ScrTypeThrou,ScrType);
	Call(Tgt,Ptr,1(that->Child));
}
static ScrTypeBase TypePtrBase(ScrType *this) { return ScbPtr;}
static ScrType *TypePtrPtrTarget(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return that->Child;
}
static ScrType *ScrTypePtr(ScrTypeBrowser *this,ScrType *Base) {
	return Call(Base,GetPtr,0);
}

static ScrType *TypeRawGetPtr(ScrType *this) {
    ScrType *r;
	ThisToThat(RawElt,ScrTypeThrou.ScrType);
	if (that->Ptr.n==ListEnd) {
		RawElt *p;
		static struct ScrType Static = {
		    TypePtrBrowse, TypePtrBase, TypeRawGetPtr,
		    {TypePtrPtrTarget},
	        {TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	        {TypeAtomFnResult,TypeAtomFnParam},
	        {TypeAtomArrayEltNb,TypeAtomArrayEltType},
	        {TypeAtomListEltType},
	        {TypeNullLabelLabel,TypeNullLabelBase}
		};
		mPush(that->Parent->Mem,p); p->ScrTypeThrou.ScrType.Static = &Static;
		p->ScrTypeThrou.Child = this;
		p->Parent = that->Parent;
		p->Ptr.n = that->Ptr.n; that->Ptr.n = &p->Ptr;
		r = &p->ScrTypeThrou.ScrType;
	} else {
		RawElt *p;
		p = CastBack(RawElt,Ptr,that->Ptr.n);
		r = &p->ScrTypeThrou.ScrType;
	}
    return r;
}
static RawElt *RawEltNew(RawType *this,ScrType *t) {
	RawElt *r;
	static struct ScrType Static = {
		TypeThrouBrowse, TypeThrouBase, TypeRawGetPtr,
		{TypeThrouPtrTarget},
		{TypeThrouStructEltNb,TypeThrouStructElt,TypeThrouStructEltNum},
		{TypeThrouFnResult,TypeThrouFnParam},
		{TypeThrouArrayEltNb,TypeThrouArrayEltType},
		{TypeThrouListEltType},
		{TypeThrouLabelLabel,TypeThrouLabelBase}
	};
	mPush(this->Mem,r); r->ScrTypeThrou.ScrType.Static = &Static;
	r->ScrTypeThrou.Child = t;
	r->Parent = this; r->Ptr.n = ListEnd;
	return r;
}

static ScrTypeThrou *RawTypeThrou(ScrTypeBrowser *this) {
	RawElt *r;
	static struct ScrType Static = {
		TypeThrouBrowse, TypeThrouBase, TypeRawGetPtr,
	    {TypeThrouPtrTarget},
	    {TypeThrouStructEltNb,TypeThrouStructElt,TypeThrouStructEltNum},
	    {TypeThrouFnResult,TypeThrouFnParam},
	    {TypeThrouArrayEltNb,TypeThrouArrayEltType},
	    {TypeThrouListEltType},
	    {TypeThrouLabelLabel,TypeThrouLabelBase}
	};
	ThisToThat(RawType,ScrTypeBrowser);
	mPush(that->Mem,r);
	r->ScrTypeThrou.ScrType.Static = &Static;
	r->ScrTypeThrou.Child = &ScrTypeNull;
	r->Parent = that;
	r->Ptr.n = ListEnd;
	return &r->ScrTypeThrou;
}

    /*___________________
	 |
	 | ListType
	 |___________________
	*/

static void TypeListBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ThisToThat(ScrTypeThrou,ScrType);
	Call(Tgt,List,1(that->Child));
}
static ScrTypeBase TypeListBase(ScrType *this) { return ScbList;}
static ScrType *ScrTypeList(ScrTypeBrowser *this,ScrType *Base) {
	RawElt *R;
	static struct ScrType Static = {
		TypeListBrowse, TypeListBase, TypeRawGetPtr,
		{TypePtrPtrTarget},
	    {TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	    {TypeAtomFnResult,TypeAtomFnParam},
	    {TypeAtomArrayEltNb,TypeAtomArrayEltType},
	    {TypeAtomListEltType},
	    {TypeNullLabelLabel,TypeNullLabelBase}
	};
	ThisToThat(RawType,ScrTypeBrowser);
	mPush(that->Mem,R);
	R->ScrTypeThrou.ScrType.Static = &Static;
	R->ScrTypeThrou.Child = Base;
	R->Parent = that;
	R->Ptr.n = ListEnd;
	return &R->ScrTypeThrou.ScrType;
}

     /*___________________
	  |
	  | LabelType
	  |___________________
	 */

typedef struct {
	RawElt RawElt;
	char *Label;
} TypeLabel;
static void TypeLabelBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ThisToThat(TypeLabel,RawElt.ScrTypeThrou.ScrType);
    Call(Tgt,Labeled,2(that->Label,that->RawElt.ScrTypeThrou.Child));
}
static ScrTypeBase TypeLabelBase(ScrType *this) { return ScbLabeled; }
static char *TypeLabelLabelLabel(ScrType *this) {
	ThisToThat(TypeLabel,RawElt.ScrTypeThrou.ScrType);
	return that->Label;
}
static ScrType *TypeLabelLabelBase(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return that->Child;
}
static ScrType *ScrTypeLabeled(ScrTypeBrowser *this,char *Label,ScrType *Base) {
	TypeLabel *R;
	static struct ScrType Static = {
		TypeLabelBrowse, TypeLabelBase, TypeRawGetPtr,
		{TypeThrouPtrTarget},
	    {TypeThrouStructEltNb,TypeThrouStructElt,TypeThrouStructEltNum},
	    {TypeThrouFnResult,TypeThrouFnParam},
	    {TypeThrouArrayEltNb,TypeThrouArrayEltType},
	    {TypeThrouListEltType},
	    {TypeLabelLabelLabel,TypeLabelLabelBase}
    };
	ThisToThat(RawType,ScrTypeBrowser);
	mPush(that->Mem,R);
	R->RawElt.ScrTypeThrou.ScrType.Static = &Static;
	R->RawElt.ScrTypeThrou.Child = Base;
	R->RawElt.Parent = that;
	R->RawElt.Ptr.n = ListEnd;
	R->Label = Label;
	return &R->RawElt.ScrTypeThrou.ScrType;
}

/*_____________________________________________________
 |
 | Fn Type
 |     When part of a complex type, FnType is mapped
 | as a function pointer.
 |_____________________________________________________
*/

typedef struct {
	RawElt RawElt;
	ScrType *Param;
} TypeFn;
static void TypeFnBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ThisToThat(TypeFn,RawElt.ScrTypeThrou.ScrType);
	Call(Tgt,Fn,2(that->RawElt.ScrTypeThrou.Child,that->Param));
}
static ScrTypeBase TypeFnBase(ScrType *this) { return ScbFn;}
static ScrType *TypeFnFnResult(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return that->Child;
}
static ScrType *TypeFnFnParam(ScrType *this) {
	ThisToThat(TypeFn,RawElt.ScrTypeThrou.ScrType);
	return that->Param;
}
static ScrType *ScrTypeFn(ScrTypeBrowser *this,ScrType *Result,ScrType *Param) {
	TypeFn *R;
	static struct ScrType Static = {
		TypeFnBrowse, TypeFnBase, TypeRawGetPtr,
		{TypeNullPtrTarget},
	    {TypeThrouStructEltNb,TypeThrouStructElt,TypeThrouStructEltNum},
	    {TypeFnFnResult,TypeFnFnParam},
	    {TypeAtomArrayEltNb,TypeAtomArrayEltType},
	    {TypeAtomListEltType},
	    {TypeNullLabelLabel,TypeNullLabelBase}
    };
	ThisToThat(RawType,ScrTypeBrowser);
	mPush(that->Mem,R);
	R->RawElt.ScrTypeThrou.ScrType.Static = &Static;
	R->RawElt.ScrTypeThrou.Child = Result;
	R->RawElt.Parent = that;
	R->RawElt.Ptr.n = ListEnd;
	R->Param = Param;
	return &R->RawElt.ScrTypeThrou.ScrType;
}

/*______________________________
 |
 | Array Type
 |     Known fixed size only.
 |______________________________
*/

typedef struct {
	RawElt RawElt;
	int nb;
} TypeArray;

static void TypeArrayBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ThisToThat(TypeArray,RawElt.ScrTypeThrou.ScrType);
	Call(Tgt,Array,2(that->nb,that->RawElt.ScrTypeThrou.Child));
}
static ScrTypeBase TypeArrayBase(ScrType *this) { return ScbArray;}
static int TypeArrayArrayEltNb(ScrType *this) {
	ThisToThat(TypeArray,RawElt.ScrTypeThrou.ScrType);
	return that->nb;
}
static ScrType *TypeArrayArrayEltType(ScrType *this) {
	ThisToThat(ScrTypeThrou,ScrType);
	return that->Child;
}
static ScrType *ScrTypeArray(ScrTypeBrowser *this,int nb,ScrType *Base) {
	TypeArray *R;
	static struct ScrType Static = {
		TypeArrayBrowse, TypeArrayBase, TypeRawGetPtr,
		{TypePtrPtrTarget},
	    {TypeAtomStructEltNb,TypeAtomStructElt,TypeAtomStructEltNum},
	    {TypeAtomFnResult,TypeAtomFnParam},
	    {TypeArrayArrayEltNb,TypeArrayArrayEltType},
	    {TypeAtomListEltType},
	    {TypeNullLabelLabel,TypeNullLabelBase}
	};
	ThisToThat(RawType,ScrTypeBrowser);
	mPush(that->Mem,R); R->RawElt.ScrTypeThrou.ScrType.Static = &Static;
	R->nb = nb;
	R->RawElt.ScrTypeThrou.Child = Base;
	R->RawElt.Parent = that;
	R->RawElt.Ptr.n = ListEnd;
	return &R->RawElt.ScrTypeThrou.ScrType;
}

/*______________________________
 |
 | Struct Type
 |______________________________
*/

#include <Patricia.h>

typedef struct {
	PatriciaNod/*<StructElt.Label>*/ Label;
	ScrType *EltType;
} StructElt;
typedef struct {
	RawElt RawElt;
	ScrTypeBase Type;
	char *Label;
	int EltNb;
	PatriciaNod/*<StructElt.Label>*/ Labels;
	StructElt *Elts;
	List/*<TypeStruct.List>*/ List;
} TypeStruct;

static void TypeStructBrowse(ScrType *this,ScrTypeBrowser *Tgt) {
	ScrTypeStructBrowser *fs;
	ThisToThat(TypeStruct,RawElt.ScrTypeThrou.ScrType);
	fs = Call(Tgt,Struct,3(that->Label,that->RawElt.ScrTypeThrou.Child,that->EltNb));
	if (fs!=&ScrTypeStructBrowserNull) {
		StructElt *p,*e;
		int num;
		p = that->Elts; e = p+that->EltNb; num = 0;
		while (p<e) {
			Call(fs,Add,3(num,p->Label.Key,p->EltType));
			p++; num++;
		}
		Call(fs,End,0);
	}
}

static ScrTypeBase TypeStructBase(ScrType *this) { 
	ThisToThat(TypeStruct,RawElt.ScrTypeThrou.ScrType); 
	return that->Type; 
}
static int TypeStructStructEltNb(ScrType *this) {
	ThisToThat(TypeStruct,RawElt.ScrTypeThrou.ScrType);
	return that->EltNb;
}
static ScrType *TypeStructStructElt(ScrType *this,char **Label,int Idx) {
	ScrType *r;
	ThisToThat(TypeStruct,RawElt.ScrTypeThrou.ScrType);
    if ((Idx<0)||(Idx>=that->EltNb)) {
		r = &ScrTypeNull; *Label = that->Label;
	} else {
		r = that->Elts[Idx].EltType; *Label = that->Elts[Idx].Label.Key;
	}
	return r;
}
static int TypeStructStructEltNum(ScrType *this,char *Label) {
	int r;
	PatriciaNod *p;
	ThisToThat(TypeStruct,RawElt.ScrTypeThrou.ScrType);
    p = PatriciaSeek(&that->Labels,Label);
	if (p==&that->Labels) {
		r = -1;
	} else {
		StructElt *P;
		P = CastBack(StructElt,Label,p);
		r = P-that->Elts;
	}
	return r;
}

static char *TypeStructLabelLabel(ScrType *this) {
	ThisToThat(TypeStruct,RawElt.ScrTypeThrou.ScrType);
	return that->Label;
}
static ScrType *TypeStructLabelBase(ScrType *this){
	ThisToThat(ScrTypeThrou,ScrType);
	return that->Child;
}

   /*_____________________________
	|
	|
	|_____________________________
   */

static void RawTypeStructAdd(ScrTypeStructBrowser *this,int num,char *Label,ScrType *Type) {
	TypeStruct *c;
	StructElt *n;
	ThisToThat(RawType,ScrTypeStructBrowser);
	c = CastBack(TypeStruct,List,that->Struct.n);
	n = c->Elts+num;
	n->EltType = Type;
	PatriciaSeekOrInsert(&c->Labels,Label,&n->Label);
}
static ScrType *RawTypeStructEnd(ScrTypeStructBrowser *this) {
	TypeStruct *r;
	ThisToThat(RawType,ScrTypeStructBrowser);
	r = CastBack(TypeStruct,List,that->Struct.n);
	that->Struct.n = that->Struct.n->n;
	r->List.n = ListEnd;
	return &r->RawElt.ScrTypeThrou.ScrType;
}
static void RawTypeStructMap(ScrTypeStructBrowser *this,int offset,int num,char *Label,ScrType *Type) {
	Call(this,Add,3(num,Label,Type));
}
static ScrType *RawTypeStructMapEnd(ScrTypeStructBrowser *this,char *Label,int align,int Size){
	return Call(this,End,0);
}

static ScrTypeStructBrowser *RawTypeStructBrowser(ScrTypeBrowser *this,char *Label,ScrType *Base,int FieldNb){
	int i;
	TypeStruct *R;
	static struct ScrType Static = {
		TypeStructBrowse, TypeStructBase, TypeRawGetPtr,
		{TypeNullPtrTarget},
	    {TypeStructStructEltNb,TypeStructStructElt,TypeStructStructEltNum},
	    {TypeAtomFnResult,TypeAtomFnParam},
	    {TypeAtomArrayEltNb,TypeAtomArrayEltType},
	    {TypeAtomListEltType},
	    {TypeStructLabelLabel,TypeStructLabelBase}
	};
	ThisToThat(RawType,ScrTypeBrowser);
	mPush(that->Mem,R);
	R->RawElt.ScrTypeThrou.ScrType.Static = &Static;
	R->RawElt.ScrTypeThrou.Child = Base;
	R->RawElt.Parent = that;
	R->RawElt.Ptr.n = ListEnd;
	R->Type = ScbStruct;
	R->Label = Label;
	PatriciaInit(&R->Labels);
	R->EltNb = FieldNb;
	mnPush(that->Mem,R->Elts,FieldNb);
	i = 0; while(i<FieldNb) {
		R->Elts[i].EltType = &ScrTypeNull;
		PatriciaInit(&R->Elts[i].Label);
		i++;
	}
	R->List.n = that->Struct.n; that->Struct.n = &R->List;
	return &that->ScrTypeStructBrowser;
}

ScrTypeBrowser *ScrRawType(void){
	MemStack *Mem;
	RawType *r;
	static struct ScrTypeStructBrowser StructStatic = {
		RawTypeStructAdd,RawTypeStructEnd,RawTypeStructMap,RawTypeStructMapEnd
	};
	static struct ScrTypeBrowser Static = {
		RawTypeAtom,RawTypeStructBrowser,ScrTypeFn,ScrTypePtr,ScrTypeArray,
		ScrTypeList,ScrTypeLabeled,RawTypeThrou
	};
	Mem = rFork(2048);
	mPush(Mem,r); r->Mem = Mem; r->ScrTypeBrowser.Static = &Static;
	r->ScrTypeStructBrowser.Static = &StructStatic;
	r->Struct.n = ListEnd;
	mIn(r->Mem,r->Int = &RawEltNew(r,&ScrTypeInt)->ScrTypeThrou.ScrType);
	mIn(r->Mem,r->String = &RawEltNew(r,&ScrTypeString)->ScrTypeThrou.ScrType);
	mIn(r->Mem,r->Void = &RawEltNew(r,&ScrTypeVoid)->ScrTypeThrou.ScrType);
	mIn(r->Mem,r->Char = &RawEltNew(r,&ScrTypeChar)->ScrTypeThrou.ScrType);
	mIn(r->Mem,r->Path = &RawEltNew(r,&ScrTypePath)->ScrTypeThrou.ScrType);
	return &r->ScrTypeBrowser;
}

