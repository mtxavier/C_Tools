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
#include <Geo.h>


/*-----------------------------------*/

static void shapeMapNullBoundary(Geo2dShapeMap *this,Geo2dRectangle *R) {
	R->Pos.x = R->Pos.y = 0; R->Extent.w = R->Extent.h = 0;
}
static int shapeMapNullMapLine(Geo2dShapeMap *this,int x,int y,int dx) {return (0==0);}
static struct Geo2dShapeMap shapeMapNullStatic = {
	shapeMapNullBoundary, shapeMapNullMapLine
};
Geo2dShapeMap Geo2dShapeMapNull = {&shapeMapNullStatic};

/*-----------------------------------*/

typedef struct {
	Geo2dShape Geo2dShape;
	Geo2dShapeMap Geo2dShapeMap;
	Geo2dShape *Shape;
	Geo2dShapeMap *Map;
	Geo2dRectangle Bound;
	int xMax,yMax;
} safeShape;

static void safeBoundary(Geo2dShapeMap *this,Geo2dRectangle *R) {
    ThisToThat(safeShape,Geo2dShapeMap);
	*R = that->Bound;
}
static int safeMapLine(Geo2dShapeMap *this,int x,int y,int dx) {
	int r;
	ThisToThat(safeShape,Geo2dShapeMap);
	r = (0!=0);
	if (x>=that->Bound.Pos.x && x<that->xMax && y>=that->Bound.Pos.y && y<that->yMax) {
		if (x+dx>that->xMax) (dx = that->xMax-x);
		r = Call(that->Map,MapLine,3(x,y,dx));
	}
	return r;
}
static void safeForEach(Geo2dShape *this,Geo2dShapeMap *Dst) {
	ThisToThat(safeShape,Geo2dShape);
	that->Map = Dst;
	Call(Dst,Boundary,1(&that->Bound));
    that->xMax = that->Bound.Pos.x + that->Bound.Extent.w;
	that->yMax = that->Bound.Pos.y + that->Bound.Extent.h;
	Call(that->Shape,ForEach,1(&that->Geo2dShapeMap));
}	

Geo2dShape *Geo2dSafeShape(Geo2dShape *Child) {
	safeShape *R;
	static struct Geo2dShapeMap Map = {safeBoundary,safeMapLine};
	static struct Geo2dShape Shape = {safeForEach};
	rPush(R);
	R->Geo2dShape.Static = &Shape;
	R->Geo2dShapeMap.Static = &Map;
	R->Shape = Child;
	R->Map = &Geo2dShapeMapNull;
	R->Bound.Pos.x = R->Bound.Pos.y = 0;
	R->Bound.Extent.w = R->Bound.Extent.h = 0;
	R->yMax = R->xMax = 0;
	return &R->Geo2dShape;
}



/*-----------------------------------*/

static void shapeNullForEach(Geo2dShape *this,Geo2dShapeMap *Dst) {}
static struct Geo2dShape Geo2dShapeNullStatic = { shapeNullForEach };
Geo2dShape Geo2dShapeNull = {&Geo2dShapeNullStatic};

/*-----------------------------------*/

static int Sqrt(float x) {
	int r;
	r = 0;
	if (x>0.) {
		float a,b,d;
		a = 2.; b = x/2.;
		d = a-b;
		while ((d<-1.)||(d>1.)) {
			a = (a+b)/2.;
			b = x/a;
			d = a-b;
		}
		r = (a<b)? a:b;
		a = r+1; a = a*a;
		if (a<=x) r++;
	}
	return r;
}

/*-----------------------------------*/

typedef struct {
	Geo2dShape Geo2dShape;
	Geo2dPoint O;
	int r,w;
} shapeCircle;

static void shapeDiskForEach(Geo2dShape *this,Geo2dShapeMap *Dst) {
	Geo2dRectangle bound;
	int end;
	int ymin,ymax,xmin,xmax,r,ox,oy;
	ThisToThat(shapeCircle,Geo2dShape);
	ox = that->O.x; oy = that->O.y;
	r = that->r;
	ymin = oy-r; ymax = oy+r;
	xmin = ox-r; xmax = ox+r;
	Call(Dst,Boundary,1(&bound));
	if (ymin<bound.Pos.y) ymin = bound.Pos.y;
	if (ymax>bound.Pos.y+bound.Extent.h) ymax = bound.Pos.y+bound.Extent.h;
	if (xmin<bound.Pos.x) xmin = bound.Pos.x;
	if (xmax>bound.Pos.x+bound.Extent.w) xmax = bound.Pos.x+bound.Extent.w;
	if ((xmin<=xmax) && (ymin<=ymax)) {
		float r2,y2,x2;
		int x,y,ye,dx,dy,dd,d2;
		r2 = that->r; 
		r2=r2*r2;
		ymin -= oy; ymax -= oy;
		xmin -= ox; xmax -= ox;
		y = ymin; y2 = y; y2+=.5; y2 = y2*y2;
		dy = 2*y+2;
	    d2 = r2-y2;	
		x = Sqrt(d2); x2=x+.5; x2 = x2*x2; 
		dx = 2*x+2;
		d2 = d2-x2;
		if (d2<0) { x--; dx-=2; x2-=dx; d2+=dx; }
		while (y<ymax) {
			if (y<-1) {dd=+2; ye=(ymax>-1)?-1:ymax;} else {dd=-2; ye=ymax;}
			while (y<ye) {
				int xl,xh;
			    xl = (-x<xmin)?xmin:-x;
			    xh = (x>xmax)? xmax:x;
			    Call(Dst,MapLine,3(xl+ox,y+oy,xh-xl));
				d2 -= dy; dy+= dd;
				while ((d2-dx)>=0) {
					d2-=dx; dx+=dd;
				}
				x = (dx>>1);
			    y++;
			}
		}
	}
}

Geo2dShape *Geo2dShapeDisk(Geo2dPoint *O,int r) {
	shapeCircle *R;
	static struct Geo2dShape Static = {shapeDiskForEach};
	rPush(R);
	R->Geo2dShape.Static = &Static;
	R->r = r;
	R->O = *O;
	return &R->Geo2dShape;
}

/*----------------------------------*/

static void shapeCircleForEach(Geo2dShape *this,Geo2dShapeMap *Dst) {
	Geo2dRectangle bound;
	int end;
	int ymin,ymax,xmin,xmax,r,ox,oy;
	int yimin,yimax,ir,iclip;
	ThisToThat(shapeCircle,Geo2dShape);
	ox = that->O.x; oy = that->O.y;
	r = that->r;
	ir = r-that->w;
	ymin = oy-r; ymax = oy+r;
	xmin = ox-r; xmax = ox+r;
	yimin = oy-ir; yimax = oy+ir;
	Call(Dst,Boundary,1(&bound));
	if (ymin<bound.Pos.y) ymin = bound.Pos.y;
	if (ymax>bound.Pos.y+bound.Extent.h) ymax = bound.Pos.y+bound.Extent.h;
	if (xmin<bound.Pos.x) xmin = bound.Pos.x;
	if (xmax>bound.Pos.x+bound.Extent.w) xmax = bound.Pos.x+bound.Extent.w;
    { int ximin,ximax;
		float r2,x2,y2;
		ximin = ox-ir; ximax = ox+ir;
		r2 = ir; r2 = r2*r2;
        #define InCircle(x,y) x2 = (x)-ox; x2 = x2*x2; y2 = (y)-oy; y2=y*y; iclip = (x2+y2)<=r2
	    InCircle(bound.Pos.x,bound.Pos.y);
		if (iclip) InCircle(bound.Pos.x+bound.Extent.w,bound.Pos.y);
		if (iclip) InCircle(bound.Pos.x,bound.Pos.y+bound.Extent.h);
		if (iclip) InCircle(bound.Pos.x+bound.Extent.w,bound.Pos.y+bound.Extent.h);
        #undef InCircle
	}
	if (yimin<bound.Pos.y) yimin = bound.Pos.y;
	if (yimax>bound.Pos.y+bound.Extent.h) yimax = bound.Pos.y+bound.Extent.h;
	if ((xmin<=xmax) && (ymin<=ymax) && (!iclip)) {
		float r2,y2,x2;
		int x,y,ye,dx,dy,dd,d2;
		int ix,dix,diy,di2;

		xmin -= ox; xmax -= ox;
		ymin -= oy; ymax -= oy;
		yimin -= oy; yimax -= oy;

        r2 = ir; r2=r2*r2;
		y2 = yimin+.5; y2 = y2*y2; diy = 2*yimin+2;
		ix = Sqrt(r2-y2); x2=ix+.5; x2 = x2*x2; dix = 2*ix+2;
		di2 = r2-(x2+y2); if (di2<0) { ix--; dix-=2; di2+=dix; }

		r2 = r; r2=r2*r2;
		y = ymin; y2 = ymin+.5; y2 = y2*y2; dy = 2*ymin+2;
		x = Sqrt(r2-y2); x2=x+.5; x2 = x2*x2; dx = 2*x+2;
		d2 = r2-(x2+y2); if (d2<0) { x--; dx-=2; d2+=dx; }

		while (y<ymax) {
			if (y<-1) { ye=(ymax>-1)?-1:ymax; dd = +2; } else { ye=ymax; dd = -2; }
			while (y<ye) {
				int xl,xh;
			    xl = (-x<xmin)?xmin:-x;
			    xh = (x>xmax)? xmax:x;
				if ((y>=yimin)&&(y<yimax)) {
					int ixl,ixh;
					ixl = (-ix<xmin)?xmin:-ix;
					ixh = (ix>xmax)?xmax:ix;
					Call(Dst,MapLine,3(xl+ox,y+oy,ixl-xl));
					Call(Dst,MapLine,3(ixh+ox,y+oy,xh-ixh));
					di2 -= diy; diy+=dd;
					while ((di2-dix)>=0) { di2-=dix; dix+=dd; }
					ix = (dix>>1);
				} else {
			        Call(Dst,MapLine,3(xl+ox,y+oy,xh-xl));
				}
				d2 -= dy; dy+=dd;
				while ((d2-dx)>=0) { d2-=dx; dx+=dd; }
				x = (dx>>1);
			    y++;
			}
		}
	}
}

Geo2dShape *Geo2dShapeCircle(Geo2dPoint *O,int r,int w) {
	shapeCircle *R;
	static struct Geo2dShape Static = {shapeCircleForEach};
	rPush(R);
	R->Geo2dShape.Static = &Static;
	if (w<1) w=1;
	R->w = w;
	R->r = r+(w>>1);
	R->O = *O;
	return &R->Geo2dShape;
}

/*----------------------------------*/

typedef struct {
	int x[2];
} point2d;
typedef struct {
	point2d O[2];
} segment2d;

typedef struct {
	Geo2dShape Geo2dShape;
	segment2d s;
	int Width;
} shapeLine;

typedef struct {
	point2d o;
	struct affine {  // affine coefficient: Y = a*X+(o/rt), with a = d+dr/rt;
	    int sgn;
		unsigned int Delta;
		unsigned int d,o,dr,rt;
	} d[2];
	int cx,cy;
	int l;
} segDesc;
typedef struct {
	int bx,ex,y,r,cbx,cex;
} segPtr;

#define point2dSubstract(R,v) {\
	(R).x[0] -= (v).x[0]; \
	(R).x[1] -= (v).x[1]; \
}

// affine : y = x*a = x * (d+(dr/rt));
static void affineGetYRange(segPtr *R,struct affine *a,int x) {
	int y,ry,r;
	long long rest;
	R->y = x;
    y = x*a->d;
	rest = a->o + x*a->dr;
	ry = rest/a->rt;
	r = rest-(ry*a->rt);
	R->r = r;
	R->cbx = R->bx = y+ry;
	R->cex = R->ex = R->bx+a->d;
	if ((r+a->dr)>a->rt) R->ex++;
}

static void SegDescInit(segDesc *l,segPtr *p,segment2d *box,segment2d *s) {
	int i,x,y;
	point2d O;
	O = s->O[0];
	l->o = O;
	point2dSubstract(s->O[0],O);
	point2dSubstract(s->O[1],O);
	point2dSubstract(box->O[0],O);
	point2dSubstract(box->O[1],O);
	for (i=0;i<2;i++) {
		l->d[i].d = 1;
		if (s->O[1].x[i]>=0) {
			l->d[i].sgn = 1;
		} else {
			l->d[i].sgn = -1;
			s->O[1].x[i] = -s->O[1].x[i];
			box->O[0].x[i] = -box->O[0].x[i];
			box->O[1].x[i] = -box->O[1].x[i];
		}
	    l->d[i].Delta = s->O[1].x[i];
	}
	if (l->d[1].Delta>l->d[0].Delta) {
        x = l->cx=1; y = l->cy=0;
	} else {
		x = l->cx=0; y = l->cy=1;
	}
	l->l = l->d[x].Delta;
	l->d[x].d  = l->d[x].Delta/l->d[y].Delta;
	l->d[x].dr = l->d[x].Delta%l->d[y].Delta;
	l->d[x].rt = l->d[y].Delta;
	l->d[x].o = 0;
	l->d[y].d = 0;
	l->d[y].dr = l->d[y].Delta;
	l->d[y].rt = l->d[x].Delta;
	l->d[y].o = 0;
	p->bx = 0; p->ex = l->d[x].d; p->y = 0; p->r = l->d[x].o;
	p->cbx = p->bx; p->cex = p->ex;
}

static void shapeLineHorForEach(segment2d *s,Geo2dShapeMap *Dst) {
	Geo2dRectangle bound;
	int y,x0,x1;
	y = s->O[0].x[1];
	x0 = s->O[0].x[0];
	x1 = s->O[1].x[0];
	if (x0>x1) { x0 = x1+1; x1 = s->O[0].x[0]+1; }
	Call(Dst,Boundary,1(&bound));
	if (y>=bound.Pos.y && y<bound.Pos.y+bound.Extent.h) {
		if (x0<bound.Pos.x) x0 = bound.Pos.x;
		if (x1>bound.Pos.x+bound.Extent.w) x1 = bound.Pos.x+bound.Extent.w;
		if (x0<x1) { Call(Dst,MapLine,3(x0,y,x1-x0)); }
	}
}

static void shapeLineVerForEach(segment2d *s,Geo2dShapeMap *Dst) {
	Geo2dRectangle bound;
	int y0,y1,x;
	x = s->O[0].x[0];
	y0 = s->O[0].x[1];
	y1 = s->O[1].x[1];
	if (y0>y1) { y0 = y1+1; y1 = s->O[0].x[1]+1; }
	Call(Dst,Boundary,1(&bound));
	if (x>=bound.Pos.x && x<bound.Pos.x+bound.Extent.w) {
		if (y0<bound.Pos.y) y0 = bound.Pos.y;
		if (y1>bound.Pos.y+bound.Extent.h) y1 = bound.Pos.y+bound.Extent.h;
		while (y0<y1) { Call(Dst,MapLine,3(x,y0,1)); y0++;}
	}
} 

static void shapeDrawHorClippedLine(segDesc *l,segPtr p,Geo2dShapeMap *Dst) {
	#define Sgn(cx,val) ((sgn##cx>0)?val:-val)
	point2d X;
	int sgnx,sgny;
	int L,dL,dL0,dL1,dr,rt;
	X = l->o; sgnx=l->d[0].sgn; sgny=l->d[1].sgn;
	X.x[0] += Sgn(x,p.bx);
	X.x[1] += Sgn(x,p.y);
	L = l->l; dL0 = l->d[0].d; dL1=dL0+1; dL = p.cex-p.cbx;
	dr = l->d[0].dr; rt = l->d[0].rt;
	if (sgnx>0) {
		L-=dL;
	    while (L>0) {
		    Call(Dst,MapLine,3(X.x[0],X.x[1],dL));
		    X.x[0]+=dL; X.x[1]+=sgny;
		    p.r+=dr; if (p.r>rt) {p.r-=rt; dL=dL1;} else {dL=dL0;}
			L-=dL;
	    }
		if (L+dL>0) Call(Dst,MapLine,3(X.x[0],X.x[1],L+dL));
	} else {
		X.x[0]++; L-=dL;
		while (L>0) {
			X.x[0]-=dL;
			Call(Dst,MapLine,3(X.x[0],X.x[1],dL));
			p.r+=dr; if (p.r>rt) {p.r-=rt; dL=dL1;} else {dL=dL0;}
			X.x[1]+=sgny; L-=dL;
		}
		if (L+dL>0) { Call(Dst,MapLine,3(X.x[0]-(L+dL),X.x[1],L+dL));}
	}
    #undef Sgn
}

static void shapeDrawClippedLine(segDesc *l,segPtr p,Geo2dShapeMap *Dst) {
	#define Sgn(cx,val) ((sgn##cx>0)?val:-val)
	point2d X;
	int x,y,sgnx,sgny;
	int *Xx,*Xy;
	int L,dL,dL0,dr,rt;
	X = l->o; x = l->cx; y = l->cy; sgnx=l->d[x].sgn; sgny=l->d[y].sgn;
	Xx = X.x+x; Xy = X.x+y;
	*Xx += Sgn(x,p.bx);
	*Xy += Sgn(y,p.y);
	L = l->l; dL0 = l->d[x].d; dL = p.cex-p.cbx;
	dr = l->d[x].dr; rt = l->d[x].rt;
	while (L) {
		L-=dL;
		while (dL) {Call(Dst,MapLine,3(X.x[0],X.x[1],1)); *Xx+=sgnx; dL--;}
		*Xy+=sgny;
		p.r += dr; if (p.r>rt) {p.r-=rt; dL=dL0+1;} else {dL=dL0;}
		if (dL>L) dL=L;
	}
    #undef Sgn
}

static int segInterBox(segPtr *R,int x,segment2d *box) {
	int y,r;
	y = 1-x;
    r = (R->y>=0)&&(R->y<=box->O[1].x[y]);
	if (r) {
		int x0,x1;
		x0 = R->bx; x1 = R->ex;
		if (x0<0) x0 = 0;
		if (x1>box->O[1].x[x]+1) x1 = box->O[1].x[x]+1;
		r = (x0<x1);
		R->cbx=x0; R->cex=x1;
	}
    return r;
}

static void shapeLineForEach(Geo2dShape *this,Geo2dShapeMap *Dst) {
	segment2d s;
	ThisToThat(shapeLine,Geo2dShape);
	s = that->s;
	if ((s.O[0].x[0]==s.O[1].x[0])||(s.O[0].x[1]==s.O[1].x[1])) {
		if (s.O[0].x[1]==s.O[1].x[1]) {
			shapeLineHorForEach(&s,Dst);
		} else {
			shapeLineVerForEach(&s,Dst);
		}
	} else {
	    int draw,Aib,Bib;
		int x,y;
	    segment2d box;
	    segDesc l;
	    segPtr inbox[6],*p;
		p = inbox;
        /* Get Boundary */ {
	        Geo2dRectangle bound;
	        Call(Dst,Boundary,1(&bound));
		    box.O[0].x[0] = bound.Pos.x; box.O[0].x[1] = bound.Pos.y; 
			box.O[1] = box.O[0]; box.O[1].x[0] += bound.Extent.w-1; box.O[1].x[1] += bound.Extent.h-1;
	    }
    #define InBox(A) (A.x[0]>=box.O[0].x[0] && A.x[0]<=box.O[1].x[0] && A.x[1]>=box.O[0].x[1] && A.x[1]<=box.O[1].x[1])
		Aib = InBox(s.O[0]); Bib = InBox(s.O[1]);
    #undef InBox
	    draw = Aib && Bib;
	    SegDescInit(&l,p,&box,&s);
		x = l.cx; y = l.cy;
	    if (!draw) { // The line is clipped or outside
			if (segInterBox(p,x,&box)) p++;
			draw = (box.O[1].x[0]>=0 && box.O[1].x[1]>=0 && s.O[1].x[0]>box.O[0].x[0] && s.O[1].x[1]>box.O[0].x[1]);
			if (draw) {
				segPtr *bf,*ef,*q;
				if (box.O[0].x[y]>0) {  // bottom clip
					affineGetYRange(p,l.d+x,box.O[0].x[y]); 
					if (segInterBox(p,x,&box)) p++;
				}
				if (box.O[1].x[y]<s.O[1].x[y]) { // top clip
					affineGetYRange(p,l.d+x,box.O[1].x[y]); 
					if (segInterBox(p,x,&box)) p++;
				}
	#define InBox() (p->bx>=box.O[0].x[x] && p->bx<=box.O[1].x[x] && p->y>=box.O[0].x[y] && p->y<=box.O[1].x[y])
				if (box.O[0].x[x]>0) { // left clip
					affineGetYRange(p,l.d+y,box.O[0].x[x]);
                    if (InBox()) {affineGetYRange(p,l.d+x,p->bx); segInterBox(p,x,&box); p++;}
				}
				if (box.O[1].x[x]<s.O[1].x[x]) { // right clip
					affineGetYRange(p,l.d+y,box.O[1].x[x]);
					if (InBox()) {affineGetYRange(p,l.d+x,p->bx); segInterBox(p,x,&box); p++;}
				}
	#undef InBox
				bf = ef = 0;
                q = inbox;
				while (q<p) {
					if (!bf) {bf = ef = q;} else {
						if (q->cex>=ef->cex) { ef = q; } else {
							if (bf->cbx>=q->cbx) {bf = q;}
						} 
					}
					q++;
				}
				draw = (bf!=0);
				if (draw) {
					p = bf;
					if (Bib) {
						l.l = s.O[1].x[x]-bf->cbx;
					} else {
						l.l = ef->cex - bf->cbx;
					}
				}
			}
	    }
	    if (draw) {
			if (x==0) {
                shapeDrawHorClippedLine(&l,*p,Dst);
			} else {
			    shapeDrawClippedLine(&l,*p,Dst);
			}
	    }
	}
}

Geo2dShape *Geo2dShapeLine(Geo2dPoint *A,Geo2dPoint *B) {
	shapeLine *r;
	static struct Geo2dShape Static = {&shapeLineForEach};
	rPush(r);
	r->Geo2dShape.Static = &Static;
	r->s.O[0].x[0] = A->x;
	r->s.O[0].x[1] = A->y;
	r->s.O[1].x[0] = B->x;
	r->s.O[1].x[1] = B->y;
	return &r->Geo2dShape;
}

/*-------------------------------------*/

static void shapeWLineForEach(Geo2dShape *this,Geo2dShapeMap *Dst) {
	shapeLineForEach(this,Dst);
}

Geo2dShape *Geo2dShapeWLine(Geo2dPoint *A,Geo2dPoint *B,int Width) {
	shapeLine *r;
	static struct Geo2dShape Static = {&shapeWLineForEach};
	rPush(r);
	r->Geo2dShape.Static = &Static;
	r->s.O[0].x[0] = A->x;
	r->s.O[0].x[1] = A->y;
	r->s.O[1].x[0] = B->x;
	r->s.O[1].x[1] = B->y;
	if (Width<=1) { Width = 1; }
	r->Width = Width;
	return &r->Geo2dShape;
}




