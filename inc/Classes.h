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

#ifndef _Classes_h_
#define _Classes_h_

#define ThisToThat(type,field)\
    type *that;\
	that = (type *)(((void *)this)+((void *)0-(void *)(&((type *)0)->field)));

#define CastBack(type,field,var)\
	((type *)(((void *)(var))+((void *)0-(void *)(&((type *)0)->field))))

#define _Count0
#define _Count1(P0) ,P0
#define _Count2(P0,P1) ,P0,P1
#define _Count3(P0,P1,P2) ,P0,P1,P2
#define _Count4(P0,P1,P2,P3) ,P0,P1,P2,P3
#define _Count5(P0,P1,P2,P3,P4) ,P0,P1,P2,P3,P4
#define _Count6(P0,P1,P2,P3,P4,P5) ,P0,P1,P2,P3,P4,P5
#define _Countp(P0,P1) _Count##P0 _Count##P1

#define Call(Object,Function,Params)\
    (Object)->Static->Function((Object)_Count##Params)

#define C_(Strct,d,Elt) (((char *)(&((struct C_##Strct *)0)->d.Elt))-(((char *)(&((struct C_##Strct *)0)->d))))
#define c_(Strct,Deriv,Elt) (((char *)(&(((C_##Strct *)0)->Deriv.Elt)))-((char *)(&(((C_##Strct *)0)->Deriv))))

#endif
