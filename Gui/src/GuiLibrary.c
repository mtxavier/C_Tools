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
#include <Gui.local.h>

#include <List.h>

/*---------------------*/

static int  libNullGetPageId(GuiLibrary *this) {return -1;}
static void libNullReleasePageId(GuiLibrary *this,int Id) {}
static int  libNullImgLoad(GuiLibrary *this,int Id,char *filename) {return (0!=0);}
static int  libNullImgImport(GuiLibrary *this,int Id,ImgDesc *Desc) {return (0!=0);}
static GuiOutput *libNullPicture(GuiLibrary *this,int PageId,Geo2dRectangle *Dim) {return &GuiOutputNull;}
static GuiOutput *libNullSwapPalPicture(GuiLibrary *this,GuiRgbPalette *Pal,int PageId,Geo2dRectangle *Dim) {return &GuiOutputNull;}
static int  libNullRecordEngine(GuiLibrary *this,Render2dEngine *Engine){return (0!=0);}
static void libNullReleaseEngine(GuiLibrary *this,int Engine) {}

static struct GuiLibrary libNullStatic = {
    libNullGetPageId,libNullReleasePageId
	,libNullImgLoad,libNullImgImport,libNullPicture,libNullSwapPalPicture
	,libNullRecordEngine,libNullReleaseEngine
};
GuiLibrary GuiLibraryNull = {&libNullStatic};

/*-------------------------*/

typedef struct { struct lPage *Static;} lPage;
struct lPage { 
	Img2dLib *(*Load)(lPage *this,Render2dEngine *Engine); 
    int IsFree;
};

typedef struct {
	lPage lPage;
	union {
		dList Free;
        ImgDesc Desc;
	} Desc;
} LlPage;

#define lPageIsFree(p) ((p)->lPage.Static->IsFree)
static Img2dLib *lPageNullLoad(lPage *this,Render2dEngine *Engine) {
	return &Img2dLibNull;
}
static lPage *lPageFree(LlPage *r,dList *Before) {
    static struct lPage Static = {&lPageNullLoad,(0==0)};
	dList *p;
	r->lPage.Static = &Static;
	p = &r->Desc.Free;
	p->n = Before->n;
	p->p = Before;
	p->p->n = p->n->p = p;
	return &r->lPage;
}
static lPage *lPageReserve(LlPage *r) {
	static struct lPage Static = {&lPageNullLoad,(0!=0)};
	dList *p;
	r->lPage.Static = &Static;
	p = &r->Desc.Free;
	p->p->n = p->n;
	p->n->p = p->p;
	p->p = p->n = p;
	return &r->lPage;
}

static Img2dLib *lFilePageLoad(lPage *this,Render2dEngine *Engine) {
	ImgDesc **imgs;
	Img2dLib *r;
	ThisToThat(LlPage,lPage);
	pOpen
		pIn(imgs = GuiLoadImage(that->Desc.Desc.Data,Engine->ScreenOrgAtBottom,Engine->AlgnImagePitch));
	    if (imgs[0]) {
            r = Call(Engine,ImportImg2dLib,1(imgs[0]));
		} else {
			r = &Img2dLibNull;
		}
	pClose
	return r;
}
static lPage *lFilePageNew(LlPage *r,char *FileName) {
	static struct lPage Static = {lFilePageLoad,(0!=0)};
    r->lPage.Static = &Static;
	r->Desc.Desc.Dim.w = r->Desc.Desc.Dim.h = 0;
	r->Desc.Desc.Data = FileName;
	return &r->lPage;
}

static Img2dLib *lDescPageLoad(lPage *this,Render2dEngine *Engine) {
	ThisToThat(LlPage,lPage);
	return Call(Engine,ImportImg2dLib,1(&that->Desc.Desc));
}
static lPage *lDescPageNew(LlPage *r,ImgDesc *Desc) {
	static struct lPage Static = {lDescPageLoad,(0!=0)};
	r->lPage.Static = &Static;
	r->Desc.Desc = *Desc;
	return &r->lPage;
}

/*--------------------------*/

#define CollectPageShift 6
#define CollectPageNb 0x40
#define CollectPageMask 0x3f

typedef struct {
	List List;
	struct pageDescCollect *Collect;
} freePageId;

struct pageDescCollect {
	BinTree nPages;
	struct pageDescList {
		struct pageDescCollect *Collect;
        LlPage Page;
	} Pages[CollectPageNb];
};

typedef struct {
	Img2dLib Img2dLib;
	struct pageCollect *Collect;
	Img2dLib *Child;
} imgLibNotLoaded;

struct pageCollect {
	BinTree nPages;
	Render2dEngine *Engine;
	struct enginePages *Root;
	struct pageDescCollect *Desc;
	imgLibNotLoaded Pages[CollectPageNb];
};
static void imgLibNotLoadedInit(struct pageCollect *Collect,imgLibNotLoaded *page);

struct enginePages {
	BinTree bEngine;
	BinTree pEngine;
	Render2dEngine *Engine;
	struct Llib *Lib;
	BinTree Collects;
};

typedef struct Llib {
	GuiLibrary GuiLibrary;
	MemStack *Mem;
	struct {
		BinTree Collects;
		dList Free;
		int CollectNb;
	} PageId;
	BinTree FreeEngines,bEngines,pEngines;
	int EngineNb;
} Llib;

/*------------------------*/


static int firstEngineFound(BinTree *bt,void *clos) {return (0==0);}

static struct enginePages *libFirstFreeEngine(struct Llib *lib) {
	BinTree *bt;
	struct enginePages *r;
	bt = BinTreeForEach(&lib->FreeEngines,firstEngineFound,0);
	if (bt==BinTreeEnd) {
		mPush(lib->Mem,r);
		BinTreeInit(&r->bEngine);
		BinTreeInit(&r->pEngine);
		r->Engine = 0; // &Render2dEngineNull;
		r->Lib = lib;
		BinTreeInit(&r->Collects);
		BinTreeSeekOrInsert(&lib->FreeEngines,lib->EngineNb,&r->bEngine);
		lib->EngineNb++;
	} else {
	    r = CastBack(struct enginePages,bEngine,bt);
	}
	return r;
}

static struct enginePages *libFirstEngine(struct Llib *lib,int *Free) {
	BinTree *bt;
	struct enginePages *r;
	bt = BinTreeForEach(&lib->bEngines,firstEngineFound,0);
	*Free = (bt==BinTreeEnd);
	if (*Free) {
		r = libFirstFreeEngine(lib);
	} else {
		r = CastBack(struct enginePages,bEngine,bt);
	}
	return r;
}


static int imgLibFree(BinTree *bt,void *Clos) {
	struct pageCollect *pC;
	int i;
	pC = CastBack(struct pageCollect,nPages,bt);
	for (i=0;i<CollectPageNb;i++) {
		Call(&pC->Pages[i].Img2dLib,Close,0);
		Call(&pC->Pages[i].Img2dLib,Free,0);
	}
	pC->Engine = pC->Root->Engine;
	return (0!=0);
}
static void enginePagesClean(struct enginePages *engP) {
	BinTreeForEach(&engP->Collects,imgLibFree,0);
}

static int imgLibInit(BinTree *bt,void *Clos) {
	struct pageCollect *pC;
	pC = CastBack(struct pageCollect,nPages,bt);
	pC->Engine = pC->Root->Engine;
	return (0!=0);
}
static void enginePagesInit(struct enginePages *engP) {
	BinTreeForEach(&engP->Collects,imgLibInit,0);
}

static struct pageDescCollect *AddPageDescCollect(Llib *l,int num) {
	int i;
	BinTree *bt;
	dList *Free;
	struct pageDescCollect *n;
	mPush(l->Mem,n);
	Free = &l->PageId.Free;
	for (i=0;i<CollectPageNb;i++) {
		n->Pages[i].Collect = n;
		lPageFree(&(n->Pages[i].Page),Free->p);
	}
	if (num==l->PageId.CollectNb) l->PageId.CollectNb++;
	bt = BinTreeSeek(&l->PageId.Collects,num);
	if (bt!=BinTreeEnd) {
		num = l->PageId.CollectNb;
		do {
			bt = BinTreeSeek(&l->PageId.Collects,num);
			num++;
		} while(bt!=BinTreeEnd);
		l->PageId.CollectNb = num;
	}
	BinTreeSeekOrInsert(&l->PageId.Collects,num,&n->nPages);
	return n;
}

static struct pageDescCollect *PageDescCollectSeekOrInsert(Llib *l,int num) {
	struct pageDescCollect *r;
	BinTree *bt;
    bt = BinTreeSeek(&l->PageId.Collects,num);
	if (bt==BinTreeEnd) {
        r = AddPageDescCollect(l,num);
	} else {
		r = CastBack(struct pageDescCollect,nPages,bt);
	}
	return r;
}

static struct pageCollect *pageCollectSeekOrInsert(struct Llib *lib,struct enginePages *engP,int cPageId) {
	struct pageCollect *r;
	BinTree *bt;
	bt = BinTreeSeek(&engP->Collects,cPageId);
	if (bt==BinTreeEnd) {
		int i;
		struct pageCollect *nw;
		mPush(lib->Mem,nw);
		nw->Engine = engP->Engine;
		nw->Root = engP;
		nw->Desc = PageDescCollectSeekOrInsert(lib,cPageId);
		for (i=0;i<CollectPageNb;i++) {
             imgLibNotLoadedInit(nw,nw->Pages+i);
		}
		bt = BinTreeSeekOrInsert(&engP->Collects,cPageId,&nw->nPages);
	}
	r = CastBack(struct pageCollect,nPages,bt);
	return r;
}


/*------------------------*/

typedef struct {
	GuiOutput GuiOutput;
	imgLibNotLoaded *Last;
	Geo2dRectangle Boundary;
} LPicture;

static void LPictureDisplay(GuiOutput *this,Render2dEngine *Engine) {
	imgLibNotLoaded *Last;
	struct pageCollect *Collect;
	ThisToThat(LPicture,GuiOutput);
	Last = that->Last;
	Collect = Last->Collect;
	if (Collect->Engine!=Engine) {
		Llib *lib;
		BinTree *bt;
		lib = Collect->Root->Lib;
	    bt = BinTreeSeek(&lib->pEngines,((int)Engine));
		if (bt!=BinTreeEnd) {
		    struct pageCollect *page;
		    struct enginePages *engP;
			int Id;
			Id = Last - Collect->Pages;
			engP = CastBack(struct enginePages,pEngine,bt);
		    page = pageCollectSeekOrInsert(lib,engP,Collect->nPages.Id);
		    if (page) {
				that->Last = page->Pages+Id;
		        Call(that->Last->Child,Display,1(&that->Boundary));
			}
		}
	} else {
		Call(Last->Child,Display,1(&that->Boundary));
	}
}
static GuiOutput *LPictureNew(Llib *lib,int Id,Geo2dRectangle *Rectangle) {
	LPicture *R;
	int fr,pId,oId;
	static struct GuiOutput Static = {LPictureDisplay};
	struct enginePages *engP;
	pId = Id >> CollectPageShift;
	oId = Id & CollectPageMask;
	rPush(R);
	R->GuiOutput.Static = &Static;
	/* Not really good, that page might never be used on that engine... */ {
		struct pageCollect *page;
	    engP = libFirstEngine(lib,&fr);
		page = pageCollectSeekOrInsert(lib,engP,pId); 
	    R->Last = page->Pages+oId;
	}
	R->Boundary = *Rectangle;
	return &R->GuiOutput;
}

/*------------------------*/

static struct pageDescList *reclaimPageId(Llib *l,int Id) {
	struct pageDescList *r;
	int cId,oId;
	r = 0;
	cId = Id>>CollectPageShift;
	oId = Id&CollectPageMask;
	{
	    struct pageDescCollect *col;
	    col = PageDescCollectSeekOrInsert(l,cId);
		r = col->Pages+oId;
		if (lPageIsFree(&r->Page)) { // Not allocated
			lPageReserve(&r->Page);
		} else {
			Call(&l->GuiLibrary,ReleasePageId,1(Id));
			r = reclaimPageId(l,Id);
		}
	}
	return r;
}

static int libGetPageId(GuiLibrary *this) {
	int r;
	dList *L;
	struct pageDescList *R;
	ThisToThat(Llib,GuiLibrary);
	L = that->PageId.Free.n;
	if (L==&(that->PageId.Free)) {
		struct pageDescCollect *n;
		n = AddPageDescCollect(that,that->PageId.CollectNb);
		L = that->PageId.Free.n;
	}
	R = CastBack(struct pageDescList,Page.Desc.Free,L);
	lPageReserve(&R->Page);
    r = (R->Collect->nPages.Id<<CollectPageShift)+(R-(R->Collect->Pages+0));
    return r;
}
struct engReleasePageId {
	int nCollect,oCollect;
};
static int engReleasePageId(BinTree *bt,void *Clos) {
	struct enginePages *eP;
	struct engReleasePageId *clos;
	BinTree *col;
	clos = Clos;
	eP = CastBack(struct enginePages,pEngine,bt);
	col = BinTreeSeek(&eP->Collects,clos->nCollect);
	if (col!=BinTreeEnd) {
		struct pageCollect *pC;
		Img2dLib *lb;
		pC = CastBack(struct pageCollect,nPages,col);
		lb = &pC->Pages[clos->nCollect].Img2dLib;
		Call(lb,Close,0);
		Call(lb,Free,0);
	}
	return (0!=0);
}
static void libReleasePageId(GuiLibrary *this,int Id) {
	struct engReleasePageId clos;
	BinTree *bt;
	ThisToThat(Llib,GuiLibrary);
	clos.nCollect = Id>>CollectPageShift;
	clos.oCollect = Id & CollectPageMask;
	bt = BinTreeSeek(&that->PageId.Collects,clos.nCollect);
	if (bt!=BinTreeEnd) {
		struct pageDescCollect *col;
		struct pageDescList *L;
		col = CastBack(struct pageDescCollect,nPages,bt);
		L = col->Pages+clos.oCollect;
		if (!lPageIsFree(&L->Page)) {
			lPageFree(&L->Page,&that->PageId.Free);
		    BinTreeForEach(&that->pEngines,engReleasePageId,&clos);
		}
	}
}
static int libImgLoad(GuiLibrary *this,int Id,char *filename) {
	struct pageDescList *pid;
	ThisToThat(Llib,GuiLibrary);
    pid = reclaimPageId(that,Id);
	if (pid!=0) {
		lFilePageNew(&pid->Page,filename);
	}
    return (pid!=0);
}
static int libImgImport(GuiLibrary *this,int Id,ImgDesc *Desc) {
	struct pageDescList *pid;
	ThisToThat(Llib,GuiLibrary);
	pid = reclaimPageId(that,Id);
	if (pid!=0) {
		lDescPageNew(&pid->Page,Desc);
	}
	return (pid!=0);
}
static GuiOutput *libPicture(GuiLibrary *this,int PageId,Geo2dRectangle *Dim) {
	ThisToThat(Llib,GuiLibrary);
	return LPictureNew(that,PageId,Dim);
}
static GuiOutput *libSwapPalPicture(GuiLibrary *this,GuiRgbPalette *Pal,int PageId,Geo2dRectangle *Dim) {
	ThisToThat(Llib,GuiLibrary);
	return LPictureNew(that,PageId,Dim); // For now.
}
static int libRecordEngine(GuiLibrary *this,Render2dEngine *Engine) {
	struct enginePages *pEng;
	int Id;
	BinTree *b,*p;
	ThisToThat(Llib,GuiLibrary);
	pEng = libFirstFreeEngine(that);
	pEng->Engine = Engine;
	b = &pEng->bEngine; Id = b->Id;
	p = &pEng->pEngine;
	BinTreeRemove(&that->FreeEngines,Id);
	enginePagesInit(pEng);
	BinTreeSeekOrInsert(&that->bEngines,Id,b);
    BinTreeSeekOrInsert(&that->pEngines,(int)Engine,p);
	return Id;
}
static void libReleaseEngine(GuiLibrary *this,int EngineId) {
	BinTree *bt;
	ThisToThat(Llib,GuiLibrary);
    bt = BinTreeSeek(&that->bEngines,EngineId);
	if (bt!=BinTreeEnd) {
		struct enginePages *pEng;
		pEng = CastBack(struct enginePages,bEngine,bt);
		BinTreeRemove(&that->bEngines,EngineId);
		BinTreeRemove(&that->pEngines,pEng->pEngine.Id);
		pEng->Engine = 0;
		enginePagesClean(pEng);
		BinTreeSeekOrInsert(&that->FreeEngines,pEng->bEngine.Id,&pEng->bEngine);
	}
}

GuiLibrary *GuiLibraryNew(void) {
	Llib *r;
	MemStack *Mem;
	static struct GuiLibrary Static = {
		libGetPageId,libReleasePageId,
		libImgLoad,libImgImport,libPicture,libSwapPalPicture,
		libRecordEngine,libReleaseEngine
	};
	Mem = rFork(2048);
	mPush(Mem,r);
	r->Mem = Mem;
	r->GuiLibrary.Static = &Static;
	/* Engines */{
	    r->EngineNb = 0;
		BinTreeInit(&r->bEngines);
		BinTreeInit(&r->pEngines);
		BinTreeInit(&r->FreeEngines);
	}
	/* Page Descriptors */{
		dList *d;
		BinTreeInit(&r->PageId.Collects);
		d  = &r->PageId.Free;
		d->p = d->n = d;
		r->PageId.CollectNb = 0;
	}
	return &r->GuiLibrary;
}

/*------------------------*/

static void imgLibStateUnloaded(imgLibNotLoaded *lib);
static void imgLibStateClosed(imgLibNotLoaded *lib);
static void imgLibStateOpened(imgLibNotLoaded *lib);

#define notLoadedIndex(p) ((p)-(p->Collect->Pages))

static void libNotLoadedInfo(Img2dLib *this,int *w,int *h,ImgFormat *format) {
	int idx;
	LlPage *page;
	ThisToThat(imgLibNotLoaded,Img2dLib);
	idx = (that-that->Collect->Pages);
	page = &that->Collect->Desc->Pages[idx].Page;
	*w = page->Desc.Desc.Dim.w;
	*h = page->Desc.Desc.Dim.h;
	*format = page->Desc.Desc.Format;
}
static void libNotLoadedDisplay(Img2dLib *this,Geo2dRectangle *img) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	imgLibStateClosed(that);
	if (Call(this,Open,0)) {
		Call(this,Display,1(img));
	}
}
static int libNotLoadedOpen(Img2dLib *this) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	imgLibStateClosed(that);
	return Call(this,Open,0);
}
static void libNotLoadedClose(Img2dLib *this) { }
static void libNotLoadedFree(Img2dLib *this) { }

static void imgLibStateUnloaded(imgLibNotLoaded *lib) {
	static struct Img2dLib Static = {
        libNotLoadedInfo,libNotLoadedDisplay,libNotLoadedOpen,libNotLoadedClose,libNotLoadedFree
	};
	lib->Img2dLib.Static = &Static;
}

    /*---------*/

static void libClosedInfo(Img2dLib *this,int *w,int *h,ImgFormat *format) {
	int idx;
	LlPage *page;
	ThisToThat(imgLibNotLoaded,Img2dLib);
	idx = (that-that->Collect->Pages);
	page = &that->Collect->Desc->Pages[idx].Page;
	*w = page->Desc.Desc.Dim.w;
	*h = page->Desc.Desc.Dim.h;
	*format = page->Desc.Desc.Format;
	if (page->lPage.Static->IsFree) {
		Call(this,Free,0);
	}
}
static void libClosedDisplay(Img2dLib *this,Geo2dRectangle *img) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	if (Call(this,Open,0)) {
	    Call(this,Display,1(img));
	}
}
static int libClosedOpen(Img2dLib *this) {
	int idx,r;
	LlPage *page;
	ThisToThat(imgLibNotLoaded,Img2dLib);
	idx = (that-that->Collect->Pages);
	page = &that->Collect->Desc->Pages[idx].Page;
	r = (!page->lPage.Static->IsFree)&&(that->Collect->Engine);
	if (r) {
	    that->Child = Call(&page->lPage,Load,1(that->Collect->Engine));
        imgLibStateOpened(that);
	} else {
		Call(this,Free,0);
	}
	return r;
}
static void libClosedClose(Img2dLib *this) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
}
static void libClosedFree(Img2dLib *this) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	if (that->Child!=this) {
		Call(that->Child,Free,0);
	    that->Child = this;
	}
	imgLibStateUnloaded(that);
}

static void imgLibStateClosed(imgLibNotLoaded *lib) {
	static struct Img2dLib Static = {
		libClosedInfo,libClosedDisplay,libClosedOpen,libClosedClose,libClosedFree
	};
	lib->Img2dLib.Static = &Static;
}

    /*---------*/

static void libOpenedInfo(Img2dLib *this,int *w,int *h,ImgFormat *format) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	Call(that->Child,Info,3(w,h,format));
}
static void libOpenedDisplay(Img2dLib *this,Geo2dRectangle *img) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	Call(that->Child,Display,1(img));
}
static int libOpenedOpen(Img2dLib *this) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	Call(that->Child,Open,0);
}
static void libOpenedClose(Img2dLib *this) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	Call(that->Child,Close,0);
	imgLibStateClosed(that);
}
static void libOpenedFree(Img2dLib *this) {
	ThisToThat(imgLibNotLoaded,Img2dLib);
	Call(that->Child,Close,0);
	imgLibStateClosed(that);
	Call(this,Free,0);
}

static void imgLibStateOpened(imgLibNotLoaded *lib) {
	static struct Img2dLib Static = {
		libOpenedInfo,libOpenedDisplay,libOpenedOpen,libOpenedClose,libOpenedFree
	};
	lib->Img2dLib.Static = &Static;
}

    /*---------*/

static void imgLibNotLoadedInit(struct pageCollect *Collect,imgLibNotLoaded *page) {
	page->Collect = Collect;
	page->Child = &page->Img2dLib;
	imgLibStateUnloaded(page);
}


