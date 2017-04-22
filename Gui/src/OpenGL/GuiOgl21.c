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
#include <Tools.h>


#include <Gui.local.h>

#include <stdarg.h>
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/freeglut.h>

/*----------------------------*/

#define TheTextureUnit 0
#define ThePaletteUnit 1
#define TheCoordIdx 0
#define TheTextCoordIdx 1
#define BaseRemoteBuffer ((void *)0)

#define MainBufferSize 0x2000

/*----------------------------*/

typedef struct OGLEngine OGLEngine;

typedef struct {
    dList/*<PalFree.n>*/ n;
    int offset;
    unsigned int FreeSlots; // each slot is of 16 colors long.
} PalFree;

typedef struct {
    dList/*<PaletteDesc.n>*/ n;
    OGLEngine *Engine;
    dList/*<PalFree.n>*/ Pal16Free;
    GLuint rData;
    GuiRGBA *Colors;
    unsigned int FreeSlots; // each slot is 256 colors long.
} PaletteDesc;

typedef struct {
    GuiRgbPalette GuiRgbPalette;
    PaletteDesc *page;
    int offset;
} GLPalette;

typedef struct {
    OGLEngine *Engine;
    GLuint Prog;
} OGLPrgm;

typedef struct { 
    struct OGL2dEngine *Static;
} OGL2dEngine;
struct OGLEngine {
    OGL2dEngine OGL2dEngine;
    Render2dEngine Render2dEngine;
    TlsMemPool *ImgPool,*PalettePool,*PalFreePool;
    PaletteDesc Palettes;
    dList/*<OGLImgLib.l>*/ Textures;
    struct {
        Geo2dRectangle Window;
        int bpp,RefRate;
    } ReqOpt,ActOpt;
    Geo2dRectangle Clip;
    struct {
        Geo2dShapeMap Geo2dShapeMap;
            void *b,*p,*e;
    } Mono;
    OGLPrgm *cProgram;
    struct {
        GLfloat v[4];
        unsigned int sgn;
    } Fg,Bg;
    struct {
         GLint PalSmplr,cColorKey,cMskdKey;
    } uniform;
    GLuint MainBuffer;
    struct GreyProgram {
        OGLPrgm Prog;
        int InvScrDimx2,InvMapDim,IdTexture,IdForeground,IdBackground;
        unsigned int sgnFg,sgnBg;
        GLint Texture;
    } FProgram256Grey,FProgram16Grey,FProgram4Grey,FProgramBitmap;
    struct TextProgram {
        OGLPrgm Prog;
        int InvScrDimx2,InvMapDim,IdTexture,IdPalette,IdPaletteBase,IdColorKey;
        GLint Palette,Texture,ColorKey;
        GLuint PaletteBase;
    } FProgram256Pixmap,FProgram16Pixmap,FProgram4Pixmap;
    struct PixProgram {
        OGLPrgm Prog;
        int InvScrDimx2,InvMapDim,IdTexture;
    GLint Texture;
    } FProgramPixmap;
    struct LinColProgram {
        OGLPrgm Prog;
        int InvScrDimx2,InvMapDim,Colors;
    } FProgramLinCol;
    struct MonoProgram {
        OGLPrgm Prog;
        int InvScrDimx2,IdFg;
        unsigned int sgnFg;
    } FProgramMono;
};

struct OGL2dEngine {
    Render2dEngine *(*Open)(OGL2dEngine *this,int w,int h,int bpp);
    void (*Resize)(OGL2dEngine *this,int w,int h);
    void (*Close)(OGL2dEngine *this);
};
typedef struct {
    GuiEngine GuiEngine;
    OGL2dEngine *OGLEngine;
    Render2dEngine *Engine2d;
    int WindowId;
    char *WindowName;
    struct { int w,h,bpp; } ReqOpt;
    struct {
        int AppLast,AppNext;
        int FrameLast,FrameNext;
        int rLast,rNext;
        int FramePerSec;
    } Time;
    int Inited,EvtSet;
    struct { int x,y; } CursPos;
    struct { int x,y,z,btn; } JoyPos;
    int KeyMod;
    struct {
        GuiLibrary *Library;
        int EngineId;
    } Img2dLib;
    GuiInput *Input;
    GuiOutput *Output;
} GlutEngine;

typedef struct {
    Img2dLib Img2dLib;
    dList/*<OGLImgLib.l>*/ l;
    OGLPrgm *Prgm;
    int w,h;
    ImgFormat format;
    GLuint IdTxt;
    GLPalette GLPalette;
} OGLImgLib;

typedef union {
    OGLImgLib ImgLib;
    struct {
        Img2dLib Img2dLib;
        dList l;
        OGLPrgm *Prgm;
        int w,h;
        GuiRGBA colors[4];
    } LinCol;
} OGLAllImg;

typedef struct {
    GLint x,y;
} TexFlatVertex;
typedef struct {
    TexFlatVertex w,t;
} TexTexVertex;

/*___________________________________
 |
 | Palettes
 |___________________________________
*/

static void GLPaletteGetRGBA(GuiRgbPalette *this,GuiRGBA *r,int Color) {
    ThisToThat(GLPalette,GuiRgbPalette);
    *r = that->page->Colors[that->offset+Color];
}
static int GLPaletteGetColor(GuiRgbPalette *this,GuiRGBA *Color) {
    ThisToThat(GLPalette,GuiRgbPalette);
    return 0;
}
static void GLPaletteSetColor(GuiRgbPalette *this,int Color,GuiRGBA *Value) {
    ThisToThat(GLPalette,GuiRgbPalette);
    int offset;
    offset = that->offset+Color;
    glActiveTexture(GL_TEXTURE0+ThePaletteUnit);
    glBindTexture(GL_TEXTURE_2D,that->page->rData);
    that->page->Colors[offset] = *Value;
    glTexSubImage2D(GL_TEXTURE_2D,0,offset&0xff,offset>>8,1,1,GL_RGBA,GL_UNSIGNED_BYTE,Value);
    glBindTexture(GL_TEXTURE_2D,0);
}

/*------------------------------*/

static void eng256PaletteDescInit(PaletteDesc *n) {
    if (n->rData==0) {
        /* Fill palettes with default colors. */{ 
            int i,j;
            GuiRgbPalette *deflt;
            GuiRGBA *p,c;
            p = n->Colors;
            deflt = GuiFixed256Palette();
            for (i=0;i<0x100;i++) {
                Call(deflt,GetRGBA,2(&c,i));
                for (j=0;j<0x2000;j+=0x100) { *(p+j) = c; }
                p++;
            }
        }
        glActiveTexture(GL_TEXTURE0+ThePaletteUnit);
        glGenTextures(1,&n->rData);
        glBindTexture(GL_TEXTURE_2D,n->rData);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D,/*lvl*/0,GL_RGBA8,0x100,0x20,/*brder*/0,GL_RGBA,GL_UNSIGNED_BYTE,(void *)n->Colors);
        glBindTexture(GL_TEXTURE_2D,0);
    }
}

static void engNew256Palette(GLPalette *r,OGLEngine *eng) {
    PaletteDesc *pal;
    dList *p,*e;
    unsigned int msk,msk1,mskshft,min,max,mdl,fre;
    static struct GuiRgbPalette Static = {
        GLPaletteGetRGBA,GLPaletteGetColor,GLPaletteSetColor
    };
    pal = &eng->Palettes;
    p = &eng->Palettes.n;
    e = eng->Palettes.n.p;
    while ((p!=e)&&(!pal->FreeSlots)) {
        p = p->n;
        pal = CastBack(PaletteDesc,n,p);
    }
    if (!pal->FreeSlots) {
        PaletteDesc *n;
        void *data;
        n = Call(pal->Engine->PalettePool,Alloc,0);
        data = n+1; n->Colors = data; 
        n->Engine = pal->Engine;
        n->Pal16Free.n = n->Pal16Free.p = &n->Pal16Free;
        n->FreeSlots = 0xffffffff;
        n->rData = 0;
        n->n.n =  pal->n.n; n->n.p = &pal->n;
        n->n.n->p = n->n.p->n = &n->n;
        eng256PaletteDescInit(n);
        pal = n;
    }
    msk = 0xffff; mskshft=16; min = 0; max = 32; fre = pal->FreeSlots;
    while (mskshft) {
        if (fre&msk) {
            max = (min+max)>>1;
        } else {
            min = (min+max)>>1;
            fre = fre>>mskshft;
        }
        mskshft = mskshft>>1;
        msk = msk>>mskshft;
    }
    if (!(fre&1)) min++;
    pal->FreeSlots = pal->FreeSlots & ~(1<<min);
    r->GuiRgbPalette.Static = &Static;
    r->page = pal;
    r->offset = min<<8;
}
static void engNew16Palette(GLPalette *r,OGLEngine *eng) {
    int found,idx;
    PaletteDesc *pal;
    dList *p,*e;
    PalFree *pg;
    static struct GuiRgbPalette Static = {
        GLPaletteGetRGBA,GLPaletteGetColor,GLPaletteSetColor
    };
    pal = &eng->Palettes;
    e = p = &eng->Palettes.n;
    do {
        found = pal->Pal16Free.n!=(&pal->Pal16Free);
        if (!found) {
            p = p->n;
            pal = CastBack(PaletteDesc,n,p);
        }
    } while ((p!=e)&&(!found));
    if (!found) {
        engNew256Palette(r,eng);
        pal = r->page; idx = r->offset,
        pg = Call(eng->PalFreePool,Alloc,0);
        pg->n.n = pg->n.p = &pg->n;
        pg->offset = idx;
        pg->FreeSlots = 0xffff;
    } else {
        dList *l;
        l = pal->Pal16Free.n;
        l->n->p = l->p; l->p->n = l->n;
        l->n = l->p = l;
        pg = CastBack(PalFree,n,l);
    }
    {   int min,max,msk,shft,fre;
        min = 0; max = 16; msk = 0xff; shft = 4; fre = pg->FreeSlots;
        while (shft) {
            if (fre&msk) {
                max = (min+max)>>1;
            } else {
                min = (min+max)>>1;
                fre = fre>>(shft<<1);
            }
            msk = msk>>shft;
            shft = shft>>1;
        }
        if (!(fre&1)) min++;
        idx = pg->offset+(min<<4);
        pg->FreeSlots = pg->FreeSlots & (~(1<<min));
        if (!pg->FreeSlots) {
            Call(eng->PalFreePool,Free,1(pg));
        } else {
            pg->n.p = &pal->Pal16Free; pg->n.n = pal->Pal16Free.n;
            pg->n.p->n = pg->n.n->p = &pg->n;
        }
    }
    r->GuiRgbPalette.Static = &Static;
    r->offset = idx;
    r->page = pal;
}

static void engRelease256Palette(GLPalette *pal,OGLEngine *eng) {
    PaletteDesc *pg;
    pg = pal->page;
    pg->FreeSlots = pg->FreeSlots|(1<<((pal->offset>>8)&0x1f));
    if ((pg->FreeSlots==0xffffffff)&&(pg!=&eng->Palettes)) {
        dList *l;
        l = &pg->n;
        l->n->p=l->p; l->p->n=l->n; l->n=l->p=l;
        glDeleteTextures(1,&pg->rData);
        pg->rData = 0;
        Call(eng->PalettePool,Free,1(pg));
    }
    pal->page = 0;
}
static void engRelease16Palette(GLPalette *pal,OGLEngine *eng) {
    PaletteDesc *pg;
    PalFree *p16;
    int off,bof,found;
    dList *n,*e;
    pg = pal->page; off = pal->offset; bof = off & (-0x100);
    n = pg->Pal16Free.n; e = &pg->Pal16Free;
    p16 = 0; found = (0!=0);
    while ((n!=e)&&(!found)) { p16 = CastBack(PalFree,n,n); found = (p16->offset==bof); if (!found) {n=n->n;} }
    if (!found) {
        p16 = Call(eng->PalFreePool,Alloc,0);
        p16->offset = bof;
        p16->FreeSlots = 0;
        n=&p16->n; n->n=e; n->p=e->p; n->n->p=n->p->n=n;
    }
    p16->FreeSlots = p16->FreeSlots | (1<<((off>>4)&0xf));
    if (p16->FreeSlots==0xffff) {
        n = &p16->n;
        n->p->n=n->n; n->n->p=n->p; n->n=n->p=n;
        Call(eng->PalFreePool,Free,1(p16));
        pal->offset=bof;
        engRelease256Palette(pal,eng);
    }
    pal->page = 0;
}


/*------------------------------------*/

static void SetTextureImgUniform(GLuint unif,int unit,GLenum Attach,GLuint Id) {
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(Attach,Id);
    glUniform1i(unif,unit);
}
static void TextRectAttrib(OGLEngine *eng,Geo2dRectangle *img) {
    TexTexVertex Dst[4];
    Dst[0].w.x = Dst[2].w.x = eng->Render2dEngine.Cursor.x;
    Dst[1].w.x = Dst[3].w.x = Dst[0].w.x+img->Extent.w;
    Dst[0].w.y = Dst[1].w.y = eng->Render2dEngine.Cursor.y;
    Dst[2].w.y = Dst[3].w.y = Dst[0].w.y+img->Extent.h;
    Dst[0].t.x = Dst[2].t.x = img->Pos.x;
    Dst[1].t.x = Dst[3].t.x = Dst[0].t.x+img->Extent.w;
    Dst[0].t.y = Dst[1].t.y = img->Pos.y;
    Dst[2].t.y = Dst[3].t.y = Dst[0].t.y+img->Extent.h;
    glBindBuffer(GL_ARRAY_BUFFER,eng->MainBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Dst),Dst);
    glEnableVertexAttribArray(TheCoordIdx);
    glVertexAttribPointer(TheCoordIdx,2,GL_INT,(0!=0),
        TlsAtomSize(TexTexVertex),BaseRemoteBuffer+TlsFieldOffset(TexTexVertex,w));
    glEnableVertexAttribArray(TheTextCoordIdx);
    glVertexAttribPointer(TheTextCoordIdx,2,GL_INT,(0!=0),
        TlsAtomSize(TexTexVertex),BaseRemoteBuffer+TlsFieldOffset(TexTexVertex,t));
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    glDisableVertexAttribArray(TheCoordIdx);
    glDisableVertexAttribArray(TheTextCoordIdx);
}

static void ImgLibInfo(Img2dLib *this,int *w,int *h,ImgFormat *format) {
    ThisToThat(OGLImgLib,Img2dLib);
    if (format) *format = that->format;
    if (w) *w = that->w;
    if (h) *h = that->h;
}
static void ImgLibLinColDisplay(Img2dLib *this,Geo2dRectangle *img) {
    OGLEngine *eng;
    struct LinColProgram *prm;
    ThisToThat(OGLAllImg,LinCol.Img2dLib);
    eng = that->LinCol.Prgm->Engine;
    prm = &eng->FProgramLinCol;
    glUseProgram(prm->Prog.Prog); eng->cProgram = &prm->Prog; 
    glUniform2f(prm->InvMapDim,1./that->LinCol.w,1./that->LinCol.h);
    TextRectAttrib(eng,img);
}
static void ImgLibGreyDisplay(Img2dLib *this,Geo2dRectangle *img) {
    struct GreyProgram *prm;
    OGLEngine *eng;
    ThisToThat(OGLImgLib,Img2dLib);
    eng = that->Prgm->Engine;
    prm = CastBack(struct GreyProgram,Prog,that->Prgm);
    if (eng->cProgram!=that->Prgm) { 
        glUseProgram(that->Prgm->Prog); eng->cProgram=that->Prgm;
        prm->Texture = 0;
    }
    if (that->IdTxt!=prm->Texture) {
        SetTextureImgUniform(prm->IdTexture,TheTextureUnit,GL_TEXTURE_2D,that->IdTxt);
        glUniform2f(prm->InvMapDim,1./that->w,1./that->h);
            prm->Texture=that->IdTxt; 
	}
	if (eng->Fg.sgn!=prm->sgnFg) {
            glUniform4f(prm->IdForeground,eng->Fg.v[0],eng->Fg.v[1],eng->Fg.v[2],eng->Fg.v[3]);
            prm->sgnFg = eng->Fg.sgn;
	}
	if (eng->Bg.sgn!=prm->sgnBg) { 
            glUniform4f(prm->IdBackground,eng->Bg.v[0],eng->Bg.v[1],eng->Bg.v[2],eng->Bg.v[3]); 
            prm->sgnBg = eng->Bg.sgn;
	}
    TextRectAttrib(eng,img);
}
static void ImgLibPalettedDisplay(Img2dLib *this,Geo2dRectangle *img) {
    struct TextProgram *prm;
    OGLEngine *eng;
    GLuint pal;
    ThisToThat(OGLImgLib,Img2dLib);
    eng = that->Prgm->Engine;
    prm = CastBack(struct TextProgram,Prog,that->Prgm);
    if (eng->cProgram!=that->Prgm) { 
        glUseProgram(that->Prgm->Prog); eng->cProgram=that->Prgm;
        prm->Texture = 0;
    }
    if (that->IdTxt!=prm->Texture) {
    SetTextureImgUniform(prm->IdTexture,TheTextureUnit,GL_TEXTURE_2D,that->IdTxt);
        glUniform2f(prm->InvMapDim,1./that->w,1./that->h);
        prm->Texture=that->IdTxt; 
    }
    pal = that->GLPalette.page->rData;
    if (pal!=prm->Palette) {
    SetTextureImgUniform(prm->IdPalette,ThePaletteUnit,GL_TEXTURE_2D,pal);
        prm->Palette=pal;
    }
    if (that->GLPalette.offset!=prm->PaletteBase) { 
        glUniform1i(prm->IdPaletteBase,that->GLPalette.offset); 
        prm->PaletteBase = that->GLPalette.offset;
    }
    if (eng->uniform.cColorKey!=prm->ColorKey) {
        glUniform1i(prm->IdColorKey,eng->uniform.cColorKey);
        prm->ColorKey=eng->uniform.cColorKey;
    }
    TextRectAttrib(eng,img);
}
static void ImgLibRGBDisplay(Img2dLib *this,Geo2dRectangle *img) {
    OGLEngine *eng;
    struct PixProgram *prm;
    ThisToThat(OGLImgLib,Img2dLib);
    eng = that->Prgm->Engine;
    prm = &eng->FProgramPixmap;
    if (eng->cProgram!=that->Prgm) { 
        glUseProgram(that->Prgm->Prog); 
        eng->cProgram=that->Prgm;
        prm->Texture = 0;
    }
    if (that->IdTxt!=prm->Texture) { 
        SetTextureImgUniform(prm->IdTexture,TheTextureUnit,GL_TEXTURE_2D,that->IdTxt);
        glUniform2f(prm->InvMapDim,1./that->w,1./that->h);
        prm->Texture=that->IdTxt; 
    }
    TextRectAttrib(eng,img);
}

typedef struct {
    int type,format,internal,mul,div;
    OGLPrgm *Prog;
} OGLImgFormat;
static void GetImgFormat(OGLImgFormat *r,ImgFormat *frm,OGLEngine *eng) {
    r->type = GL_UNSIGNED_BYTE;
    r->format = GL_LUMINANCE;
    r->internal = GL_LUMINANCE8;
    r->mul = r->div = 1;
    if ((frm->Format==C_(Img,Format.Type,Palette))||(frm->Format==C_(Img,Format.Type,Grey))) {
        if (frm->Desc.Palette.nb<=2) {
            frm->Format=C_(Img,Format.Type,Grey);
            r->div=8;
            r->Prog = &eng->FProgramBitmap.Prog;
        } else {
            if (frm->Desc.Palette.nb>0x10) {
                r->Prog = (frm->Format==C_(Img,Format.Type,Palette))?
                    &eng->FProgram256Pixmap.Prog:
                    &eng->FProgram256Grey.Prog;
            } else {
                if (frm->Desc.Palette.nb>0x4) {
                    r->div = 2;
                    r->Prog = (frm->Format==C_(Img,Format.Type,Palette))?
                        &eng->FProgram16Pixmap.Prog:
                        &eng->FProgram16Grey.Prog;
                    } else {
                        r->div = 4;
                        r->Prog = (frm->Format==C_(Img,Format.Type,Palette))?
                            &eng->FProgram4Pixmap.Prog:
                            &eng->FProgram4Grey.Prog;
                    }
                }
            }
        } else {
            r->Prog = &eng->FProgramPixmap.Prog;
            if (frm->Format!=C_(Img,Format.Type,Unknown)) {
                int bits,R,G,B,A;
                R = frm->Desc.RGBA.R.depth+frm->Desc.RGBA.R.offset;
                G = frm->Desc.RGBA.G.depth+frm->Desc.RGBA.G.offset;
                B = frm->Desc.RGBA.B.depth+frm->Desc.RGBA.B.offset;
                bits = (R>B)?R:B; if (bits<G) bits = G;
                if (frm->Format==C_(Img,Format.Type,RGBA)) {
                    r->internal = GL_RGBA8;
                    A = frm->Desc.RGBA.A.depth+frm->Desc.RGBA.A.offset;
                    if (bits<A) bits = A;
                    if (R<B) {
                        r->format = GL_RGBA;
                    } else {
                        r->format = GL_BGRA;
                    }
                    if ((bits==8)||(bits==16)) {
                        if (bits==8) {
                            r->format = GL_RGB;
                            r->type = GL_UNSIGNED_BYTE_3_3_2;
                            r->internal = GL_R3_G3_B2;
                        } else {
                            if (frm->Desc.RGBA.A.depth==4) {
                                r->type = GL_UNSIGNED_SHORT_4_4_4_4;
                                r->internal = GL_RGBA4;
                            } else {
                                r->type = GL_UNSIGNED_SHORT_5_5_5_1;
                                r->internal = GL_RGB5_A1;
                            }
                        }
                    } else {
                        if (bits==32) {
                            if (frm->Desc.RGBA.A.depth==8) {
                                r->type = GL_UNSIGNED_BYTE;
                                r->internal = GL_RGBA8;
                            } else {
                                r->type = GL_UNSIGNED_INT_10_10_10_2;
                                r->internal = GL_RGB10_A2;
                            }
                        }
                    }
                } else {
                    r->internal = GL_RGB8;
                    A = B;
                    if (R<B) {
                        r->format = GL_RGB;
                    } else {
                        r->format = GL_BGR;
                    }
                    if ((bits==8)||(bits==16)) {
                        if (bits==8) {
                            r->type = GL_UNSIGNED_BYTE_3_3_2;
                            r->internal = GL_R3_G3_B2;
                        } else {
                            r->type = GL_UNSIGNED_SHORT_5_6_5;
                            r->internal = GL_RGB5;
                            // r->internal = GL_RGB565;
                        }
                    } else {
                        if (bits==32) {
                            r->type = GL_UNSIGNED_BYTE;
                            r->internal = GL_RGB8;
                        } else {
                            r->type = GL_UNSIGNED_BYTE;
                            r->internal = GL_RGB8;
                        }
                    }
                }
            }
	}
}
static int ImgLibOpen(Img2dLib *this) {
    ThisToThat(OGLImgLib,Img2dLib);

    return (0==0);
}
static void ImgLibClose(Img2dLib *this) { ThisToThat(OGLImgLib,Img2dLib); }
static void ImgLibFree(Img2dLib *this) {
    OGLEngine *eng;
    ThisToThat(OGLImgLib,Img2dLib);
    eng = that->Prgm->Engine;
    glDeleteTextures(1,&that->IdTxt);
    that->l.n->p = that->l.p;
    that->l.p->n = that->l.n;
    if (that->format.Format==C_(Img,Format.Type,Palette)) {
        if (that->format.Desc.Palette.nb>0x10) {
            engRelease256Palette(&that->GLPalette,eng);
        } else {
            engRelease16Palette(&that->GLPalette,eng);
        }
    }
    Call(eng->ImgPool,Free,1(that));
}

/*__________________________________________________
 |
 |
 |__________________________________________________
*/

static Img2dLib *RenderImportImg2dLib(Render2dEngine *this,ImgDesc *Img) {
	static struct Img2dLib Paletted = {
		ImgLibInfo,ImgLibPalettedDisplay,ImgLibOpen,ImgLibClose,ImgLibFree
	};
	static struct Img2dLib RGB = {
		ImgLibInfo,ImgLibRGBDisplay,ImgLibOpen,ImgLibClose,ImgLibFree
	};
	static struct Img2dLib Gray = {
		ImgLibInfo,ImgLibGreyDisplay,ImgLibOpen,ImgLibClose,ImgLibFree
	};
	OGLImgLib *r;
	OGLImgFormat frm;
	ThisToThat(OGLEngine,Render2dEngine);
	r = Call(that->ImgPool,Alloc,0);
	r->format = Img->Format;
    GetImgFormat(&frm,&r->format,that);
	r->w = ((Img->Dim.w*frm.mul)+(frm.div-1))/frm.div; r->h = Img->Dim.h;
	r->Prgm=frm.Prog;

	glActiveTexture(GL_TEXTURE0+TheTextureUnit);
	glGenTextures(1,&r->IdTxt);
	glBindTexture(GL_TEXTURE_2D,r->IdTxt);
	glPixelStorei(GL_UNPACK_ALIGNMENT,this->AlgnImagePitch);
	glTexImage2D(GL_TEXTURE_2D,0,frm.internal,r->w,r->h,/*border*/0,frm.format,frm.type,Img->Data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	if (r->format.Format==C_(Img,Format.Type,Palette)||(r->format.Format==C_(Img,Format.Type,Grey))) {
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D,0);
		if ((r->format.Format==C_(Img,Format.Type,Palette))&&(r->format.Desc.Palette.nb>2)) {
			int i;
		    r->Img2dLib.Static = &Paletted;
			if (r->format.Desc.Palette.nb>0x10) {
                engNew256Palette(&r->GLPalette,that);
			} else {
                engNew16Palette(&r->GLPalette,that);
			}
			r->format.Desc.Palette.Colors = &r->GLPalette.GuiRgbPalette;
			for (i=0;i<r->format.Desc.Palette.nb;i++) {
				GuiRGBA c;
				Call(Img->Format.Desc.Palette.Colors,GetRGBA,2(&c,i));
				Call(r->format.Desc.Palette.Colors,SetColor,2(i,&c));
			}
		} else {
			r->Img2dLib.Static = &Gray;
		}
	} else {
		r->Img2dLib.Static = &RGB;
        glBindTexture(GL_TEXTURE_2D,0);
	}

	r->l.n = &that->Textures; r->l.p = r->l.n->p;
	r->l.n->p = r->l.p->n = &r->l;
	return &r->Img2dLib;
}

static void setFlatShader(OGLEngine *eng){
	if (eng->cProgram!=&eng->FProgramMono.Prog) {
        glUseProgram(eng->FProgramMono.Prog.Prog);
		eng->cProgram = &eng->FProgramMono.Prog;
	}
	if (eng->Fg.sgn!=eng->FProgramMono.sgnFg) {
		glUniform4f(eng->FProgramMono.IdFg,eng->Fg.v[0],eng->Fg.v[1],eng->Fg.v[2],eng->Fg.v[3]);
		eng->FProgramMono.sgnFg = eng->Fg.sgn;
	}
}
static void RenderColorUpdateFront(Render2dEngine *this) {
	ThisToThat(OGLEngine,Render2dEngine);
    that->Fg.v[0] = this->Front.r/255.;
	that->Fg.v[1] = this->Front.g/255.;
	that->Fg.v[2] = this->Front.b/255.;
	that->Fg.v[3] = this->Front.a/255.;
	that->Fg.sgn = (this->Front.r)+(this->Front.g<<8)+(this->Front.b<<16)+(this->Front.a<<24);
}
static void RenderColorUpdateBack(Render2dEngine *this) {
	ThisToThat(OGLEngine,Render2dEngine);
    that->Bg.v[0] = this->Back.r/255.;
	that->Bg.v[1] = this->Back.g/255.;
	that->Bg.v[2] = this->Back.b/255.;
	that->Bg.v[3] = this->Back.a/255.;
	that->Bg.sgn = (this->Back.r)+(this->Back.g<<8)+(this->Back.b<<16)+(this->Back.a<<24);
}
static void RenderColorUpdateKey(Render2dEngine *this) { // Enable/Disable color keying
	ThisToThat(OGLEngine,Render2dEngine);
    if (this->ColorKeyEnabled) {
		that->uniform.cColorKey = that->uniform.cMskdKey;
	} else {
		that->uniform.cColorKey = -1;
	}
}
static void RenderiColorUpdateKey(Render2dEngine *this) { // Select color key
	ThisToThat(OGLEngine,Render2dEngine);
	if (that->uniform.cColorKey==-1) {
		that->uniform.cMskdKey = this->iColorKey;
	} else {
		that->uniform.cMskdKey = that->uniform.cColorKey = this->iColorKey;
	}
}
static void RenderAlphaKeyEnabledUpdate(Render2dEngine *this) {
	ThisToThat(OGLEngine,Render2dEngine);
	if (this->AlphaEnabled) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}
static void RenderUpdateClip(Render2dEngine *this) {
	Geo2dRectangle Clip;
	ThisToThat(OGLEngine,Render2dEngine);
	// be careful here: if the window is resized the clipping might come out wrong.
	that->Clip = this->Clip;
	Geo2dRectangleClip(Clip,this->Clip,that->ActOpt.Window);
	glScissor(Clip.Pos.x,that->ActOpt.Window.Extent.h-(Clip.Pos.y+Clip.Extent.h),Clip.Extent.w,Clip.Extent.h);
}
static void RenderUpdateSetStyle(Render2dEngine *this,int LineWidth,int JoinStyle,int FillType,void *FillPattern) {
	ThisToThat(OGLEngine,Render2dEngine);
	this->LineWidth = LineWidth;
	this->JoinStyle = JoinStyle;
	this->FillType = FillType;
	this->FillPattern = FillPattern;
}
static void RenderPoint(Render2dEngine *this) {
	GLint Coord[2];
	ThisToThat(OGLEngine,Render2dEngine);
	setFlatShader(that);
	Coord[0]=this->Cursor.x; Coord[1]=this->Cursor.y;
	glBindBuffer(GL_ARRAY_BUFFER,that->MainBuffer);
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Coord),Coord);
	glEnableVertexAttribArray(TheCoordIdx);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,(0!=0),0,BaseRemoteBuffer);
	glDrawArrays(GL_POINTS,0,1);
	glDisableVertexAttribArray(TheCoordIdx);
}
static void RenderLine(Render2dEngine *this,int dx,int dy) {
	TexFlatVertex coord[2];
	int h;
	ThisToThat(OGLEngine,Render2dEngine);
	setFlatShader(that);
	coord[0].x = this->Cursor.x; coord[0].y = this->Cursor.y;
	coord[1].x = coord[0].x+dx; coord[1].y = coord[0].y+dy;
	glBindBuffer(GL_ARRAY_BUFFER,that->MainBuffer);
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(coord),coord);
	glEnableVertexAttribArray(TheCoordIdx);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,(0!=0),0,BaseRemoteBuffer);
	glDrawArrays(GL_LINES,0,2);
	glDisableVertexAttribArray(TheCoordIdx);
}
static void RenderShape(Render2dEngine *this,Geo2dShape *shp) {
	GLint *b,*e;
	ThisToThat(OGLEngine,Render2dEngine);
	setFlatShader(that);
	glBindBuffer(GL_ARRAY_BUFFER,that->MainBuffer);
	glEnableVertexAttribArray(TheCoordIdx);
	that->Mono.b = that->Mono.p = glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	that->Mono.e = that->Mono.b+MainBufferSize;
	Call(shp,ForEach,1(&that->Mono.Geo2dShapeMap));
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexAttribPointer(TheCoordIdx,2,GL_INT,(0!=0),0,BaseRemoteBuffer);
	b = that->Mono.b; e = that->Mono.p;
	glDrawArrays(GL_LINES,0,(e-b)>>1);
	glDisableVertexAttribArray(TheCoordIdx);
}
/*
static void RenderCircle(Render2dEngine *this,int r) {
	ThisToThat(OGLEngine,Render2dEngine);
    rOpen {
		Geo2dShape *circ;
		if (this->FillType == C_(Img.Style.Polygon,Filled)) {
		    circ = Geo2dShapeDisk(&this->Cursor,r);
		} else {
		    circ = Geo2dShapeCircle(&this->Cursor,r,this->LineWidth);
		}
        OGLEngineMapShape(that,circ);
	} rClose
}*/
static void RenderBox(Render2dEngine *this,int w,int h) {
    TexFlatVertex coord[4];
    ThisToThat(OGLEngine,Render2dEngine);
    setFlatShader(that);
    coord[0].x = coord[2].x = this->Cursor.x;
    coord[0].y = coord[1].y = this->Cursor.y;
    coord[1].x = coord[3].x = coord[0].x+w;
    coord[2].y = coord[3].y = coord[0].y+h;
    glBindBuffer(GL_ARRAY_BUFFER,that->MainBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(coord),coord);
    glEnableVertexAttribArray(TheCoordIdx);
    glVertexAttribPointer(TheCoordIdx,2,GL_INT,0,(0!=0),BaseRemoteBuffer);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    glDisableVertexAttribArray(TheCoordIdx);
}

static void ClearScreen(Render2dEngine *this) {
    ThisToThat(OGLEngine,Render2dEngine);
    glClearColor(that->Bg.v[0],that->Bg.v[1],that->Bg.v[2],that->Bg.v[3]);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

/*---------------------------*/

static void OGLEngineBoundary(Geo2dShapeMap *this,Geo2dRectangle *R) {
    ThisToThat(OGLEngine,Mono.Geo2dShapeMap);
    Geo2dRectangleClip(*R,that->Clip,that->ActOpt.Window);
    R->Pos.x -= that->Render2dEngine.Cursor.x;
    R->Pos.y -= that->Render2dEngine.Cursor.y;
}
static int OGLEngineMapLine(Geo2dShapeMap *this,int x,int y,int dx) {
    GLint *b,*e;
    ThisToThat(OGLEngine,Mono.Geo2dShapeMap);
    x+=that->Render2dEngine.Cursor.x;
    y+=that->Render2dEngine.Cursor.y;
    b = that->Mono.p; e = b+4;
    if (((void *)e)>that->Mono.e) {
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glVertexAttribPointer(TheCoordIdx,2,GL_INT,0,(0!=0),BaseRemoteBuffer);
        b = that->Mono.b; e = that->Mono.p;
        glDrawArrays(GL_LINES,0,(e-b)>>1);
        b = that->Mono.b = that->Mono.p = glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
        that->Mono.e = that->Mono.b+MainBufferSize;
        e = b+4;
    }
    b[0]=x; b[1]=b[3]=y;
    b[2]=b[0]+dx;
    that->Mono.p = e;
    return (0!=0);
}

/*__________________________________________________
 |
 |
 |__________________________________________________
*/


static GLchar *FileRead(char *name) {
    GLchar *r;
    FILE *f;
    int l;
    r = "";
    f = fopen(name,"r");
    if(f==NULL) {
        printf("Error: Couldn't open file %s\n",name);
    } else {
        fseek(f,0,SEEK_END);
        l = ftell(f);
        rnPush(r,TlsRoundUp(int,l+1));
        fseek(f,0,SEEK_SET);
        fread(r,1,l,f);
        r[l]=0;
        fclose(f);
    }
    return r;
}

typedef struct {
    char *Name;
    GLuint Desc;
} ShaderDesc;

static void newShader(ShaderDesc *r,GLuint stage,char *codefile) {
    GLchar *code[1];
    r->Name = codefile;
    rOpen {
        GLint resul;
        code[0] = FileRead(codefile);
        r->Desc = glCreateShader(stage);
        glShaderSource(r->Desc,1,(const GLchar **)code,NULL);
        glCompileShader(r->Desc);
        glGetShaderiv(r->Desc,GL_COMPILE_STATUS,&resul);
        if (resul==GL_FALSE) {
            printf("Compilation error(s) in shader %s\n",codefile);
            glGetShaderiv(r->Desc,GL_INFO_LOG_LENGTH,&resul);
            rOpen {
                GLchar *log;
                GLsizei lng;
                rnPush(log,resul+1);
                glGetShaderInfoLog(r->Desc,resul,&lng,log);
                log[lng]=0;
                printf(log); printf("\n");
            } rClose
        }
    } rClose
}

static GLuint newProgram(ShaderDesc *Vertex,ShaderDesc *Fragment,char *In,...) {
    GLuint r;
    GLint err;
    char *in;
    va_list l;
    r = glCreateProgram();
    in = In;
    va_start(l,In);
    while (in!=NULL) {
        int idx;
        idx = va_arg(l,int);
        glBindAttribLocation(r,idx,in);
        in = va_arg(l,char *);
    }
    va_end(l);
    glAttachShader(r,Vertex->Desc);
    glAttachShader(r,Fragment->Desc);
    glLinkProgram(r);
    glGetProgramiv(r,GL_LINK_STATUS,&err);
    if (err==GL_TRUE) {
    } else {
        printf("Link Error %s <-> %s\n",Vertex->Name,Fragment->Name);
        glGetProgramiv(r,GL_INFO_LOG_LENGTH,&err);
        rOpen  {
            GLchar *log;
            GLsizei lng;
            rnPush(log,err+1);
            glGetProgramInfoLog(r,err,&lng,log);
            log[lng]=0;
            printf(log); printf("\n");
        } rClose
    }
    glDetachShader(r,Fragment->Desc);
    glDetachShader(r,Vertex->Desc);
    return r;
}
static void newTextProgram(OGLEngine *eng,struct TextProgram *r,ShaderDesc *Vertex,char *FragFile) {
    GLuint p;
    ShaderDesc Fragment;
    newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
    r->Prog.Engine = eng;
    r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
    glDeleteShader(Fragment.Desc);
    glUseProgram(r->Prog.Prog);
    r->InvMapDim = glGetUniformLocation(p,"InvMapDim");
    r->IdTexture = glGetUniformLocation(p,"CurrentPTexture");
    r->Texture = -1;
    r->IdPalette = glGetUniformLocation(p,"CurrentPalette");
    r->Palette = -1;
    r->IdPaletteBase = glGetUniformLocation(p,"CurrentPaletteBase");
    glUniform1i(r->IdPaletteBase,0); r->PaletteBase = 0;
    r->IdColorKey = glGetUniformLocation(p,"ColorKey");
    glUniform1i(r->IdColorKey,-1); r->ColorKey = -1;
    r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
    glUniform2f(r->InvScrDimx2,2./eng->ActOpt.Window.Extent.w,2./eng->ActOpt.Window.Extent.h);
}
static void newMonoProgram(OGLEngine *eng,struct MonoProgram *r,ShaderDesc *Vertex,char *FlatFile) {
    GLuint p;
    ShaderDesc Fragment;
    newShader(&Fragment,GL_FRAGMENT_SHADER,FlatFile);
    r->Prog.Engine = eng;
    r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,NULL);
    glDeleteShader(Fragment.Desc);
    glUseProgram(p);
    r->IdFg = glGetUniformLocation(p,"Foreground");
    glUniform4f(r->IdFg,1.,1.,1.,1.);
    r->sgnFg = 0xffffffff;
    r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
    glUniform2f(r->InvScrDimx2,2./eng->ActOpt.Window.Extent.w,2./eng->ActOpt.Window.Extent.h);
}
static void newLinColProgram(OGLEngine *eng,struct LinColProgram *r,ShaderDesc *Vertex,char *FragFile) {
    GLuint p;
    ShaderDesc Fragment;
    newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
    r->Prog.Engine = eng;
    r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
    glDeleteShader(Fragment.Desc);
    glUseProgram(p);
    r->InvMapDim = glGetUniformLocation(p,"InvMapDim");
    glUniform2f(r->InvMapDim,1./512.,1./512.);
    r->Colors = glGetUniformLocation(p,"Colors");
    {
        GLfloat colors[16] = { 1.,0.,0.,1.,  0.,1.,0.,1.,  0.,0.,1.,1.,  1.,1.,1.,1.  };
        glUniformMatrix4fv(r->Colors,1,GL_FALSE,colors);
    }
    r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
    glUniform2f(r->InvScrDimx2,2./eng->ActOpt.Window.Extent.w,2./eng->ActOpt.Window.Extent.h);
}
static void newPixProgram(OGLEngine *eng,struct PixProgram *r,ShaderDesc *Vertex,char *FragFile) {
    GLuint p;
    ShaderDesc Fragment;
    newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
    r->Prog.Engine = eng;
    r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
    glDeleteShader(Fragment.Desc);
    glUseProgram(p);
    r->InvMapDim = glGetUniformLocation(p,"InvMapDim");
    r->IdTexture = glGetUniformLocation(p,"CurrentPTexture");
    r->Texture = -1;
    r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
    glUniform2f(r->InvScrDimx2,2./eng->ActOpt.Window.Extent.w,2./eng->ActOpt.Window.Extent.h);
}
static void newGreyProgram(OGLEngine *eng,struct GreyProgram *r,ShaderDesc *Vertex,char *FragFile) {
    GLuint p;
    ShaderDesc Fragment;
    newShader(&Fragment,GL_FRAGMENT_SHADER,FragFile);
    r->Prog.Engine = eng;
    r->Prog.Prog = p = newProgram(Vertex,&Fragment,"Coord",(int)TheCoordIdx,"TextCoord",(int)TheTextCoordIdx,NULL);
    glDeleteShader(Fragment.Desc);
    glUseProgram(p);
    r->IdForeground = glGetUniformLocation(p,"Foreground");
    glUniform4f(r->IdForeground,eng->Fg.v[0],eng->Fg.v[1],eng->Fg.v[2],eng->Fg.v[3]);
    r->sgnFg = eng->Fg.sgn;
    r->IdBackground = glGetUniformLocation(p,"Background");
    glUniform4f(r->IdBackground,eng->Bg.v[0],eng->Bg.v[1],eng->Bg.v[2],eng->Bg.v[3]);
    r->sgnBg = eng->Bg.sgn;
    r->InvMapDim = glGetUniformLocation(p,"InvMapDim");
    r->IdTexture = glGetUniformLocation(p,"CurrentPTexture");
    glUniform1i(r->IdTexture,TheTextureUnit);
    r->Texture = -1;
    r->InvScrDimx2 = glGetUniformLocation(p,"InvScrDimx2");
    glUniform2f(r->InvScrDimx2,2./eng->ActOpt.Window.Extent.w,2./eng->ActOpt.Window.Extent.h);
}

/*---------------------------*/

static Render2dEngine *OGL2dOpen(OGL2dEngine *this,int w,int h,int bpp) {
    ThisToThat(OGLEngine,OGL2dEngine);

    that->ActOpt.Window.Extent.w = w;
    that->ActOpt.Window.Extent.h = h;
    that->ActOpt.bpp = bpp;
    {
        GLenum Err;
        const GLubyte *sExtensions;
        GLint val;
        int v0,v1;
        Err = glewInit();
        if (Err!=GLEW_OK) {
            printf("Unable to initialize glew.\n");
            exit(0);
        }
        // sExtensions = glGetString(GL_EXTENSIONS);
        // printf("\n OpenGL Extensions:\n %s \n",sExtensions);
        if (GLEW_VERSION_1_1) { v0=1; v1=1; }
        if (GLEW_VERSION_1_2) { v0=1; v1=2; }
        if (GLEW_VERSION_1_3) { v0=1; v1=3; }
        if (GLEW_VERSION_1_5) { v0=1; v1=5; }
        if (GLEW_VERSION_2_0) { v0=2; v1=0; }
        if (GLEW_VERSION_2_1) { v0=2; v1=1; }
        if (GLEW_VERSION_3_0) { v0=3; v1=0; }
        if (GLEW_VERSION_3_1) { v0=3; v1=1; }
        if (GLEW_VERSION_3_2) { v0=3; v1=2; }
        if (GLEW_VERSION_3_3) { v0=3; v1=3; }
        printf("\nVersion de OpenGL: %d.%d",v0,v1);
        if (GL_ARB_shading_language_100) {
            sExtensions = glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
            if (glGetError()==GL_INVALID_ENUM) { sExtensions = "(probably) 1.0.51"; }
            printf("\n\tShading language version: %s",sExtensions);
        } else {
           printf("\nNo shading language supported");
        }
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&val);
        printf("\n\tMax TextureSize:%d\n",val);
    }
    /**/{
        ShaderDesc p;

        newShader(&p,GL_VERTEX_SHADER,"data/OpenGL21/Vert2dText.glsl"); 

        newLinColProgram(that,&that->FProgramLinCol,&p,"data/OpenGL21/FragLinClr.glsl");
        newGreyProgram(that,&that->FProgram256Grey,&p,"data/OpenGL21/Frag256Grey.glsl");
        newGreyProgram(that,&that->FProgram16Grey,&p,"data/OpenGL21/Frag16Grey.glsl");
        newGreyProgram(that,&that->FProgram4Grey,&p,"data/OpenGL21/Frag4Grey.glsl");
        newGreyProgram(that,&that->FProgramBitmap,&p,"data/OpenGL21/FragBitmap.glsl");
        newTextProgram(that,&that->FProgram256Pixmap,&p,"data/OpenGL21/Frag256Text.glsl");
        newTextProgram(that,&that->FProgram16Pixmap,&p,"data/OpenGL21/Frag16Text.glsl");
        newTextProgram(that,&that->FProgram4Pixmap,&p,"data/OpenGL21/Frag4Text.glsl");
        newPixProgram(that,&that->FProgramPixmap,&p,"data/OpenGL21/FragTCText.glsl");
        glDeleteShader(p.Desc);

        newShader(&p,GL_VERTEX_SHADER,"data/OpenGL21/Vert2dFlat.glsl");
        newMonoProgram(that,&that->FProgramMono,&p,"data/OpenGL21/FragMono.glsl");
        glDeleteShader(p.Desc);

        glGenBuffers(1,&that->MainBuffer);
        glBindBuffer(GL_ARRAY_BUFFER,that->MainBuffer);
        glBufferData(GL_ARRAY_BUFFER,MainBufferSize,(void *)0,GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
                    
        eng256PaletteDescInit(&that->Palettes);

        glEnable(GL_SCISSOR_TEST);

        Gui2dEngineDefaultInit(&that->Render2dEngine,that->ReqOpt.Window.Extent.w,that->ReqOpt.Window.Extent.h);
    }
    return &that->Render2dEngine;
}

static void OGL2dResize(OGL2dEngine *this,int w,int h) {
    ThisToThat(OGLEngine,OGL2dEngine);
    Geo2dRectangle Clip;
    GLfloat inv2w,inv2h;
    inv2w = 2./w; inv2h = 2./h;
    that->ActOpt.Window.Extent.w = w;
    that->ActOpt.Window.Extent.h = h;
    Geo2dRectangleClip(Clip,that->Clip,that->ActOpt.Window);
    glScissor(Clip.Pos.x,h-(Clip.Pos.y+Clip.Extent.h),Clip.Extent.w,Clip.Extent.h);
    glViewport(0,0,(GLsizei)w,(GLsizei)h);
    #define ProgUpdate(prog) \
        glUseProgram(that->prog.Prog.Prog); \
        glUniform2f(that->prog.InvScrDimx2,inv2w,inv2h);
    ProgUpdate(FProgramLinCol);
    ProgUpdate(FProgram256Grey);
    ProgUpdate(FProgram16Grey);
    ProgUpdate(FProgram4Grey);
    ProgUpdate(FProgramBitmap);
    ProgUpdate(FProgram256Pixmap);
    ProgUpdate(FProgram16Pixmap);
    ProgUpdate(FProgram4Pixmap);
    ProgUpdate(FProgramPixmap);
    ProgUpdate(FProgramMono);
    glUseProgram(0);
    #undef ProgUpdate
}

static void OGL2dClose(OGL2dEngine *this) {
    ThisToThat(OGLEngine,OGL2dEngine);
    dList *l;
    l = that->Textures.n;
    while (l!=&that->Textures) {
        OGLImgLib *L;
        L = CastBack(OGLImgLib,l,l);
        Call(&L->Img2dLib,Close,0);
        Call(&L->Img2dLib,Free,0);
        l = that->Textures.n;
    }
    glDeleteTextures(1,&that->Palettes.rData);
    that->Palettes.rData=0;
    glDeleteProgram(that->FProgramLinCol.Prog.Prog);
    glDeleteProgram(that->FProgram256Grey.Prog.Prog);
    glDeleteProgram(that->FProgram16Grey.Prog.Prog);
    glDeleteProgram(that->FProgram4Grey.Prog.Prog);
    glDeleteProgram(that->FProgram256Pixmap.Prog.Prog);
    glDeleteProgram(that->FProgram16Pixmap.Prog.Prog);
    glDeleteProgram(that->FProgram4Pixmap.Prog.Prog);
    glDeleteProgram(that->FProgramBitmap.Prog.Prog);
    glDeleteProgram(that->FProgramPixmap.Prog.Prog);
    glDeleteProgram(that->FProgramMono.Prog.Prog);
    glDeleteBuffers(1,&that->MainBuffer);
}

OGL2dEngine *OGL2dEngineNew(int w,int h,int bpp) {
    static struct OGL2dEngine RenderCtrlStatic = {
        OGL2dOpen,OGL2dResize,OGL2dClose
    };
    static struct Render2dEngine RenderStatic = {
        RenderImportImg2dLib,RenderColorUpdateFront,RenderColorUpdateBack,
        RenderColorUpdateKey,RenderiColorUpdateKey,RenderAlphaKeyEnabledUpdate,
        RenderUpdateClip,RenderUpdateSetStyle,RenderPoint,RenderLine,RenderShape,
        RenderBox,ClearScreen
    };
    static struct Geo2dShapeMap ShapeMapStatic = {OGLEngineBoundary,OGLEngineMapLine};
    OGLEngine *r;
    TlsMemPool *pool;
    GuiRGBA *Palettes;
    rPush(r);
    r->OGL2dEngine.Static = &RenderCtrlStatic;
    r->Render2dEngine.Static = &RenderStatic;
    r->Mono.Geo2dShapeMap.Static = &ShapeMapStatic;
    rnPush(Palettes,0x100*0x20);
    r->Mono.b = r->Mono.p = r->Mono.e = 0;
    pool = TlsMemPoolNew(2048);
    r->ImgPool = Call(pool,SizeSelect,1(TlsAtomSize(OGLImgLib)));
    r->PalettePool = Call(pool,SizeSelect,1(TlsAtomSize(PaletteDesc)+0x8000));
    r->PalFreePool = Call(pool,SizeSelect,1(TlsAtomSize(PalFree)));
    r->ReqOpt.Window.Pos.x = r->ReqOpt.Window.Pos.y = 0;
    r->ReqOpt.Window.Extent.w = w; r->ReqOpt.Window.Extent.h = h;
    r->ReqOpt.bpp = bpp; r->ReqOpt.RefRate = 60;
    r->ActOpt = r->ReqOpt;
    r->Clip = r->ReqOpt.Window;
    r->cProgram = 0;
    r->Palettes.n.n = r->Palettes.n.p = &r->Palettes.n;
    r->Palettes.Engine = r;
    r->Palettes.Pal16Free.n = r->Palettes.Pal16Free.p = &r->Palettes.Pal16Free;
    r->Palettes.rData = 0;
    r->Palettes.Colors = Palettes;
    r->Palettes.FreeSlots = 0xffffffff;
#define colset(c,r,g,b,a) { \
    c.v[0]=r/255.;\
    c.v[1]=g/255.;\
    c.v[2]=b/255.;\
    c.v[3]=a/255.;\
    c.sgn=r+(g<<8)+(b<<16)+(a<<24);\
}
    colset(r->Fg,255,255,255,255);
    colset(r->Bg,0,0,0,255);
#undef colset
    r->Textures.n = r->Textures.p = &r->Textures;
    r->Render2dEngine.ScreenOrgAtBottom = (0!=0);
    r->Render2dEngine.MaxImagePitch = 0;
    r->Render2dEngine.AlgnImagePitch = 1;
    return &r->OGL2dEngine;
}

/*------------------*/

static GlutEngine *eThis = 0;

static int checkEnd(int e) {
    int r;
    r = (e==C_(Gui,Event,Finished));
    if (r) {
        glutLeaveMainLoop();
    }
    return r;
}
static int checkMouse(int x,int y) {
    int r;
    if ((x!=eThis->CursPos.x)||(y!=eThis->CursPos.y)) {
        eThis->CursPos.x = x;
        eThis->CursPos.y = y;
        r = Call(eThis->Input,Cursor,3(InputDevice_Mouse|0,x,y));
    } else {
        r = C_(Gui,Event,Ignored);
    }
    return r;
}
#include <GuiKeyMap.h>
static int checkKeyMod(void) {
    int r,mod,dMod,end;
    mod = glutGetModifiers();
    if ((mod!=eThis->KeyMod)) {
        dMod = eThis->KeyMod ^ mod;
        end = (0!=0);
        if (dMod&GLUT_ACTIVE_SHIFT) {
            if (mod&GLUT_ACTIVE_SHIFT) {
                r=Call(eThis->Input,KeyDown,2(
                    InputDevice_Keyboard|0,GuiKey(Alt,ShiftLeft)));
            } else {
                r=Call(eThis->Input,KeyUp,2(
                    InputDevice_Keyboard|0,GuiKey(Alt,ShiftLeft)));
            }
            end = (r==C_(Gui,Event,Finished));
        }
        if ((!end)&&(dMod&GLUT_ACTIVE_CTRL)) {
            if (mod&GLUT_ACTIVE_CTRL) {
                r=Call(eThis->Input,KeyDown,2(
                    InputDevice_Keyboard|0,GuiKey(Alt,CtrlLeft)));
            } else {
                r=Call(eThis->Input,KeyUp,2(InputDevice_Keyboard|0,GuiKey(Alt,CtrlLeft)));
            }
            end = (r==C_(Gui,Event,Finished));
        }
        if ((!end)&&(dMod&GLUT_ACTIVE_ALT)) {
            if (mod&GLUT_ACTIVE_ALT) {
                r=Call(eThis->Input,KeyDown,2(
                   InputDevice_Keyboard|0,GuiKey(Alt,AltLeft)));
            } else {
                r=Call(eThis->Input,KeyUp,2(InputDevice_Keyboard|0,GuiKey(Alt,AltLeft)));
            }
            end = (r==C_(Gui,Event,Finished));
        }
        eThis->KeyMod = mod;
    } else {
        r = C_(Gui,Event,Ignored);
    }
    return r;
}
static void ManageDisplay(void) {
    Call(eThis->Output,Display,1(eThis->Engine2d));
    glutSwapBuffers();
}
static void ManageResize(int w,int h) {
    Call(eThis->OGLEngine,Resize,2(w,h));
}
static void ManageJoystick(unsigned int buttons,int xax,int yax,int zax) {
    int end;
    end = (0!=0);
    if (eThis->JoyPos.x!=xax) {
        end = checkEnd(Call(eThis->Input,Cursor,3(InputDevice_JoypadAxis|(0<<8)|0,xax,0)));
        eThis->JoyPos.x = xax;
    }
    if ((!end)&&(eThis->JoyPos.y!=yax)) {
        end = checkEnd(Call(eThis->Input,Cursor,3(InputDevice_JoypadAxis|(1<<8)|0,yax,0)));
        eThis->JoyPos.y = yax;
    }
    if ((!end)&&(eThis->JoyPos.z!=zax)) {
        end = checkEnd(Call(eThis->Input,Cursor,3(InputDevice_JoypadAxis|(2<<8)|0,zax,0)));
        eThis->JoyPos.z = zax;
    }
    if ((!end)&&(eThis->JoyPos.btn!=buttons)) {
        unsigned int n,msk;
        n = 0; msk = (eThis->JoyPos.btn)^buttons;
        while ((!end)&&(msk)) {
            if (msk&1) {
                if ((buttons>>n)&1) {
                    end = checkEnd(Call(eThis->Input,KeyUp,2(InputDevice_Joypad|0,n)));
                } else {
                    end = checkEnd(Call(eThis->Input,KeyDown,2(InputDevice_Joypad|0,n)));
                }
            }
            n++; msk = msk>>1;
        }
        eThis->JoyPos.btn = buttons;
    }
}
#include <GuiKeyMap.h>
static int fKeyTrans[128];
static int KeyTrans[256];
static void KeyInit(void) {
    int *e,*q,*q1,i;
    i = 0;
    q = fKeyTrans;
    q1 = KeyTrans;
    e = q+128;
    while (q<e) {
        *q = *q1 = GuiKeyTypAscii|i<<8|i;
        q++; q1++; i++;
    }
    q = q1; e = q1+128;
    while (q<e) {
        *q = GuiKeyTypAscii|i<<8|i;
        q++; i++;
    }
    fKeyTrans[GLUT_KEY_F1] = GuiKey(Fn,F1);
    fKeyTrans[GLUT_KEY_F2] = GuiKey(Fn,F2);
    fKeyTrans[GLUT_KEY_F3] = GuiKey(Fn,F3);
    fKeyTrans[GLUT_KEY_F4] = GuiKey(Fn,F4);
    fKeyTrans[GLUT_KEY_F5] = GuiKey(Fn,F5);
    fKeyTrans[GLUT_KEY_F6] = GuiKey(Fn,F6);
    fKeyTrans[GLUT_KEY_F7] = GuiKey(Fn,F7);
    fKeyTrans[GLUT_KEY_F8] = GuiKey(Fn,F8);
    fKeyTrans[GLUT_KEY_F9] = GuiKey(Fn,F9);
    fKeyTrans[GLUT_KEY_F10] = GuiKey(Fn,F10);
    fKeyTrans[GLUT_KEY_F11] = GuiKey(Fn,F11);
    fKeyTrans[GLUT_KEY_F12] = GuiKey(Fn,F12);
    fKeyTrans[GLUT_KEY_PAGE_UP] = GuiKey(Fn,PgUp);
    fKeyTrans[GLUT_KEY_PAGE_DOWN] = GuiKey(Fn,PgDown);
    fKeyTrans[GLUT_KEY_HOME]  = GuiKey(Fn,Begin);
    fKeyTrans[GLUT_KEY_END] = GuiKey(Fn,End);
    fKeyTrans[GLUT_KEY_LEFT] = GuiKey(Fn,Left);
    fKeyTrans[GLUT_KEY_RIGHT] = GuiKey(Fn,Right);
    fKeyTrans[GLUT_KEY_UP] = GuiKey(Fn,Up);
    fKeyTrans[GLUT_KEY_DOWN] = GuiKey(Fn,Down);
    fKeyTrans[GLUT_KEY_INSERT] = GuiKey(Fn,Insert);
}
static void ManageKeyboard(unsigned char key,int x,int y) {
    if (!checkEnd(checkMouse(x,y))){
        if (!checkEnd(checkKeyMod())) {
            checkEnd(Call(eThis->Input,KeyDown,2(
               InputDevice_Keyboard|0,KeyTrans[key&0xff])));
        }
    }
}
static void ManageKeyboardUp(unsigned char key,int x,int y) {
    if (!checkEnd(checkMouse(x,y))) {
        if (!checkEnd(checkKeyMod())) {
            checkEnd(Call(eThis->Input,KeyUp,2(
                InputDevice_Keyboard|0,KeyTrans[key&0xff])));
        }
    }
}
static void fManageKeyboard(int key,int x,int y) {
    if (!checkEnd(checkMouse(x,y))){
        if (!checkEnd(checkKeyMod())) {
            checkEnd(Call(eThis->Input,KeyDown,2(
                InputDevice_Keyboard|0,fKeyTrans[key&0x7f])));
        }
    }
}
static void fManageKeyboardUp(int key,int x,int y) {
    if (!checkEnd(checkMouse(x,y))) {
        if (!checkEnd(checkKeyMod())) {
            checkEnd(Call(eThis->Input,KeyUp,2(
                InputDevice_Keyboard|0,fKeyTrans[key&0x7f])));
        }
    }
}
static void ManageMouseKey(int button,int state,int x,int y) {
    if (!checkEnd(checkMouse(x,y))) {
        int key;
        key = 2;
        if (button==GLUT_LEFT_BUTTON) { key=0; }
        if (button==GLUT_RIGHT_BUTTON) { key=1; }
        if (state==GLUT_DOWN) {
            checkEnd(Call(eThis->Input,KeyDown,2(InputDevice_Mouse|0,key)));
        } else {
            checkEnd(Call(eThis->Input,KeyUp,2(InputDevice_Mouse|0,key)));
        }
    }
}
static void ManageMouseMotion(int x,int y) {
    checkEnd(checkMouse(x,y));
}
#include <GuiTime.h>
#define WrapAround (GuiTimSWrapAround*1000)
static void ManageIdle(void) {
    int t,tl,tn,dt;
    if (eThis->Time.FrameLast>=eThis->Time.FrameNext) {
        int period,fl,fn;
        fn = eThis->Time.FrameNext;
        fl = eThis->Time.FrameLast;
        period = eThis->Time.FramePerSec*1000; // 1s
        Call(eThis->Output,Display,1(eThis->Engine2d));
        glutSwapBuffers();
        fn += 1000*(1+((fl-fn)/1000));
        if (fn>period) { int ofn; ofn = fn; fn=fn%period; fl=fl-(ofn-fn);}
        eThis->Time.FrameNext = fn;
        eThis->Time.FrameLast = fl;
    }
    tl = eThis->Time.rLast; tn = eThis->Time.rNext;
    t = (GuiTimeGetTime()/1000)%WrapAround;
    if (t<tl) (tl-=WrapAround);
    dt = t-tl;
    eThis->Time.AppLast += dt;
    eThis->Time.FrameLast += dt*eThis->Time.FramePerSec;
    eThis->Time.rLast = t;
    if ((eThis->Time.AppLast-eThis->Time.AppNext)>=0) {
        int nxt;
        checkEnd(Call(eThis->Input,msTime,2(&nxt,eThis->Time.AppLast)));
        if ((nxt-eThis->Time.AppLast)>0) {
            eThis->Time.AppNext = nxt;
        } else {
            eThis->Time.AppNext = eThis->Time.AppLast+30;
        }
    }
}

static int EngineOpen(GuiEngine *this) {
    int r;
    ThisToThat(GlutEngine,GuiEngine);
    r = !that->Inited;
    if (r) {
        int w,h,bpp;
        glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
        glutInitWindowSize(that->ReqOpt.w,that->ReqOpt.h);
        glutInitWindowPosition(0,0);
        that->WindowId = glutCreateWindow(that->WindowName);
        glutDisplayFunc(ManageDisplay);
        glutReshapeFunc(ManageResize);
        glutKeyboardFunc(ManageKeyboard);
        glutKeyboardUpFunc(ManageKeyboardUp);
        glutSpecialFunc(fManageKeyboard);
        glutSpecialUpFunc(fManageKeyboardUp);
        glutJoystickFunc(ManageJoystick,25);
        glutMouseFunc(ManageMouseKey);
        glutMotionFunc(ManageMouseMotion);
        glutPassiveMotionFunc(ManageMouseMotion);
        glutIdleFunc(ManageIdle);

        w = glutGet(GLUT_WINDOW_WIDTH);
        h = glutGet(GLUT_WINDOW_HEIGHT);
        bpp = glutGet(GLUT_WINDOW_RED_SIZE)+glutGet(GLUT_WINDOW_GREEN_SIZE)+
        glutGet(GLUT_WINDOW_BLUE_SIZE)+glutGet(GLUT_WINDOW_ALPHA_SIZE);

        that->Engine2d = Call(that->OGLEngine,Open,3(w,h,bpp));
        that->Inited = (0==0);
        if (that->Img2dLib.EngineId==-1)  {
            that->Img2dLib.EngineId=Call(that->Img2dLib.Library,RecordEngine,1(
               that->Engine2d));
        }
    }
    return r;
}
static void EngineEventLoop(GuiEngine *this,GuiOutput *Output,GuiInput *Input) {
    ThisToThat(GlutEngine,GuiEngine);
    if (that->Inited) {
        eThis = that;
        that->Time.AppLast = that->Time.AppNext = 0;
        that->Time.FrameLast = that->Time.FrameNext = 0;
        that->Time.rLast = that->Time.rNext = (GuiTimeGetTime()/1000)%WrapAround;
        that->Input = Input;
        that->Output = Output;

        /* {
            int t0,t1,fps;
            Call(that->Engine2d,ScreenClear,0);
            glutSwapBuffers();
            t0 = GuiTimeGetTime();
            Call(that->Engine2d,ScreenClear,0);
            glutSwapBuffers();
            t1 = GuiTimeGetTime();
            // this only work when glutSwapBuffers waits for vert sync
            fps = 1000000/(t1-t0);
            // printf("Measured fps: %d\n",fps);
            if ((fps>=20)&&(fps<=75)) {
                that->Time.FramePerSec = fps;
            }
        }*/

        glutMainLoop();

        that->Input = 0;
        that->Output = 0;
    }
}
static void EngineClose(GuiEngine *this) {
    ThisToThat(GlutEngine,GuiEngine);
    if (that->Inited) {
        if (that->Img2dLib.EngineId != -1) {
            Call(that->Img2dLib.Library,ReleaseEngine,1(that->Img2dLib.EngineId));
            that->Img2dLib.EngineId = -1;
        }
        Call(that->OGLEngine,Close,0);
        that->Engine2d = 0;
        glutDestroyWindow(that->WindowId);
        that->Inited = (0!=0);
    }
}

/*----------------------------------*/

GuiEngine *OGLGuiEngineNew(char *WindowName,GuiLibrary *Library,int w,int h,int *FrameRate) {
    GlutEngine *r;
    static struct GuiEngine EngineStatic = {EngineOpen,EngineClose,EngineEventLoop};
    static first = (0==0);
    if (first) {
        int Argc;
        char *Argv[1];
        Argv[0] = WindowName; Argc = 1;
        glutInit(&Argc,Argv);
        first = (0!=0);
    }
    rPush(r);
    r->GuiEngine.Static = &EngineStatic;
    r->Inited = (0!=0);
    r->Img2dLib.EngineId = -1;
    r->Img2dLib.Library = Library;
    r->WindowName = WindowName;
    r->ReqOpt.w = w;
    r->ReqOpt.h = h;
    r->ReqOpt.bpp = 0x20;
    r->CursPos.x = r->CursPos.y = 0;
    r->JoyPos.x = r->JoyPos.y = r->JoyPos.z = r->JoyPos.btn = 0;
    r->KeyMod = 0;
    r->Input = &GuiInputNull;
    r->Output = &GuiOutputNull;
    r->EvtSet = (0!=0);
    KeyInit();
    r->Time.FramePerSec = *FrameRate;
    r->OGLEngine = OGL2dEngineNew(w,h,0x20);
    r->Engine2d = 0;
    return &r->GuiEngine;
}

