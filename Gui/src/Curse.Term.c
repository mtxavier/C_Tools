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

#include <stdio.h>
#include <curses.h>
#include <Classes.h>
#include <StackEnv.h>
#include <Gui/GuiKeyMap.h>
#include <Gui.Term.h>

/*-------------------------*/

typedef struct {
    unsigned char *d;
    int dx,dy;
} Map;

typedef struct {
    int _inited;
    GuiTerm GuiTerm;
    GuiTermCtrl GuiTermCtrl;
    GuiTermInfo Info;
    WINDOW *wnd;
} CurseEnv;

static GuiTermInfo *CurseCtrlGetInfo(GuiTermCtrl *this) {
    ThisToThat(CurseEnv,GuiTermCtrl);
    return &that->Info;
}
static GuiTerm *CurseGetViewPort(GuiTermCtrl *this) {
    ThisToThat(CurseEnv,GuiTermCtrl);
    return &that->GuiTerm;
}
static int CurseSetColor(GuiTermCtrl *this,int num,GeoRGBA *n) {
    int res;
    ThisToThat(CurseEnv,GuiTermCtrl);
    res = ((num>=that->Info.Color.Palette.ModRange.b)&&(num<that->Info.Color.Palette.ModRange.e));
    if (res) {
        GeoRGBA *c;
            int r,g,b;
            c = that->Info.Color.Palette.b + num;
            *c = *n;
            r = (n->r*1000)/255;
            g = (n->g*1000)/255;
            b = (n->b*1000)/255;
            init_color(num,r,g,b);
	}
	return res;
}
static int CurseSetColorPair(GuiTermCtrl *this,int num,int bg,int fg) {
    int res;
    ThisToThat(CurseEnv,GuiTermCtrl);
    res = ((num>=that->Info.Color.Pairs.ModRange.b)&&(num<that->Info.Color.Pairs.ModRange.e));
    if (res) {
        init_pair(num,fg,bg);
    }
    return res;
}

static int CurseGetCh(GuiTermCtrl *this) {
    int r,c;
    ThisToThat(CurseEnv,GuiTermCtrl);
    r = c = getch();
    if (c<0x20) {
        if (c<0) {
            r = GuiKeyNoKey; // includes the case c == ERR
        } else {
            static int transl[] = {
                /* 0*/ 0, 1, 2, 3,
                /* 4*/ 4, 5, 6, 7,
                /* 8*/ GuiKey(Fn,BackSpace),GuiKey(Fn,Tab),GuiKey(Fn,Enter), 11,
                /*12*/ 12, GuiKey(Fn,Enter), 14, 15,
                /*16*/ 16, 17, 18, 19,
                /*20*/ 20, 21, 22, 23,
                /*24*/ 24, 25, GuiKeyNoKey, GuiKey(Fn,Esc),
                /*28*/ 28, 29, 30, 31
            };
            r = transl[c];
        }
    } else {
        if (c>=KEY_MIN) {
            r = ' ';
            if (c<=KEY_F(12)) {
                static int transl[] = {
                /* 0401 */ GuiKey(Fn,Pause), GuiKey(Fn,Down), GuiKey(Fn,Up), GuiKey(Fn,Left),
                /* 0405 */ GuiKey(Fn,Right), GuiKey(Fn,Begin), GuiKey(Fn,BackSpace), ' ',
                /* 0411 */ GuiKey(Fn,F1), GuiKey(Fn,F2), GuiKey(Fn,F3), GuiKey(Fn,F4), 
                /* 0415 */ GuiKey(Fn,F5), GuiKey(Fn,F6), GuiKey(Fn,F7), GuiKey(Fn,F8), 
                /* 0421 */ GuiKey(Fn,F9), GuiKey(Fn,F10), GuiKey(Fn,F11), GuiKey(Fn,F12)
                };
                r = transl[c-KEY_MIN];
            }
            if ((c>=0510)&&(c<0640)) {
                static int transl[] = {
                    /* 0510 */ ' ',' ',GuiKey(Fn,Suppr),GuiKey(Fn,Insert),
                    /* 0514 */ ' ',' ',' ',' ',
                    /* 0520 */ ' ',' ',GuiKey(Fn,PgDown),GuiKey(Fn,PgUp),
                    /* 0524 */ ' ',' ',' ',' ',
                    /* 0530 */ ' ',' ',GuiKey(Fn,Print),' ',
                    /* 0534 */ GuiKey(Fn,xBegin),GuiKey(Fn,xPgUp),GuiKey(Fn,x5),GuiKey(Fn,xEnd),
                    /* 0540 */ GuiKey(Fn,xPgDown),' ',' ',' ',
                    /* 0544 */ ' ',' ',' ',' ',
                    /* 0550 */ GuiKey(Fn,End),' ',' ',' ',
                    /* 0554 */ ' ',' ',' ',' ',
                    /* 0560 */ ' ',' ',' ',' ',
                    /* 0564 */ ' ',' ',' ',' ',
                    /* 0570 */ ' ',' ',' ',' ',
                    /* 0574 */ ' ',' ',' ',GuiKey(Fn,Suppr),
                    /* 0600 */ ' ',' ',GuiKey(Fn,End),' ',
                    /* 0604 */ ' ',' ',' ',GuiKey(Fn,Begin),
                    /* 0610 */ GuiKey(Fn,Insert),GuiKey(Fn,Left),' ',' ',
                    /* 0614 */ ' ',' ',' ',' ',
                    /* 0620 */ ' ',' ',GuiKey(Fn,Right),' ',
                    /* 0624 */ ' ',' ',' ',' ',
                    /* 0630 */ ' ',GuiKeyNoKey,GuiKeyNoKey,GuiKeyNoKey,
                    /* 0634 */ ' ',' ',' ',' '
                };
                r = transl[c-0510];
            }
        }
        if (c==KEY_RESIZE) {
            that->Info.Dim.w = COLS;
            that->Info.Dim.h = LINES;
            r = GuiKeyNoKey;
        }
        if (c==KEY_MOUSE) { }
    }
    return r;
}

static void CurseClose(GuiTermCtrl *this) {
    ThisToThat(CurseEnv,GuiTermCtrl);
    if (that->_inited) {
        that->_inited = (0!=0);
        endwin();
    }
}

/*   */

static GuiTermInfo *CurseGetInfo(GuiTerm *this) {
    ThisToThat(CurseEnv,GuiTerm);
    return &that->Info;
}
static void CurseClear(GuiTerm *this) {
    ThisToThat(CurseEnv,GuiTerm);
    clear();
    move(0,0);
}
static void CurseFontSet(GuiTerm *this,int fnt) {
    static int TranslAttr[4] = {A_NORMAL,A_BOLD,A_NORMAL,A_BOLD};
    ThisToThat(CurseEnv,GuiTerm);
    attrset(TranslAttr[fnt&3]);
}
static void CurseColorSet(GuiTerm *this,int Pair) {
    // color_set(((Back&7)<<3)|(Front&7),NULL);
    color_set(Pair,NULL);
}
static void CurseAddStr(GuiTerm *this,int x,int y,const char *s) {
    ThisToThat(CurseEnv,GuiTerm);
    mvaddstr(y,x,s);
}
static void CurseRefresh(GuiTerm *this) {
    ThisToThat(CurseEnv,GuiTerm);
    mvaddstr(that->Info.Dim.h-1,that->Info.Dim.w-1,"");
    refresh();
}

/*   */

static void CurseOpen(CurseEnv *that) {
    if (!that->_inited) {
    that->wnd = initscr();
    that->_inited = (0==0);
    that->Info.Dim.w = COLS;
    that->Info.Dim.h = LINES;
    if (has_colors()) {
        int fg,bg,n,colnb,pairnb;
        struct GuiColorPair *pairs,*epairs;
        GeoRGBA *pal,*epal;
        start_color();
        colnb = COLORS;
        rnPush(pal,colnb);
        that->Info.Color.Palette.b = pal;
        that->Info.Color.Palette.e = pal+colnb;
        that->Info.Color.Palette.ModRange.b = 0;
        that->Info.Color.Palette.ModRange.e = 0;
        if (can_change_color()) {
            that->Info.Color.Palette.ModRange.b = 0;
            that->Info.Color.Palette.ModRange.e = colnb-1;
                /* for (fg=0;fg<8;fg++) {
                        int r,g,b;
                        r = (fg&1)?1000:0;
                        g = (fg&2)?1000:0;
                        b = (fg&3)?1000:0;
                        init_color(fg,r,g,b);
                } */
        }
        epal = pal+colnb; n = 0;
        while (pal<epal) {
            short r,g,b;
            int f;
            pal->a = 0xff;
            color_content(n,&r,&g,&b);
            f = r; pal->r = (f<<8)/1001;
            f = g; pal->g = (f<<8)/1001;
            f = b; pal->b = (f<<8)/1001;
            pal++; n++;
        }
        pairnb = COLOR_PAIRS;
        rnPush(pairs,pairnb);
        that->Info.Color.Pairs.b = pairs;
        that->Info.Color.Pairs.e = pairs+pairnb;
        that->Info.Color.Pairs.ModRange.b = 1; 
        that->Info.Color.Pairs.ModRange.e = pairnb; 
        epairs = pairs+pairnb; n = 0;
        while (pairs<epairs) {
            short fg,bg;
            pair_content(n,&fg,&bg);
            pairs->fg = fg;
            pairs->bg = bg;
            pairs++; n++;
        }
        if (pairnb>=64) {
        for (bg=0;bg<8;bg++) {
            for (fg=0;fg<8;fg++) {
                if (bg||fg) { init_pair((bg<<3)+fg,fg,bg); }
            }
        }
        }
    } else {
        static struct GuiColorPair pairs[1]={{0,1}};
        static GeoRGBA pal[2]={{0,0,0,0xff},{0xff,0xff,0xff,0xff}};
        that->Info.Color.Palette.b = pal;
        that->Info.Color.Palette.e = pal+2;
        that->Info.Color.Palette.ModRange.b = 0;
        that->Info.Color.Palette.ModRange.e = 0;
        that->Info.Color.Pairs.b = pairs;
        that->Info.Color.Pairs.e = pairs+1;
        that->Info.Color.Pairs.ModRange.b = 0;
        that->Info.Color.Pairs.ModRange.e = 0;
    }
    {
        static struct GuiTermFontDesc fd[4] = {
        {0,(0!=0),(0!=0)},{0,(0==0),(0!=0)},{0,(0!=0),(0==0)},{0,(0==0),(0==0)}
        };
        that->Info.Font.b = fd;
        that->Info.Font.e = fd+2;
    }
    cbreak();
    noecho();
    timeout(40);
    // timeout(-1); for getch; timeout(n): wait at most n ms; timeout(0): do not wait; timeout(-n) : block.
    // leaveok(that->wnd,TRUE); // ignore cursor motion
    nonl();
    intrflush(stdscr,FALSE);
    keypad(stdscr,TRUE);
    }
}
GuiTermCtrl *GuiCurseTerm(void) {
    static struct GuiTermCtrl CtrlStatic = {
        CurseCtrlGetInfo,CurseGetViewPort,CurseSetColor,CurseSetColorPair,CurseGetCh,CurseClose
    };
    static struct GuiTerm Static = {
        CurseGetInfo,CurseClear,CurseFontSet,CurseColorSet,CurseAddStr,CurseRefresh
    };
    CurseEnv *r;
    rPush(r);
    r->GuiTerm.Static = &Static;
    r->GuiTermCtrl.Static = &CtrlStatic;
    r->_inited = (0!=0);
    CurseOpen(r);
    return &r->GuiTermCtrl;
}

