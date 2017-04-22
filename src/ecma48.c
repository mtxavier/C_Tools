#include <Classes.h>
#include <StackEnv.h>
#include <List.h>
#include <Tools.h>

/*______________________________________________________________________
 |
 | Terminal
 |----------------------------------------------------------------------
 | Data are coded in internal format as integers. This
 | allows a direct treatment of international codes.
 | For importation/exportation purpose, you might want
 | to run them throu an UTF-8 coder/decoder.
 |----------------------------------------------------------------------
 |
 | Escape sequence an code interpretation are given by the
 | ECMA-048 standard
 | Codes 0x00-0x1f are special characters with the following meaning
 |
 |  0x00 : NUL SOH STX ETX EOT ENQ ACK BEL
 |  0x08 :  BS  HT  LF  VT  FF  CR  SO  SI
 |  0x10 : DLE DC1 DC2 DC3 DC4 NAK SYN ETB
 |  0x18 : CAN  EM SUB ESC IS4 IS3 IS2 IS1
 |
 | Codes 0x80-0x9f are also special character with the following meaning
 |
 |  0x80 : --- --- BPH NBH --- NEL SSA ESA
 |  0x88 : HTS HTJ VTS PLD PLU  RI SS2 SS3
 |  0x90 : DCS PU1 PU2 STS CCH  MW SPA EPA
 |  0x98 : SOS --- SCI CSI  ST OSC  PM APC
 |
 | They might also be represented with the escape sequence begining with
 | the ESC code followed by a code in the range 0x40-0x5f.
 |
 | APC,DCS,OSC,PM,SOS means that the following sequence, terminated by
 | ST, will be sent to the appropriate channel instead of the display.
 | The sequence might be of two types:
 |   * command strings: codes in the range [0x08-0x0d] and [0x20-0x7e]
 |   * character string : any codes except SOS and ST.
 | As for the chanel:
 |   * APC : Application program command
 |   * DCS : Device control string
 |   * OSC : Operating system command
 |   * PM : Privacy message
 |   * SOS : Start of string
 |
 | CSI introduces the control sequence, it must follow the syntax:
 |     CSI P P ... P I I ... I T
 | with 
 |     Ps 0 or more codes in the range 0x30..0x3f
 |     Is 0 or more codes in the range 0x20..0x2f
 |     T a terminal code in the range 0x40..0x7e
 | T is combined with Is to select an operation, P are the parameters.
 | T in the range 0x70..0x7e can be used by the user.
 | For now, the norm gives meaning for selectors of the form 
 | (T) and (0x20 T).
 | (T):
 |   0x40 : ICH CUU CUD CUF CUB CNL CPL CHA
 |   0x48 : CUP CHT  ED  EL  IL  DL  EF  EA
 |   0x50 : DCH SEE CPR  SU  SD  NP  PP CTC
 |   0x58 : ECH CVT CBT SRS PTX SDS SIMD --
 |   0x60 : HPA HPR REP  DA VPA VPR HVP TBC 
 |   0x68 :  SM  MC HPB VPB  RM SGR DSR DAQ
 | (0x20 T):
 |   0x40 :  SL  SR GSM GSS FNT TSS JFY SPI
 |   0x48 :QUAD SSU PFS SHS SVS IGS -- IDCS
 |   0x50 : PPA PPR PPB SPD DTA SHL SLL FNK
 |   0x58 :SPQR SEF PEC SSW SACS SAPV STAB GCC
 |   0x60 :TATE TALE TAC TCC TSR SCO SRCS SCS
 |   0x68 : SLS --- SPL SCP --- --- --- ---
 |
 | Parameters format: Parameters are decimal values separated by the 
 | code 0x3b. The 0x3a delimiter might be used for composite parameter.
 |  * 0x3c-0x3f are not used, normally.
 |  * Empty values are allowed and stand for 'default' value.
 |  * Leading 0x30 ('0') are allowed and ignored.
 |
 | [ESC 0x60] .. [ESC 0x7e] represent control functions:
 |    60 : DMI INT EMI RIS CMD
 |    6e : LS2 LS3
 |    7c : LS3R LS2R LS1R
 |______________________________________________________________________
*/

/*____________________________________________________________________
 |
 | Modes;
 |
 | Graphic Rendition Combination Mode, (reset/replace SGR settings)
 | HEM  Preceding -> insertion push the rest of the line backward
 |      Following -> insertion push the rest of the line forward
 | VEM  Preceding -> line insertion push the current and preceding lines backward
 | IRM Replace -> added characters replace (or are surimposed) on present characters.
 |-----------
 | CRM Control : control functions are performed. Graphic: they are simply rendered (except RM). 
 |-----------
 | BDSM implicit: all manipulations done in the datas
 |      explicit: manipulations done in the datas or on screen, depending on DCSM
 | DCSM presentation: manipulations are done on the screen, the screen cursor is the reference
 |      data: some manipulations are done in the data (there might be hiden characters).
 |      (affect CPR,CR,DCH,DL,EA,ECH,ED,EF,EL,ICH,IL,LF,NEL,RI,SLH,SLL,SPH,SPL)
 | FEAM Execute: formator functions are executed.
 |      Store: formator functions are simply stored.
 |      (affect BPH,BS,CR,DTA,FF,FNT,GCC,GSM,GSS,HPA,HPB,HPR,HT,HTJ,HTS,HVP,JFY,
 |      NEL,PEC,PFS,PLD,PLU,PPA,PPB,PPR,PTX,QUAD,RI,SACS,SAPV,SCO,SCS,SGR,SHS,SLH,
 |      SLL,SLS,SPD,SPI,SPQR,SRCS,SRS,SSU,SSW,STAB,SVS,TAC,TALE,TATE,TBC,TCC,TSS,VPA,
 |      VPB,VPR,VTS)
 | FETM Insert: when transferring data, formator functions might be inserted.
 |      Exclude: only those formator functions stored by FEAM are to be transfered.
 |----------
 | ERM protect: protected areas aren't affected by erasure (EA,ECH,ED,EF,EL)
 | GATM Guard : when doing a transfer, only unguarded area are transfered.
 | MATM Single : only the current selected field is transferred. All: all selected fields are transfered.
 | SATM Select: only selected aread is transferred. All all characters are transmitted.
 | TTM_Cursor: no data passed the cursor is transfered
 |---------
 | SRM Monitor: Data locally entered are imaged. Simultaneous: only data received are imaged.
 |--------
 | KAM Disabled : Keyboard disabled
 |____________________________________________________________________________
*/

typedef struct { int Page,x,y; } TrmPos;
typedef struct { TrmPos b,e; } TrmArea;
typedef struct { int b,e; } TrmRange;
typedef struct { TrmRange Page,Line,Col; } TrmLimits;
typedef struct { struct TrmEngine *Static; } TrmEngine;
typedef struct { struct TrmChanel *Static; } TrmChanel;
typedef struct { struct TrmDisplay *Static} TrmDisplay;
struct TrmDisplay {
    TrmLimits *(Size)(TrmDisplay *this);
	// expected behaviour of resize is to lose data
	// to large to be kept in the new area.
	void (*Resize)(TrmDisplay *this,TrmLimits *L);
	void (*Clear)(TrmDisplay *this,TrmPos *b,TrmPos *e);
	// by convention, data leaving the clip window is lost.
	// Except may be what leaves top of the clip window, 
	// which might be kept in a log.
	// Scroll scrolls the content of the clip windows only.
	void (*ClipWndGet)(TrmDisplay *this,TrmArea *wnd);
	void (*ClipWndSet)(TrmDisplay *this,TrmArea *wnd);
	void (*Scroll)(TrmDisplay *this,int dx,int dy);

	void (*CursorGet)(TrmDisplay *this,TrmPos *r);
	void (*CursorSet)(TrmDisplay *this,int ShowBlink,TrmPos *n);
    int  (*ColorNum)(TrmDisplay *this,int r,int g,int b);
	void (*SetColor)(TrmDisplay *this,int Fg,int Bg); // -1 == no change; -2 transparent
	int  (*GetAttribute)(TrmDisplay *this);
	void (*SetAttribute)(TrmDisplay *this,int value);
	void (*SetFnt)(TrmDisplay *this,int value);
	// Add data: . Cursor is set on the char following the added data
	// Remove data: removed data is the data strictly preceding the cursor.
	void (*AddData)(TrmDisplay *this,int *b,int *e);
	void (*RemoveData)(TrmDisplay *this,int nb);
	// Display that keep the data stream should also keep special chars.
	// Otherwise, they can safely be interpreted and discarded on the spot.
	void (*AddSpecChar)(TrmDisplay *this,int v);
	void (*AddCharMovePos)(TrmDisplay *this,TrmPos *p); 
	void (*AddCharSetPos)(TrmDisplay *this,TrmPos *p);
	void (*SetTabulation)(TrmDisplay *this,int type,int x,int y);
};
#define TrmTabIsVertical(t) (((t)>>0)&1)
#define TrmTabIsLocal(t)    (((t)>>1)&1)
#define TrmTabJustify()     (((t)>>2)&3)
#define TrmTabJustifyLeft 0
#define TrmTabJustifyRight 1
#define TrmTabJustifyCentered 2
#define TrmTabJustifyWide 3
#define TrmTabSetVertical(t,v) (t)=((t)&(-2))|((v)&1)
#define TrmTabSetLocal(t,v) (t)=((t)&(-3))|(((v)&1)<<1)
#define TrmTabSetJustify(t,v) (t)=((t)&(-13))|(((v)&3)<<2)

struct TrmEngine {
	void (*AddData)(TrmEngine *this,int *b,int *e);
};

// Devices attributes: primary: capacities, secondary: Identifier/firmware version
// Primary ex: 
//     "?1;2" -> VT100 with advanced options
//     "?1;0" -> VT101 with no options
//     "?6" -> VT102
//     "?60;1;2;6;8;9;15" -> VT220, options are:
//         1 -> 132 columns
//         2 -> Printer
//         6 -> Selective erase
//         8 -> User defined key
//         9 -> National replacement character sets
//        15 -> Technical character
//        22 -> ANSI colors
//        29 -> ANSI Text locator
//
// Secondary:
//     ">0;95;0" -> VT100, v95, cartridge registration numbers = 0 (always 0)
//     ">1;95;0" -> VT220, v95, 0
//
//
struct TrmDevice {
	void (*Reset)(TrmDevice *this);
	char *(*Attributes)(TrmDevice *this,int Primary);
	int *(*AddData)(TrmDevice *this,int *b,int *e);
	void (*Start)(TrmChanel *this);
	void (*Spec)(TrmDevice *this);
	void (*Pause)(TrmDevice *this);
	void (*Stop)(TrmDevice *this);
};

/*---------------------------*/

typedef struct { struct TrmDecode *Static; } TrmDecode;
struct TrmDecode {
	int *(*Decode)(TrmDecode *this,int *b,int *e);
};

typedef struct {
	List/*<CSIParams.List>*/ List;
	int P[0x10];
} CSIParams;
typedef struct {
	TlsMemPool/*<CSIParams>*/ *Pool;
    CSIParams b[1],*e;
	int *p,Len;
} TrmBuffer;
typedef struct {
	TrmEngine TrmEngine;
	struct {
	     TrmDecode *PC;
	     TrmDecode Raw,Esc,SCI;
		 TrmPBuffer Sequence;
		 struct CSIDecode {
			 TrmDecode TrmDecode;
			 TrmBuffer *Params;
			 int IPos,LastI,State,Custom;
		 } CSI;
		 struct StrDecode {
			 TrmDecode StrDecode,CmdDecode;
			 TrmBuffer *String;
			 int LastChar; 
			 TrmEngine *Target;
		 } Str;
         int LastChar; // Last data char, used with REP function
	} Decode;
	struct {
		int SizeUnit,DefAttr;
        struct { int dx,dy; } FntDim,CelSz;
		int FontMap[10];
		struct DefColor { int r,g,b,num; } DefFg,DefBg;
		int Mode;
	} Settings,InitSettings;
	TrmDisplay *Display;
	TrmDevice *Aux,*Client; // Client: terminal, Aux: printer ...
	TrmEngine *Application,*OS,*Private,*pClient,*Dump,*Host;
} theEngine;

#define TrmGetAttr(attr,shft,msk) (((attr)>>shft)&msk)
#define TrmSetAttr(attr,v,shft,msk) (attr)=((attr)&(~(msk<<shft)))|((v)<<(shft))

#define ModeSetGRCM_Cumulative(m,v) TrmSetAttr(m,v,21,1)
#define ModeSetHEM_Preceding(m,v)   TrmSetAttr(m,v,10,1)
#define ModeSetVEM_Preceding(m,v)   TrmSetAttr(m,v,7,1)
#define ModeSetIRM_Insert(m,v)      TrmSetAttr(m,v,4,1)
#define ModeSetCRM_Graphic(m,v)     TrmSetAttr(m,v,3,1)
#define ModeSetBDSM_Implicit(m,v)   TrmSetAttr(m,v,8,1)
#define ModeSetDCSM_Data(m,v)       TrmSetAttr(m,v,9,1)
#define ModeSetFEAM_Store(m,v)      TrmSetAttr(m,v,13,1)
#define ModeSetFETM_Exclude(m,v)    TrmSetAttr(m,v,14,1)
#define ModeSetERM_All(m,v)         TrmSetAttr(m,v,6,1)
#define ModeSetGATM_All(m,v)        TrmSetAttr(m,v,1,1)
#define ModeSetMATM_Multiple(m,v)   TrmSetAttr(m,v,15,1)
#define ModeSetSATM_All(m,v)        TrmSetAttr(m,v,17,1)
#define ModeSetTTM_All(m,v)      TrmSetAttr(m,v,16,1)
#define ModeSetSRM_Simultaneous(m,v) TrmSetAttr(m,v,12,1)
#define ModeSetSRTM_Diagnostic(m,v) TrmSetAttr(m,v,5,1)
#define ModeSetKAM_Disabled(m,v)    TrmSetAttr(m,v,2,1)
#define ModeSetTSM_Single(m,v)      TrmSetAttr(m,v,18,1)
#define ModeSetPUM_Size(m,v)  TrmSetAttr(m,v,11,1)
#define ModeSetZDM_Default(m,v)  TrmSetAttr(m,v,22,1)

#define ModeGetGRCM_Cumulative(m) TrmGetAttr(m,21,1)
#define ModeGetHEM_Preceding(m)   TrmGetAttr(m,10,1)
#define ModeGetVEM_Preceding(m)   TrmGetAttr(m,7,1)
#define ModeGetIRM_Insert(m)      TrmGetAttr(m,4,1)
#define ModeGetCRM_Graphic(m)     TrmGetAttr(m,3,1)
#define ModeGetBDSM_Implicit(m)   TrmGetAttr(m,8,1)
#define ModeGetDCSM_Data(m)       TrmGetAttr(m,9,1)
#define ModeGetFEAM_Store(m)      TrmGetAttr(m,13,1)
#define ModeGetFETM_Exclude(m)    TrmGetAttr(m,14,1)
#define ModeGetERM_All(m)         TrmGetAttr(m,6,1)
#define ModeGetGATM_All(m)        TrmGetAttr(m,1,1)
#define ModeGetMATM_Multiple(m)   TrmGetAttr(m,15,1)
#define ModeGetSATM_All(m)        TrmGetAttr(m,17,1)
#define ModeGetTTM_All(m)         TrmGetAttr(m,16,1)
#define ModeGetSRM_Simultaneous(m) TrmGetAttr(m,12,1)
#define ModeGetSRTM_Diagnostic(m) TrmGetAttr(m,5,1)
#define ModeGetKAM_Disabled(m)    TrmGetAttr(m,2,1)
#define ModeGetTSM_Single(m)      TrmGetAttr(m,18,1)
#define ModeGetPUM_Size(m)  TrmGetAttr(m,11,1)
#define ModeGetZDM_Default(m)  TrmGetAttr(m,22,1)



typedef struct TrmBufferPos TrmBufferPos;
static  TrmBufferPos *BufferStart(TrmBufferPos *r,TrmBuffer *t);

typedef struct CSIParamList CSIParamList;
static CSIParamList *CSIParamStart(CSIParamList *r,TrmBufferPos *Pos);
static int CSIParamNext(CSIParamList *t,int deflt);
static int CSIParamSubNext(CSIParamPos *t,int deflt);

typedef void (EscapeFn)(theEngine *Engine,int code,CSIParamList *Params);
static EscapeFn *C0SpecialChar[];
static EscapeFn *C1EscapeSeq[];
static EscapFn *CSISeq[];
static EscapFn *CSI20Seq[];
static EscapeFn *EscFn[];

static void EngineReset(theEngine *t) {
	Call(t->Aux,Reset,0);
	Call(t->Client,Reset,0);
	t->Settings = t->InitSettings;
}

/*------------*/

static void BufferInit(TrmBuffer *r,TlsMemPool *Pool) {
	r->Pool = Pool;
	r->e = r->b;
	r->b->List.n = ListEnd;
	r->P = r->b->P;
	r->Len = 0;
}
static void BufferClear(TrmBuffer *t) {
	List *p;
	p = t->b->List.n;
	while (p!=ListEnd) {
	    CSIParams *P;
		P = CastBack(CSIParams,List,p);
		Call(t->Pool,Free,1(P));
		p = p->n;
	}
	t->b->List.n = ListEnd;
	t->p = t->b->P;
	t->Len = 0;
}

static int BufferAdd(TrmBuffer *t,int d) {
	int *b,*e;
	b = t->e->P; e = b+0x10;
	if (t->p>=e) {
		CSIParams *P;
		P = Call(t->Pool,Alloc,0);
		P->List.n = &t->e->List;
		t->e = P;
		t->p = P->P;
	}
	*t->p++ = d;
	t->Len++;
	return t->Len;
}

struct TrmBufferPos {
    TrmBuffer *Buf;
	CSIParams *p;
	List *n;
	int *c,*ec,idx;
};
static TrmBufferPos *BufferStart(TrmBufferPos *r,TrmBuffer *t) {
	int *ec0,*ec1;
	r->Buf = t;
	r->p = t->b;
	r->n = t->b->List.n;
	r->idx = 0;
	r->c = r->p->P; ec0 = r->c+0x10; ec1 = r->c+t->Len;
    r->ec = (ec1<ec0)?ec1:ec0;
	return r;
}
static int BufferPosNext(TrmBufferPos *t) {
	int r,l;
	r = -1;
	if ((t->c>=t->ec)&&(t->n!=ListEnd)&&(t->idx<t->Buf->Len)) {
        List *p;
		p = t->n; t->n = p->n;
		t->p = CastBack(CSIParams,List,p);
		t->c = t->p->P;
		l = t->Buf->Len-t->idx;
		t->ec = t->c + ((l<0x10)?l:0x10);
	} 
	if (t->c<t->ec) {
		r = *t->c++;
		t->idx++;
	}
	return r;
}

struct CSIParamList {
	TrmBufferPos *Pos;
	int p,Sub,pNum,end,Last;
};
static CSIParamList *CSIParamStart(CSIParamList *r,TrmBufferPos *Pos) {
	r->Pos = Pos;
	r->p = 0;
	r->Sub = 0;
	r->Last = BufferPosNext(Pos);
	r->end = (r->Last==-1);
	return r;
}
static CSIParamList *CSIParamBegin(CSIParamList *r) {
	r = CSIParamStart(r,BufferStart(r->Pos,r->Pos->Buf));
	return r;
}
static int CSIParamGetInt(CSIParamList *t,int def) {
	int r,d;
	d = t->Last;
	if ((d>'9')||(d<'0')) {
		r = def;
	} else {
		r = 0;
		while ((d>='0')&&(d<='9')) {
		    r = (r*10) + d-'0';
		    d = BufferPosNext(Pos);
		}
	}
	if (!t->Sub) { t->p = r; t->pNum++; }
	t->Sub = 0;
	if (d>'9') {
		if (d!=';') { t->Sub = d; }
		d = BufferPosNext(Pos); 
	}
	t->end = (d<'0')||(d>=0x40);
	if (t->end) { d = -1; }
	t->Last = d;
	return r;
}
static int CSIParamNext(CSIParamList *t,int def) {
	int r;
	r = def;
	while ((!t->end)&&(t->Sub)) { CSIParamGetInt(t,0); }
	if (!t->end) { r = CSIParamGetInt(t,r); }
	t->p =r;
	return r;
}
static int CSIParamSubNext(CSIParamPos *t,int def) {
	int r;
	r = def;
	if ((!t->end)&&(t->Sub)) { r = CSIParamGetInt(t,r); }
	return r;
}

/*------------*/

static int *BufAddInt(int *b,int val) {
	int *p,*q,*e;
	if (val<0) val = 0;
	p = b;
	while (val) { *p++ = (val%10)+'0'; val = val/10; }
	e = p;
	if (p==b) { 
		*e++ = '0'; 
	} else {
		q = b; p = e-1;
		while (q<p) { val = *q; *q = *p; *p = val; q++; p--; }
	}
	return e;
}

static void CSICmdTransmit(TrmEngine *tgt,int cmd,CSIParamList *P) {
	int *b,*e,*p,len;
	TrmBufferPos *ps;
	ps = P->Pos; len = ps->Buf->Len-ps->idx;
	pOpen
		pnPush(b,8+len);
	    e = b; *e++ = 0x1b; *e++ = 0x5b;
		while (len) { *e++ = BufferPosNext(ps); len--; }
		while (cmd) { *e++ = cmd; cmd = cmd>>8; }
		Call(tgt,AddData,2(b,e));
	pClose
}



/*------------*/

static void EscEnd(theEngine *t) { t->Decode.PC = &t->Decode.Raw; }
static void theEngineAddData(TrmEngine *this,int *b,int *e) {
	ThisToThat(theEngine,TrmEngine);
	while (b!=e) { b = Call(that->Decode.PC,Decode,2(b,e)); }
}
static int *theEngineRawDecode(TrmDecode *this,int *b,int *e) {
	int cont,c;
	int *db,*p;
	ThisToThat(theEngine,Decode.Raw);
	db = p = b; c = 0x20;
	while ((((c>=0x20)&&(c<0x80))||(c>=0xa0))&&(p!=e)) { c = *p++; }
	if (p!=db) {
		that->Decode.LastChar = p[-1];
		Call(that->Display,AddData,2(db,p));
	}
    if ((c>=0)&&(c<0x20)) {
		C0SpecialChar[c](that,c,0);
	}
	if ((c>=0x80)&&(c<0xa0)) {
		C1EscapeSeq[(c-0x80)](that,c,0);
	}
	return p;
}
static int *theEngineEscDecode(TrmDecode *this,int *b,int *e) {
	ThisToThat(theEngine,Decode.Esc);
	if (b!=e) {
        int c;
		c = *b++;
		EscEnd(that);
		if ((c>=0x40)&&(c<0x60)) {
		    C1EscapeSeq[(c-0x40)](that,c,0);
		} else {
		    if ((c>=0x60)&&(c<0x7f)) {
		        EscFn[(c-0x60)](that,c,0);
		    } else {
				int AddData[2];
				AddData[0] = 0x1b; AddData[1] = c;
		        Call(that->Display.Display,AddData,2(AddData,AddData+2));
			}
		}
	}
	return b;
}
static int *theEngineSCIDecode(TrmDecode *this,int *b,int *e) {
	ThisToThat(theEngine,Decode.SCI);
	if (b!=e) { b++; EscEnd(that); }
	return b;
}
static void CSIClean(struct CSIDecode *t) {
	BufferClear(t->Params);
	t->State = 0; t->IPos = 0; t->Custom = (0!=0);
}
static int *theEngineCSIDecode(TrmDecode *this,int *b,int *e) {
	struct CSIDecode *t;
	int err,c,*p;
	ThisToThat(theEngine,Decode.CSI.TrmDecode);
	t = &that->Decode.CSI;
	err = (0!=0); p = b; c = 0;
	while ((p!=e)&&(!err)) {
        c = *p;
		err = (c<0x20)||(c>0x7e);
		if (!err) {
		if (t->State==0) {
			if ((c>=0x30)&&(c<0x40)) {
				t->State = 2;
				t->Custom = (c>0x3b);
			} else {
				if (c<0x30) { t->State = 3; } else { t->State = 4; }
			}
		}
		if ((t->State==1)||(t->State==2)) {
			while ((p!=e)&&(!err)&&(c>=0x30)&&(c<0x40)){
				BufferAdd(t->Params,c);
				t->Custom = t->Custom||̀(c>0x3b);
				if (!err) { p++; if (p!=e) { c = *p; err = (c<0x20)||(c>0x7e); } }
			}
			if ((c<0x30)&&(!err)&&(p!=e)) {
				t->IPos = t->Params.Len;
				t->State=3; 
			}
		}
		if ((t->State==3)&&(!err)&&(p!=e)) {
			err = (c>=0x30)&&(c<0x40);
			while ((!err)&&(p!=e)&&(c>=0x20)&&(c<0x30)) {
				BufferAdd(t->Params,c);
				t->LastI = c;
				p++; if (p!=e) { c=*p; err = (c<0x20)||(c>0x7e)||((c>=0x30)&&(c<0x40));}
			}
		}
		}
	}
	if (!err) {
	    if ((c>=0x40)&&(c<=0x7e)) {
            int Ilen;
			TrmBufferPos Pos;
			CSIParamList Params;
			p++;
			Ilen = t->Params.Len-t->IPos;
			t->Params.Len = t->IPos;
			CSIParamStart(&Params,BufferStart(&Pos,t->Params));
            if (Ilen==0) {
				if (t->Custom) {
					CSICmdTransmit(that->pClient,c,&Params);
				} else {
				    CSISeq[c-0x40](that,c,&Params);
				}
			} else {
				if ((Ilen==1)&&(t->LastI==0x20)&&(!t->Custom)) {
					CSI20Seq[c-0x40](that,c,&Params);
				} else {
					CSICmdTransmit(that->pClient,(c<<8)|(t->LastI),&Params); /* ToDo: case where Ilen>1 */
				}
			}
		    BufferClear(t->Params);
			EscEnd(that);
	    }
	} else {
		BufferClear(t->Params);
		EscEnd(that);
	}
	return p;
}

/*---------------------------*/

static void Rsrvd(theEngine *Engine,int code,CSIParamList *bPrm) { }

//---- Escape sequence
static void spESC(theEngine *Engine,int code,CSIParamList *bPrm) {
	/* Escape */
	Engine->Decode.PC = &Engine->Decode.Esc;
}
static void spSCI (theEngine *Engine,int code,CSIParamList *bPrm) {
	/* Single character */
	Engine->Decode.PC = &Engine->Decode.SCI;
}
static void spCSI(theEngine *Engine,int code,CSIParamList *bPrm) {
	/* Sequence */
	CSIClean(&Engine->Decode.CSI);
	Engine->Decode.PC = &Engine->Decode.CSI.TrmDecode;
}

//---- Messages Passing

static void StrDecodeClear(theEngine *Engine,int Cmd,TrmEngine *Target) {
	struct StrDecode *t;
	List *p;
	t = &Engine->Decode.Str;
	TrmBufferClear(t->String);
	t->LastChar = ' ';
    t->Target = Target;
	if (Cmd) {
		Engine->Decode.PC = &t->CmdDecode.TrmEngine;
	} else {
		Engine->Decode.PC = &t->StrDecode.TrmEngine;
	}
}
static void StrCmdTransmit(struct StrDecode *t) {
	int *b,*e,*p,*ep,*q;
	TrmBuffer *B;
	List *lp;
	B = t->String;
	pOpen
	pnPush(b,B->Len);
	e = b+B->Len;
	q = b; p = B->b->P; lp = B->b->List.n;
	while (q<e) {
	    CSIParams *P;
        ep = p+0x10;
		while ((p<ep)&&(q<e)) { *q++ = *p++; }
		if (q<e) {
			P = CastBack(CSIParams,List,lp);
			lp  = lp->n;
            p = P->P;
		}
	}
	Call(t->Target,AddData,2(b,e));
	pClose
}
static int *theEngineStrDecode(TrmDecode *this,int *b,int *e) {
	struct StrDecode *t;
	int err,end;
	ThisToThat(theEngine,Decode.Str.StrDecode.TrmDecode);
	t = &that->Decode.Str;
	end = (b==e); err = (0!=0);
    while ((b!=e)&&(!end)&&(!err)) {
		c = *b;
		if (t->LastChar==0x1b) {
			end = (c==0x5c);
			err = (c==0x58); // SOS forbidden
			if ((!end)&&(!err)) BufferAdd(t->String,0x1b);
		} else {
			end = (c==0x9c);
			err = (c==0x98);
		}
		if (end||err) {
			if (!err) StrCmdTransmit(t);
			BufferClear(t->String);
			EscEnd(that);
		} else {
			t->LastChar = c;
            if (c!=0x1b) { BufferAdd(t->String,c); }
		}
		b++;
	}
	return b;
}
static int *theEngineCmdDecode(TrmDecode *this,int *b,int *e) {
	struct StrDecode *t;
	int err,end,c;
    ThisToThat(theEngine,Decode.CmdDecode.TrmDecode);
	t = &that->Decode.Str;
	end = (b==e); err = (0!=0);
	while ((b!=e)&&(!end)&&(!err)) {
		c = *b;
		if (t->LastChar==0x1b) {
			end = (c==0x5c);
			err = (c==0x58); // SOS forbidden
			if ((!end)&&(!err)) StrCmdAdd(t,0x1b);
		} else {
			end = (c==0x9c);
			err = (c==0x98);
		}
		err = err||(c<0x8)||((c>0xd)&&(c<0x20))||(c>0x7e);
		if (end||err) {
			if (!err) StrCmdTransmit(t);
			EscEnd(that);
		} else {
			t->LastChar = c;
			if (c!=0x1b) StrCmdAdd(t,c);
		}
	}
	return b;
}
static void spAPC(theEngine *Engine,int code,CSIParamList *b) {
	/* Send following string (Command) to Application. */
	StrDecodeClear(Engine,(0==0),Engine->Application);
}
static void spDCS(theEngine *Engine,int code,CSIParamList *b) {
	/* Send following string (Command) to the Device. Might be a follow up to the last IDCS */
	StrDecodeClear(Engine,(0==0),Engine->pClient);
}
static void spOSC(theEngine *Engine,int code,CSIParamList *b) {
	/* Send following string (Command) to the OS.*/
	StrDecodeClear(Engine,(0==0),Engine->OS);
}
static void spPM(theEngine *Engine,int code,CSIParamList *b)  {
	/* Private message (Command) */
	StrDecodeClear(Engine,(0==0),Engine->Private);
} 
static void spSOS(theEngine *Engine,int code,CSIParamList *b) {
	/* (1b 58 | 98) Start of string (String) */
	StrDecodeClear(Engine,(0!=0),Engine->Dump);
}
static void spST(theEngine *Engine,int code,CSIParamList *b)  {
	/* (1b 5c | 9c) String terminator */ 
	Engine->Decode.PC = &Engine->Decode.Raw;
}

static EscapeFn	spCMD {/*Coding method delimiter*/} 

static void C0Transmit(theEngine *t,int c) { 
	Call(t->Display,AddSpecChar,1(c)); 
}
static void C1Transmit(theEngine *t,int c) {
	int t[2];
	if (c>=0x80) { c-=0x40; }
	t[0] = 0x1b; t[2] = c;
	Call(t->Display,AddData,2(t,t+2));
}

//---- Special keys
// Usually used to extend character set
static EscapeFn spSO  {/* C0 shift Out */}
static EscapeFn spSI  {/* C0 shift In */}
static EscapeFn	spLS2 {/* Cf Locking shift 2 */}
static EscapeFn	spLS3 {/* Cf Locking shift 3 */}
static EscapeFn	spLS3R {/* Cf Locking shift 3 right */}
static EscapeFn	spLS2R {/* Cf Locking shift 2 right */}
static EscapeFn	spLS1R {/* Cf Locking shift 1 right */}
static EscapeFn	spSS2 {/* C1 Single shift 2 */}
static EscapeFn	spSS3 {/* C1 Single shift 3 */}

/* C0 Information separator 4 (Document terminator DT, File Separator FS ) */
/* C0 Information separator 3 (Page terminator PT, Group Separator GS ) */ 
/* C0 Information separator 2 (Record Separator RS ) */
/* C0 Information separator 1 (Unit Separator US ) */
static void spIS4(theEngine *t,int c,CSIParamList *b) {C0Transmit(t,c);}
static void spIS3(theEngine *t,int c,CSIParamList *v) {C0Transmit(t,c);}
static void spIS2(theEngine *t,int c,CSIParamList *b) {C0Transmit(t,c);}
static void spIS1(theEngine *t,int c,CSIParamList *b) {C0Transmit(t,c);}
static void spNUL(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 Null */
static void spBEL(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 Bell */
static void spCR(theEngine *t,int c,CSIParamList *e)  {C0Transmit(t,c);} /* FF Dt/Sc + C0 Carriage return */
static void spHT (theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* FF + C0 Horizontal Tab */ 
static void spVT (theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* + C0 Vertical Tab */
static void spLF(theEngine *t,int c,CSIParamList *e)  {C0Transmit(t,c);} /* Dt/Sc + C0 Line Feed */  
static void spFF(theEngine *t,int c,CSIParamList *e)  {C0Transmit(t,c);} /* FF + C0 Form Feed */
static void spBS(theEngine *t,int c,CSIParamList *e)  {C0Transmit(t,c);} /* FF + C0 BackSpace */
static void spRI(theEngine *t,int c,CSIParamList *e)  {C0Transmit(t,0x8d);} /* FF Dt/Sc + C1 Reverse line feed */
static void spHTJ(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,0x89);} /* FF + C1 Horizontal Tab with justification */
static void spNEL(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,0x86);} /* FF Dt/Sc + C1 Next Line */

//---- ISO 1745
static void spETB(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 End of transmission block */
static void spEOT(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 End of transmission */
static void spEM (theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 End of medium */
static void spSOH(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 Start of heading */
static void spSTX(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 Start of text */
static void spETX(theEngine *t,int c,CSIParamList *e) {C0Transmit(t,c);} /* C0 End of text */

static void spRIS(theEngine *t,int c,CSIParamList *e) {
	/* Cf Reset to initial state */
	EngineReset(t);
}
static void spSM(theEngine *t,int c,CSIParamList *e)  {
	/* f Set Mode */
    int p,m,msk;
	m = t->Settings.Mode;
	while (!e->end) {
		p = CSIParamListNext(e,-1);
	    if (p>0) {
		    msk = 1<<p;
		    m = m|msk;
		}
	}
	t->Settings.Mode = m;
}
static void spRM(theEngine *t,int c,CSIParamList *e)  {
	/* f Reset Mode */
    int p,m,msk;
	m = t->Settings.Mode;
	while (!e->end) {
		p = CSIParamListNext(e,-1);
	    if (p>0) {
		    msk = 1<<p;
		    m = (m&(~msk));
		}
	}
	t->Settings.Mode = m;
}

//---- Auxiliary device

static void spDC1(theEngine *t,int c,CSIParamList *e) {
	/* C0 Device control 1: (usually device Start) */
	Call(t->Aux,Start,0);
}
static void spDC2(theEngine *t,int c,CSIParamList *e) {
	/* C0 Device control 2: (usually device Spec mode) */
	Call(t->Aux,Spec,0);
}
static void spDC3(theEngine *t,int c,CSIParamList *e) {
    /* C0 Device control 3: (usually device Pause) */
	Call(t->Aux,Pause,0);
}
static void spDC4(theEngine *t,int c,CSIParamList *e) {
	/* C0 Device control 4: (usually device Stop) */
	Call(t->Aux,Stop,0);
}
static EscapeFn spMC  {
	/* f Media copy: send data to auxiliary device. */
}

static EscapeFn spIDCS{
	/* f2 Identify device control string */
}
static void spDA(theEngine *t,int c,CSIParamList *e)  {
	/* f Device attributes */
	int p,*bb,*eb,sel;
	char *attr,*ep;
	p = CSIParamNext(e,-1);
	if ((p==0)||((p==-1)&&(!e->Sub))) {
		attr = Call(t->Client,Attributes,1(0));
	    ep = attr; while (*ep) ep++;
	    pOpen
	        pnPush(bb,(ep-attr)+5);
	        ep = attr; eb = bb;
		    *eb++ = 0x1b; *eb++ = 0x5b;
		    while (ep) { *eb++ = *ep++; }
		    *eb++ = 'c';
		    Call(t->Host,AddData,2(bb,eb));
        pClose
	}
}
static void spDSR(theEngine *t,int c,CSIParamList *e) {
	/* f Device status report  */
	int p;
	int r[28],*er;
	p = CSIParamNext(e,0);
	switch (p) {
	case 5: {
		er = r; *er++ = 0x1b; *er++ = 0x5b; *er++='0'; *er++='n';
		Call(t->Host,AddData,2(r,er));
	} break;
	case 6: {
        TrmPos P;
		er = r;
		Call(t->Display,CursorGet,1(&P));
		*er++ = 0x1b; *er++ = 0x5b;
		er = BuffAddInt(er,P.y); *er++ = ';';
		er = BuffAddInt(er,P.x); *er++ = 'R';
		Call(t->Host,AddData,2(r,er));
	} break;
	default: {
		// response is also a DSR msg:
		// 0 -> Ok  
		// 1 -> busy(try later)
		// 2 -> busy(will call back)  
		// 3 -> malfunction(try later)
		// 4 -> malfunction(will call back)
		CSICmdTransmit(that->pClient,0x6e,CSIParamStart(e,BufferStart(e->Pos,e->Pos->Buf)));
	} break;
	}
}

static EscapeFn	spMW  {/* C1 Message waiting */}
static EscapeFn	spBPH {/* FF C1 Break Permitted here*/}
static EscapeFn	spNBH {/* C1 No break here*/}
static EscapeFn	spPU1 {/* C1 Private use 1*/}
static EscapeFn	spPU2 {/* C1 Private use 2*/}
static EscapeFn	spSTS {/* C1 Set transmit state */}

static EscapeFn	spINT {/* Cf Interrupt */}
static EscapeFn	spDMI {/* Cf Disable manual input */}
static EscapeFn	spEMI {/* Cf Enable manual input */}

static EscapeFn spENQ {/* C0 Enquiry */}
static EscapeFn spDLE {/* C0 Data link escape */}
static EscapeFn spACK {/* C0 Acknowledge */}
static EscapeFn spNAK {/* C0 Negative acknowledgement */}
static EscapeFn spSYN {/* C0 synchronous Idle */}
static EscapeFn spCAN {/* C0 Cancel */}
static EscapeFn	spCCH {/* C1 Cancel character */}
static EscapeFn spSUB {/* C0 Substitute (replace erroneous char) */}

//---- Selected/Guarded area
static EscapeFn	spSSA {/* C1 Start Selected area*/}
static EscapeFn	spESA {/* C1 End Selected area*/}
static EscapeFn spDAQ {/* f Area delimiter */}
static EscapeFn	spSPA {/* C1 Start Guarded area*/}
static EscapeFn	spEPA {/* C1 End Guarded area*/}

//---- Tabulations:
static EscapeFn spCTC {/* C f Cursor tabulation control */}
static EscapeFn	spHTS {/* FF + C1 Set horizontal tabulation*/}
static EscapeFn	spVTS {/* FF + C1 Set vertical tabulation*/}
static EscapeFn	spTAC {/* FF f2 Tabulation aligned center*/}
static EscapeFn	spTALE{/* FF f2 Tabulation aligned leading edge*/}
static EscapeFn spTATE{/* FF f2 Tabulation aligned trailing edge*/}
static EscapeFn	spTCC {/* FF f2 Tabulation centered on character*/}
static EscapeFn spTBC {/* FF + f Tabulation clear */}
static EscapeFn spTSR {/* + f2 Tabulation stop remove*/}
static EscapeFn	spSTAB{/* FF f2 Selective tabulation */}

//---- cursor
static EscapeFn	spPLD {/* FF + C1 Partial line forward */}
static EscapeFn	spPLU {/* FF + C1 Partial line backward */}

static EscapeFn	spSEE {/* Select editing extent */}
static EscapeFn spSHL {/* FF Dt/Sc Set f2 home line */}
static EscapeFn spSLL {/* FF Dt/Sc Set f2 Line Limit */}
static EscapeFn spSPL {/* Dt/Sc Set f2 Page Limit */}

static EscapeFn spPPA {/* FF + f2 Page position absolute*/}
static EscapeFn spPPR {/* FF + f2 Page position forward*/}
static EscapeFn spPPB {/* FF + f2 Page position backward*/}

static void spCUU(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor Up */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.y -= CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCUD(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor Down */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.y += CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCUF(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor right */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
    P.x += CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCUB(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor Left */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.x -= CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCNL(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor next line */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.x = 1; P.y += CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCPL(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor preceding line */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.x = 1; P.y -= CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCHA(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor character absolute */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.x = CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCUP(theEngine *t,int c,CSIParamList *e) {
	/* C f Cursor Position */
	TrmPos P;
    Call(t->Display,CursorGet,1(&P));
	P.y = CSIParamNext(e,1);
	P.x = CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spNP(theEngine *t,int c,CSIParamList *e) {
	/* D f Next Page */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.Page += CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spPP(theEngine *t,int c,CSIParamList *e)  {
	/* D f Previous Page */
    TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.Page -= CSIParamNext(e,1);
	Call(t->Display,CursorSet,2(-1,&P));
}
static void spCPR(theEngine *t,int c,CSIParamList *e) {
	/* Dt/Sc f Active position report, usually, the client asked it */
	TrmPos P;
	int *p,b[32];
	P.y = CSIParamNext(e,1);
	P.x = CSIParamNext(e,1);
	p = b; 
	*p++ = 0x1b; *p++ = 0x5b;
	p = BuffAddInt(p,P.y);
	*p++ = ';';
	p = BuffAddInt(p,P.x);
	*p++ = 0x52;
	Call(t->pClient,AddData,2(b,p));
}

static EscapeFn	spCHT {/* C f Cursor forward tabulation */}
static EscapeFn	spCVT {/* C f Cursor vertical tabulation */}
static EscapeFn	spCBT {/* C f Cursor backward tabulation */}

static void spSU(theEngine *t,int c,CSIParamList *b)  {
	/* D f Scroll Up */
	int d;
	d = CSIParamNext(b,1);
	Call(t->Display,Scroll,2(0,-d));
}
static void spSD(theEngine *t,int c,CSIParamList *b)  {
	/* D f Scroll Down */
	int d;
	d = CSIParamNext(b,1);
	Call(t->Display,Scroll,2(0,d));
}
static void spSL(theEngine *t,int c,CSIParamList *b)  {
	/* D f2 Scroll left */
	int d;
	d = CSIParamNext(b,1);
	Call(t->Display,Scroll,2(-d,0));
}
static void spSR(theEngine *t,int c,CSIParamList *b)  {
	/* D f2 Scroll right */
	int d;
	d = CSIParamNext(b,1);
	Call(t->Display,Scroll,2(d,0));
}

static EscapeFn	spSIMD{/* f Select Implicit Movement Direction */ }

static void spHPA(theEngine *t,int c,CSIParamList *b) {
	/* FF + f Character position absolute */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.x = CSIParamNext(b,1);
	Call(t->Display,AddCharSetPos,1(&P));
}
static void spHPB(theEngine *t,int c,CSIParamList *b) {
    /* FF + f Character position backward */
	TrmPos P;
	P.y = P.Page = 0;
	P.x = -CSIParamNext(b,1);
	Call(t->Display,AddCharMovePos,1(&P));
}
static void spHPR(theEngine *t,int c,CSIParamList *b) {
	/* FF + f Character position forward */
	TrmPos P;
	P.y = P.Page = 0;
	P.x = CSIParamNext(b,1);
	Call(t->Display,AddCharMovePos,1(&P));
}
static void spVPA(theEngine *t,int c,CSIParamList *b) {
	/* FF + f Line:absolute */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.y = CSIParamNext(b,1);
	Call(t->Display,AddCharSetPos,1(&P));
}
static void spVPB(theEngine *t,int c,CSIParamList *b) {
    /* FF + f Character position backward */
	TrmPos P;
	P.x = P.Page = 0;
	P.y = -CSIParamNext(b,1);
	Call(t->Display,AddCharMovePos,1(&P));
}
static void spVPR(theEngine *t,int c,CSIParamList *b) {
	/* FF + f Character position forward */
	TrmPos P;
	P.x = P.Page = 0;
	P.y = CSIParamNext(b,1);
	Call(t->Display,AddCharMovePos,1(&P));
}
static void spHVP(theEngine *t,int c,CSIParamList *b) {
	/* FF + f Character and line position */
	TrmPos P;
	Call(t->Display,CursorGet,1(&P));
	P.y = CSIParamNext(b,1);
	P.x = CSIParamNext(b,1);
	Call(t->Display,AddCharSetPos,1(&P));
}
//----- edit

static void spREP(theEngine *t,int c,CSIParamList *b) {
	/* f Repeat */
	if (that->Decode.LastChar>0) {
	    int *p,nb,bb[16],*eb;
	    nb = CSIParamNext(b,0);
	    eb = bb+((nb>16)?16:nb);
	    for (p=bb;p<eb;p++) { *p = t->Decode.LastChar; }
        while (nb) {
			Call(t->Display,AddData,2(bb,eb));
			nb -= (eb-bb);
			eb = bb+((nb>16)?16:nb);
	    }
	}
}

static EscapeFn spICH {
	/* Dt/Sc E f Insert character, Mode HEM */
}
static EscapeFn	spED  {/* Dt/Sc E f Erase in page */}
static EscapeFn	spEL  {/* Dt/Sc E f Erase in line */}
static EscapeFn	spIL  {/* Dt/Sc E f Insert line */}
static EscapeFn	spDL  {/* Dt/Sc E f Delete line */}
static EscapeFn	spEF  {/* Dt/Sc E f Erase in field */} 
static EscapeFn	spEA  {/* Dt/Sc E f Erase in area */}
static EscapeFn spDCH {
	/* Dt/Sc E f Delete character, Mode HEM */
}
static EscapeFn spECH {/* Dt/Sc E f Erase character */}

// Justify and squad fit a line or multiple line to the width of the display.
static EscapeFn spJFY  {/* FF f2 Justify */}
static EscapeFn spQUAD {/* FF f2 Quad */}

//---- Visual

/*_______________________________________________________________
 |
 | Attributes:
 | 1 Bold/increase intensity
 | 2 Faint/decrease intensity/second colour
 | 3 Italicized
 | 4 Single underlined
 | 5 Slow blink
 | 6 Rapid blink
 | 7 Negative
 | 8 Concealed
 | 9 Crossed
 | 10-19 Font select.
 | 20 fraktur (Gothic)
 | 21 doubly underlined
 | 22 normal intensity
 | 23 no italized, nor fraktur
 | 24 not underlined
 | 25 steady (not blinking).
 | 26 (rsrvd) proportional spacing
 | 27 positive image
 | 28 revealed character
 | 29 Not crossed out
 | 30-37 character color : black, red, green, yellow, blue, magenta, cyan, white
 | 38 more character color : extra parameters are used, they are separated by 0x3a
 |     The first extra parameter as the following meaning:
 |         0 -> implementation specific
 |         1 -> Transparent
 |         2;r;g;b
 |         3;C;M;Y
 |         4;C;M;Y;K
 |         5;idx
 | 39 default character color
 | 40-49 same for background
 | 50 (rsrvd) cancel 26
 | 51 framed
 | 52 encircled
 | 53 overlined
 | 54 not framed, not encircled
 | 55 not overlined
 | 56-59 (rsrvd)
 | 60 right line
 | 61 double right line
 | 62 left line
 | 63 double left line
 | 64 ideogram stress marking
 | 65 Cancel 60-64
 | Mapping:
 |   [0..1]: 00 normal, 01 Bold, 10 Faint
 |   [2..3]: 00 normal, 01 Italicized, 10 Fracktur
 |   [4..5]: 00 normal, 01 underlined, 10 double underlined
 |       6 : !crossed
 |       7 : !overlined
 |   [8..9]: 00 normal, 01 framed, 10 encircled
 | [10..11]: 00 normal, 01 slow blink, 10 Rapid blink
 |      12 : !negative
 |      13 : !concealed
 |      14 : 
 |_______________________________________________________________
*/

#define TrmGetAttrBold(attr)       TrmGetAttr(attr,0,3)
#define TrmGetAttrItalic(attr)     TrmGetAttr(attr,2,3)
#define TrmGetAttrUnderlined(attr) TrmGetAttr(attr,4,3)
#define TrmGetAttrCrossed(attr)    TrmGetAttr(attr,6,1)
#define TrmGetAttrOverlined(attr)  TrmGetAttr(attr,7,1)
#define TrmGetAttrFramed(attr)     TrmGetAttr(attr,8,3)
#define TrmGetAttrConcealed(attr)  TrmGetAttr(attr,10,1)
#define TrmGetAttrNegative(attr)   TrmGetAttr(attr,11,1)
#define TrmGetAttrBlink(attr)      TrmGetAttr(attr,12,3)
#define TrmGetAttrRight(attr)      TrmGetAttr(attr,14,3)
#define TrmGetAttrLeft(attr)       TrmGetAttr(attr,16,3)
#define TrmGetAttrIdeogram(attr)   TrmGetAttr(attr,18,1)

#define TrmSetAttrBold(attr,v)       TrmSetAttr(attr,v,0,3)
#define TrmSetAttrItalic(attr,v)     TrmSetAttr(attr,v,2,3)
#define TrmSetAttrUnderlined(attr,v) TrmSetAttr(attr,v,4,3)
#define TrmSetAttrCrossed(attr,v)    TrmSetAttr(attr,v,6,1)
#define TrmSetAttrOverlined(attr,v)  TrmSetAttr(attr,v,7,1)
#define TrmSetAttrFramed(attr,v)     TrmSetAttr(attr,v,8,3)
#define TrmSetAttrConcealed(attr,v)  TrmSetAttr(attr,v,10,1)
#define TrmSetAttrNegative(attr,v)   TrmSetAttr(attr,v,11,1)
#define TrmSetAttrBlink(attr,v)      TrmSetAttr(attr,v,12,3)
#define TrmSetAttrRight(attr)      TrmSetAttr(attr,v,14,3)
#define TrmSetAttrLeft(attr)       TrmSetAttr(attr,v,16,3)
#define TrmSetAttrIdeogram(attr)   TrmSetAttr(attr,v,18,1)


static cym2rgb(int *c0,int *c1,int *c2) {
	int c,y,m,r,g,b;
	c = *c0; y = *c1; m = *c2;
	r = (0x1fe-(y+m))>>1; // 1fe? 1ff ? check as to be made
	g = (0x1fe-(c+y))>>1;
	b = (0x1fe-(m+c))>>1;
	*c0 = r; *c1 = g; *c2 = b;
}
static rgb2cym(int *c0,int *c1,int *c2) {
	int c,y,m,r,g,b;
	r = *c0; g = *c1; b = *c2;
	c = (g+b)>>1;
	y = (r+g)>>1;
	m = (b+r)>>1;
	*c0 = c; *c1 = y; *c2 = m;
}
static void spSGR(theEngine *t,int c,CSIParamList *e) {
	/* FF f Select graphic rendition */
	int attr,oattr,dec;
	oattr = attr = Call(t->Display,GetAttribute,0);
	if (t->Settings.GRCM_Replacing) { // replacing mode cancels effect of last SGR
		attr = t->Settings.DefAttr;
		Call(t->Display,SetFnt,1(0));
		Call(t->Display,SetColor,2(t->Settings.DefFg.num,t->Settings.DefBg.num));
	}
	while (!e->end) {
		dec = CSIParamNext(e,0);
		switch (dec) {
        case 1: TrmSetAttrBold(attr,1); break;
		case 2: TrmSetAttrBold(attr,2); break;
		case 3: TrmSetAttrItalic(attr,1); break;
		case 4: TrmSetAttrUnderlined(attr,1); break;
		case 5: TrmSetAttrBlink(attr,1); break;
		case 6: TrmSetAttrBlink(attr,2); break;
		case 7: TrmSetAttrNegative(attr,1); break;
		case 8: TrmSetAttrConcealed(attr,1); break;
		case 9: TrmSetAttrCrossed(attr,1); break;
		case 10: case 11: case 12: case 13: case 14: 
		case 15: case 16: case 17: case 18:
		case 19: Call(t->Display,SetFnt,1(t->Settings.FontMap[dec-10])); break;
		case 20: TrmSetAttrItalic(attr,2); break;
		case 21: TrmSetAttrUnderlined(attr,2); break;
		case 22: TrmSetAttrBold(attr,0); break;
		case 23: TrmSetAttrItalic(attr,0); break;
		case 24: TrmSetAttrUnderlined(attr,0); break;
		case 25: TrmSetAttrBlink(attr,0); break;
		case 26: /* reserved for proportional spacing */
		case 27: TrmSetAttrNegative(attr,0); break;
		case 28: TrmSetAttrConcealed(attr,0); break;
		case 29: TrmSetAttrCrossed(attr,0); break;
		case 30: case 31: case 32: case 33: 
		case 34: case 35: case 36: 
		case 37: Call(t->Display,SetColor,2(dec-30,-1)); break;
		case 39: Call(t->Display,SetColor,2(t->Settings.DefFg.num,-1)); break;
		case 40: case 41: case 42: case 43:
		case 44: case 45: case 46: 
        case 47: Call(t->Display,SetColor,2(-1,dec-30)); break;
		case 49: Call(t->Display,SetColor,2(-1,t->Settings.DefBg.Num,-1)); break;
		case 38: case 48: {
			int c[2],*cc,sel;
			struct DefColor *rgb;
			c[0]=c[1]=-1;
			if (dec==38) { 
				cc = c+0; rgb = &t->Settings.DefFg; 
			} else {
			    cc = c+1; rgb = &t->Settings.DefBg;
			}
			cc = c+(dec==38)?0:1; rgb = 
			sel = CSIParamNext(e,0);
			switch(sel) {
			case 0: break;
			case 1: *cc = -2; break;
			case 2: {
				int r,g,b;
			    r = CSIParamNext(e,rgb->r);
				g = CSIParamNext(e,rgb->g);
				b = CSIParamNext(e,rgb->b);
				*cc=Call(t->Display,ColorNum,3(r,g,b));
			} break;
			case 3: case 4: {
				int c0,c1,c2;
				c0 = rgb->r; c1 = rgb->g; c2 = rgb->b;
				rgb2cym(&c0,&c1,&c2);
				c0 = CSIParamNext(e,c0);
				c1 = CSIParamNext(e,c1);
				c2 = CSIParamNext(e,c2);
				if (sel==4) {
					k = CSIParamNext(e,0);
					c0+=k; if (c0>0xff) c0 = 0xff;
					c1+=k; if (c1>0xff) c1 = 0xff;
					c2+=k; if (c2>0xff) c2 = 0xff;
				}
                cym2rgb(&c0,&c1,&c2);
				*cc=Call(t->Display,ColorNum,3(c0,c1,c2));
			} break;
			case 5: *cc = CSIParamNext(e,-1); break;
			}
			Call(t->Display,SetColor,2(c[0],c[1]));
		} break;
		case 50: /* reserved for proportional spacing cancellation */ break;
		case 51: TrmSetAttrFramed(attr,1); break;
		case 52: TrmSetAttrFramed(attr,2); break;
		case 53: TrmSetAttrOverlined(attr,1); break;
		case 52: TrmSetAttrFramed(attr,0); break;
		case 55: TrmSetAttrOverlined(attr,0); break;
		case 56: case 57: case 58: case 59: /* reserved */ break;
		case 60: TrmSetAttrRight(attr,1); break;
		case 61: TrmSetAttrRight(attr,2); break;
		case 62: TrmSetAttrLeft(attr,1); break;
		case 63: TrmSetAttrLeft(attr,2); break;
	    case 64: TrmSetAttrIdeogram(attr,1); break;
		case 65: TrmSetAttrLeft(attr,0); TrmSetAttrRight(attr,0) TrmSetAttrIdeogram(attr,0); break;
		/* 16 colors extension */
		case 90: case 91: case 92: case 93: case 94: case 95: case 96:
		case 97: Call(t->Display,SetColor,2(dec-74,-1)); break;
		case 100: case 101: case 102: case 103: case 104: case 105: case 106:
		case 107: Call(t->Display,SetColor,2(-1,dec-84)); break;
		}
	}
	if (oattr!=attr) Call(t->Display,SetAttribute,1(attr));
}
static void spFNT(theEngine *t,int c,CSIParamList *p) {
	/* FF f2 Font selection setting */
	int idx,num;
	idx = CSIParamNext(p,0);
	num = CSIParamNext(p,0);
	if ((idx>=0)&&(idx<10)) { t->Settings.FontMap[idx] = num; }
}

// GCC and PTX might be used for ideogram composition and/or anotation
// SAPV presents regional presentation variants
static EscapeFn spPTX {/* FF f Parallel texts */}
static EscapeFn	spGCC {/* FF f2 Graphic character combination */}
static EscapeFn	spSAPV{/* FF f2 Select alternative presentation variant */}

static void spSSU(theEngine *t,int c,CSIParamList *e) {
	/* FF f2 Select Size Unit */
	int p,sz;
	p = CSIParamList(e,0);
	sz = 0;
	switch (p) {
	case 0: /*Character*/ break;
	case 1: /* mm */ sz=1000;  break;
	case 2:  sz = (2540/72); break;
	case 3: sz = (10000/266); break;
	case 4: sz = (25400/1000); break;
	case 5: sz = (25400/1200); break;
	case 6: sz = 1; break;
	case 7: sz = 0x200; break;
	case 8: sz = (35000/996); break;
	}
	t->Settings.SizeUnit = sz;
}
static EscapeFn spDTA {/* FF f2 Dimension text area */}

static EscapeFn spTSS {/* FF f2 Thin space specification */}
static EscapeFn spPEC {/* FF f2 Presentation Expand or contract */}
static EscapeFn spSACS{/* FF f2 Set additional character separation */}
static EscapeFn spSRCS{/* FF f2 Set reduced character separation */}
static EscapeFn spSSW {
	/* FF f2 Select Space width (in size units) */
	/* in effect until next (CR/LF),(CR/FF) or (NEL) */
	/* Probably used to locally enable/override proportional spacing. */
}
static EscapeFn spSPI {/* FF f2 Spacing increment */}
static EscapeFn spSHS {/* FF f2 Select horizontal spacing */}
static EscapeFn	spSCS {/* FF f2 Set character spacing */}
static EscapeFn spSVS {/* FF f2 Select vertical spacing */}
static EscapeFn spSLS {/* FF f2 Set line spacing */}
static EscapeFn spGSM {/* FF f2 Graphic size modification */}
static EscapeFn spGSS {/* FF f2 Graphic size selection */}

static EscapeFn spSPD {/* FF f2 Select Presentation direction */}
static EscapeFn	spSCP {/* f2 Select character path */}
static EscapeFn spSRS {/* FF f Start reverse string */ }
static EscapeFn spSDS {/* f Start directed string */ }
static EscapeFn spSCO {/* FF f2 Set Character orientation */}

//---- SIO
static EscapeFn spIGS {/* f2 Identify graphic subrepertoire */}
static EscapeFn spFNK {/* f2 Function key */}

static EscapeFn spPFS {/* FF f2 Page format selection */}
static EscapeFn spSPQR{/* FF f2 Select print quality and rapidity */}
static EscapeFn	spSEF {/* f2 Sheet eject and feed */}

static EscapeFn *C0SpecialChar[] = {
	spNUL, spSOH, spSTX, spETX, spEOT, spENQ, spACK, spBEL,
	spBS,  spHT,  spLF,  spVT,  spFF,  spCR,  spSO,  spSI,
	spDLE, spDC1, spDC2, spDC3, spDC4, spNAK, spSYN, spETB,
	spCAN, spEM,  spSUB, spESC, spIS4, spIS3, spIS2, spIS1
};
static EscapeFn *C1EscapeSeq[] = {
	Rsrvd, Rsrvd, spBPH, spNBH, Rsrvd, spNEL, spSSA, spESA,
	spHTS, spHTJ, spVTS, spPLD, spPLU, spRI,  spSS2, spSS3,
	spDCS, spPU1, spPU2, spSTS, spCCH, spMW,  spSPA, spEPA,
	spSOS, Rsrvd, spSCI, spCSI, spST,  spOSC, spPM,  spAPC
};
static EscapFn *CSISeq[] = {
    spICH, spCUU, spCUD, spCUF, spCUB, spCNL, spCPL, spCHA,
    spCUP, spCHT, spED,  spEL,  spIL,  spDL,  spEF,  spEA,
    spDCH, spSEE, spCPR, spSU,  spSD,  spNP,  spPP,  spCTC,
    spECH, spCVT, spCBT, spSRS, spPTX, spSDS, spSIMD,Rsrvd,
    spHPA, spHPR, spREP, spDA,  spVPA, spVPR, spHVP, spTBC,
    spSM,  spMC,  spHPB, spVPB, spRM,  spSGR, spDSR, spDAQ
};
static EscapFn *CSI20Seq[] = {
    spSL,  spSR,  spGSM, spGSS, spFNT, spTSS, spJFY, spSPI,
    spQUAD,spSSU, spPFS, spSHS, spSVS, spIGS, Rsrvd, spIDCS,
    spPPA, spPPR, spPPB, spSPD, spDTA, spSHL, spSLL, spFNK,
    spSPQR,spSEF, spPEC, spSSW, spSACS,spSAPV,spSTAB,spGCC,
    spTATE,spTALE,spTAC, spTCC, spTSR, spSCO, spSRCS,spSCS,
    spSLS, Rsrvd, spSPL, spSCP, Rsrvd, Rsrvd, Rsrvd, Rsrvd
};
static EscapeFn *EscFn[] = {
	spDMI, spINT, spEMI, spRIS, spCMD, Rsrvd, Rsrvd, Rsrvd,
	Rsrvd, Rsrvd, Rsrvd, Rsrvd, Rsrvd, Rsrvd, spLS2, spLS3,
	Rsrvd, Rsrvd, Rsrvd, Rsrvd, Rsrvd, Rsrvd, Rsrvd, Rsrvd,
	Rsrvd, Rsrvd, Rsrvd, Rsrvd,spLS3R,spLS2R,spLS1R, Rsrvd,
};

