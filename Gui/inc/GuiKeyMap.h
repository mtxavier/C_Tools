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

#ifndef _GuiKeyMap_h_
#define _GuiKeyMap_h_

/*______________________________________________________________________________
 |
 | These are the physical binding for a standard 102 touch keyboard.
 | The keys are labeled after the QWERTY layout. Otherwise, the labels have no
 | special significance. For instance, you could bind the "Q" label to an extra
 | ALT key should you fancy it...
 |______________________________________________________________________________
*/

#define GuiKeyTyp(n) (n&0xff0000)
#define GuiKeyTypAscii 0x0
#define GuiKeyTypAlt 0x10000
#define GuiKeyTypFn 0x20000

#define GuiKey(f,v) (GuiKeyTyp##f|((C_(Key,Scan,f.v))<<8)|(C_(Key,Scan,f.v)))

struct C_Key {
    struct {
    struct { 
        char _,
        AltLeft,AltRight,CtrlLeft,CtrlRight,
        ShiftLeft,ShiftRight,CapsLock,NumLock,
        nb;
    } Alt;
    struct {
        char _,
        Esc,BackSpace,Tab,Enter, 
        Left,Right,Up,Down,Insert,Suppr,PgUp,PgDown,Begin,End,
        F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
        xEnter, xLeft, xRight, xUp, xDown, xInsert, xSuppr,
        xPgUp, xPgDown, xBegin, xEnd, xPlus, xMinus, xDiv,
        xMul, x5, xEqual, Pause, Print, ScrollLock,
        nb;
    } Fn;
// There are rumor of other key fn like window, menu, apple... that never 
// happened.
// We're going QWERTY as a default (the less symbols, the better)
    struct {
        char _, 
        Tilde, k1, k2, k3, k4, k5, k6, k7, k8, k9, k0, Minus, Equal, 
        Q, W, E, R, T, Y, U, I, O, P, OpenBracket, CloseBracket, 
        A, S, D, F, G, H, J, K, L, SemiColon, Quote, BackSlash, 
        Less, Z, X, C, V, B, N, M, Coma, Period, Slash, Space,
        nb;
    } Key;
    } Scan;
};

#endif
