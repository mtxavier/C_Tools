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

#include <Classses.h>
#include <StackEnv.h>
#include <RpgAltrn.h>

#define l_(b,c) c_(Altrn,b,c)

/*____________________________________________________________________________
 |
 | Races
 |____________________________________________________________________________
*/

#define Feature(StatId,Val) \
    Call(T,AddModifier,2( \
        l_(PC,StatId),RpgStraightModifier(l_(Modifier,Rank),Val)) \
    )
static RpgObjTemplate *Tiny(RpgObjTemplate *T) {
    Feature(Body.Size,-6);
    Feature(Body,Reach,-1);
    Feature(Body,DR,-2);
    return T;
}
static RpgObjTemplate *Small(RpgObjTemplate *T) {
    Feature(Body.Size,-3);
    Feature(Body,DR,-1);
    return T;
}
static RpgObjTemplate *Medium(RpgObjTemplate *T) {
    return T;
}
static RpgObjTemplate *Large(RpgObjTemplate *T) {
    Feature(Body.Size,+3);
    Feature(Body,DR,+1);
    return T;
}
static RpgObjTemplate *VeryLarge(RpgObjTemplate *T) {
    Feature(Body.Size,+6);
    Feature(Body,DR,+2);
    Feature(Body,Reach,+1);
    return T;
}
static RpgObjTemplate *Huge(RpgObjTemplate *T) {
    Feature(Body.Size,+9);
    Feature(Body,DR,+3);
    Feature(Body,Reach,+2);
    return T;
}
static RpgObjTemplate *Gargantuan(RpgObjTemplate *T) {
    Feature(Body.Size,+12);
    Feature(Body,DR,+4);
    Feature(Body,Reach,+3);
    return T;
}

/*----------------*/

static RpgObjTemplate *Mammal(RpgObjTemplate *T) {
    return T;
}
static RpgObjTemplate *Reptilian(RpgObjTemplate *T) {
    Feature(Affinity,Ice,-1);
    Feature(Affinity,Light,+1);
    Feature(Affinity.Resilience,+1);
    return T;
}
static RpgObjTemplate *Insectoid(RpgObjTemplate *T) {
    Feature(Affinity.Soul,-1);
    Feature(Affinity.Creativity,-1);
    Feature(Affinity.Acid,+1);
    Feature(Affinity.Poison,+1);
    Feature(Body.DR,+1);
    return T;
}
static RpgObjTemplate *Ooze(RpgObjTemplate *T) {
    Feature(Base.Blue,+2);
    Feature(Base.Green,+1);
    Feature(Base.Red,+1);
    Feature(Base,Fire,+1);
    Feature(Base,Water,+1);
    return T;
}
static RpgObjTemplate *Avian(RpgObjTemplate *T) {
    Feature(Base.Air,+1);
    Feature(Base.Earth,-1);
}
static RpgObjTemplate *Fish(RpgObjTemplate *T) {
    Feature(Base.Water,+2);
    Feature(Base.Air,-1);
    Feature(Base.Fire,-1);
}

