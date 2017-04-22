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

#include <Classes.h>
#include <StackEnv.h>
#include <List.h>
#include <Tools.h>
#include <Browser.h>
#include <RpgCore.h>

typedef struct { struct RpgLootItem *Static; } RpgLootItem;
typedef struct { struct RpgLootList *Static; } RpgLootList;
typedef struct { struct RpgLootTable *Static; } RpgLootTable;
struct RpgLootTable {
	int (*AddItem)(RpgLootTable *this,int Weight,int Nb,RpgLootItem *Item);
	int (*Weight)(RpgLootTable *this);
	void (*Supply)(RpgLootTable *this,RpgLootItem *Item,int nb);
    void (*Draw)(RpgLootTable *this,RpgLootList *Target,int nb);
	void (*Reset)(RpgLootTable *this);
	void (*Clear)(RpgLootTable *this);
};
struct RpgLootList {
	void (*Add)(RpgLootList *this,int Nb,RpgLootItem *Item);
};

/*--------------------*/

typedef struct { struct RpgWeapon *Static; } RpgWeapon;
struct RpgWeapon {
    RpgPhyItem *(*GetItem)(RpgWeapon *this);
};

/*---------------------*/

typedef struct {
	List/*<ItemDist.List>*/ List;
	RpgLootItem *Item;
	int Nb,Left,Weight;
} ItemDist;
typedef struct {
	RpgLootTable RpgLootTable;
	BrwRSerial *Draw;
	TlsMemPool *Pool;
	int TotalWeight,CommonWeight;
	List/*<ItemDist.List>*/ Commons,Uniques;
} LootTable;
static int LootTableAddItem(RpgLootTable *this,int weight,int nb,RpgLootItem *Item) {
	List *l,*e;
	ItemDist *L;
	int cont,uniques,*w;
	ThisToThat(LootTable,RpgLootTable);
	uniques = that->CommonWeight-that->TotalWeight;
	if (nb>=0) { e = &(that->Uniques); w = &uniques; } else {  e = &(that->Commons); w = &that->CommonWeight; }
	l = e->n; cont = (l!=e);
	while (cont) {
		L = CastBack(ItemDist,List,l);
		cont = (L->Item!=Item);
		if (cont) { l=l->n; cont=(l!=e); }
	}
	if (l==e) {
		L = Call(that->Pool,Alloc,0);
		L->Item = Item;
		L->Nb = L->Left = nb; L->Weight = weight;
		l = &L->List; l->n = e; e->n = l;
	} else {
		if (nb>=0) { L->Nb+=nb; }
		L->Weight += weight;
	}
	*w += weight;
	that->TotalWeight = uniques+that->CommonWeight;
	return (0==0);
}
static int LootTableWeight(RpgLootTable *this) {
	ThisToThat(LootTable,RpgLootTable);
	return that->TotalWeight;
}
static void LootTableSupply(RpgLootTable *this,RpgLootItem *Item,int nb) {
	List *l,*e;
	ItemDist *L;
	int cont;
	ThisToThat(LootTable,RpgLootTable);
	e = &that->Uniques;
	l = e->n; cont = (l!=e);
	while (cont) {
		L = CastBack(ItemDist,List,l);
		cont = (L->Item = Item);
		if (!cont) {
			if ((L->Left<=0)&&((L->Left+nb)>0)) { that->TotalWeight+=L->Weight; }
			if ((L->Left>0)&&((L->Left+nb)<=0)) { that->TotalWeight-=L->Weight; }
			L->Left += nb;
		} else {
		    l = l->n; cont = (l!=e);
		}
	}
}
static void LootTableDraw(RpgLootTable *this,RpgLootList *Target,int nb) {
	int drw,*bd,wgt;
	ThisToThat(LootTable,RpgLootTable);
	bd = drw;
	wgt = that->TotalWeight;
	while (nb) {
		List *l,*e;
		ItemDist *L;
		int cont;
        drw = wgt;
	    Call(that->Draw,Read,2(bd,bd+1));
		if (drw>=that->CommonWeight) {
			drw -= that->CommonWeight;
			e = &that->Uniques; l = e->n; cont = (l!=e);
			while (cont) {
				L = CastBack(ItemDist,List,l);
				if (L->Left>0) { 
					drw -= L->Weight;
					cont = (drw>=0);
					if (cont) l = l->n;
				} else {
					l = l->n;
				}
				cont = cont&&(l!=e);
			}
			if (l!=e) {
				L = CastBack(ItemDist,List,l);
				Call(Target,Add,2(1,L->Item));
				L->Left--;
				if (!L->Left) { wgt -= that->Weight; }
			}
		} else {
			e = &that->Commons; l = e->n; cont = (l!=e);
			while (cont) {
				L = CastBack(ItemDist,List,l);
				drw -= L->Weight;
				cont = (drw>=0);
				if (cont) { l=l->n; cont=(l!=e); }
			}
			if (l!=e) {
				L = CastBack(ItemDist,List,l);
				Call(Target,Add,2(1,L->Item));
			}
		}
	    nb--;
	}
	that->TotalWeight = wgt;
}
static void LootTableReset(RpgLootTable *this) {
	List *l,*e;
	ItemDist *L;
	ThisToThat(LootTable,RpgLootTable);
	e = &(that->Uniques); l = e->n;
	while (l!=e) {
		L = CastBack(ItemDist,List,l);
		if ((L->Left<=0)&&(L->Nb>0)) { that->TotalWeight += L->Weight; }
		if ((L->Left>0)&&(L->Nb<=0)) { that->TotalWeight -= L->Weight; }
		L->Left = L->Nb;
		l = l->n;
	}
}
static void LootTableClear(RpgLootTable *this) {
	List *e;
	ThisToThat(LootTable,RpgLootTable);
	e = &(that->Commons);
	while (e) {
		l = e->n; e->n = e;
	    while (l!=e) {
	        ItemDist *L;
			L = CastBack(ItemDist,List,l);
			l = l->n;
            Call(that->Pool,Free,1(L));
	    }
		if (e==&(that->Commons)) { e=&(that->Uniques); } else { e = 0;}
	}
	that->TotalWeight = that->CommonWeight = 0;
}
static RpgLootTable *RpgLootTableNew(TlsMemPool *Pool,BrwRSerial *u256Draw) {
	LootTable *R;
	static struct RpgLootTable Static = {
		LootTableAddItem,LootTableWeight,LootTableSupply,LootTableDraw,LootTableReset,LootTableClear
	};
	rPush(R);
	R->RpgLootTable.Static = &Static;
	R->Pool = Call(Pool,SizeSelect,1(sizeof(ItemDist)));
	R->TotalWeight = R->CommonWeight = 0;
	R->Commons.n = &R->Commons;
	R->Uniques.n = &R->Uniques;
	R->Draw = BrwMixedDice(256,u256Draw);
	return &R->RpgLootTable;
}

