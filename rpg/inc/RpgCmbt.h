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

#include <RpgCore.h>

/*_________________________________
 |
 |
 |_________________________________
*/

typedef struct CmbtActr CmbtActr;

typedef struct CmbtFaction CmbtFaction;
typedef struct MemberStatus MemberStatus;

static int ActrIsAble(CmbtActr *a);
static int ActrGetStatus(CmbtActr *a);
static void ActrSetStatus(CmbtActr *a,int Status);
static MemberStatus *MemberGetStatus(CmbtFaction *t,CmbtActr *a);
static int MemberStatusChanged(CmbtFaction *t,MemberStatus *a);

static CmbtFaction *CmbtFactionInit(CmbtFaction *r,char *Name,TlsMemPool *mem);
static MemberStatus *FactionAddMember(CmbtFaction *t,CmbtActr *act);
static int FactionRemoveMember(CmbtFaction *t,MemberStatus *a);
static void FactionAddOther(CmbtFaction *t,CmbtFaction *other,int Lvl);
static void FactionFree(CmbtFaction *t);

static CmbtActr *CmbtFactionForEachMember(
    CmbtFaction *t,int (*Cont)(CmbtActr *Actr,void *cls),void *cls
);
static CmbtFaction *CmbtFactionForEachOther(
    CmbtFaction *t,int (*Cont)(CmbtFaction *f,int FriendLvl,void *cls),void *cls
);

/*_________________________________
 |
 |
 |_________________________________
*/



