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

#ifndef _RpgAltrn_h_
#define _RpgAltrn_h_

#include <RpgCore.h>

typedef struct {
    struct {
         char d1,d2,d4,d6,d8,d10,d12,d20,d100,nb;
    } Dice;
    struct {
        char Lvl,Exp;
        struct {
            char Blue,Green,Red,Earth,Water,Air,Fire,Balance,nb;
        } Base;
        struct {
            char Blue,Green,Red,Earth,Water,Air,Fire,Balance,Inbalance;
            char Body,Mind,Soul,Perception,Sharpness,Resilience,Power;
            char Metal,Wood,Lightning,Ice,Heat,Darkness,Light,Magma,Crystal,
                 Acid,Sound,Force,Holy,Curse,Poison;
            char Constitution,Strength,Reaction,Agility;
            char Understanding,Technicity,Sanity,Memory;
            char Intuition,Charisma,Determination,Creativity;
            char Magic,Ki,Channel;
            char nb;
        } Affinity;
        struct {
            char Size,Weight,MaxHp,Hp,DR,Material,Reach,nb;
        } Body;
        struct {
            struct {
                char Brawl,Melee,Range,Defense,
                Athletism,Swim,Fly,Climb,Acrobatic,
                Ride,Survival,FirstAid,Stealth;
            } General;
            struct {
                char Alchemy,Thaumaturgy,Mechanic,Metallurgy,Pottery,Tressage,
                Electronic,Chemistry,Biochemistry,Cooking,Masonry,Sewing,
                Infrastructure,Production,Medicine,Physics,Mathematic;
            } Engineer;
            struct {
                char Leadership,Accounting,Law,Logistic,Strategy,Command,
                State,City,Organisation,Trade,Warfare,Farming,Negociation;
            } Administration;
            struct {
                 char Game,Drawing,Disguise,Literature,Music,Acting,FastTalk,
                 Interrogation;
            } Art;
            struct {
                char Geography,History,Folklore,Religion,Tradition,CityFolks,
                CountryFolks,CoastFolks,Tribes,Workers,Mobs,Artists,Nobles,
                Clergy,Corporates;
            } Culture;
            char nb;
        } Skills;
        struct {
            struct {
                char Lvl;
                struct {
                    char Help,Tool,Workshop,Spell;
                } Bonus;
                struct {
                    char Handicap,Wound,Light,Balance,Limb,Spell;
                } Malus;
                char nb;	
            } Skill;
            char nb;
        } Mod;
    } PC;
    struct {
        char Ok,Wounded,KO,Killed,Subdued,Downed,
        Petrified,Paralyzed,Frozen,
        Dominated,Confused,Berserked,Delirious,Dizzy,Sickened,
        Panicked,Frightened,Terrified,
        Asleep,Dowsy;
        char Tired,Exhausted,Cursed,Hungry,Starving,Famished,Thirsty,
        Dehydrated,HoverHeat,Freezing,Suffocating,Drowning;
        char Flying,Hanging,Standing,OnFour,Sitting,Kneeling,Laying,Prone,Tied;
        char Stealthed,Walking,Moving,Still;
        char Blinded,Deafened;
        char Poisoned,Burning,Ill;
        char nb;
    } Status;
    struct {
        char Base,Rank,AltBase,Workshop,Tool,Affinity,Buff,Debuff,Expertise,
        Environment,Opposition,Support,Requisit,Wound,Resistance,Vulnerability,
        nb;
    } Modifier;
    struct {
        struct {
            char Dwarf,Elf,Gnome,Gobelin,HalfGiant,Human,Orc,Pixie,
            Ogre,Troll,Giant,Lutin,Farfadet,Diablotin,Fae,
            nb;
        } Race;
        struct {
            char Schemer[20],Crafty[20],Chaneler[20];
            struct {
                char Duelist[20],Brawler[20],Hoplite[20],Berserker[20],
                Ravager[20],Archer[20],Thrower[20],Rider[20];
                char Diplomat[20],Infiltrator[20],Scout[20],Leader[20],
                Investigator[20],Investor[20],Crafter[20],Artist[20],
                Athlet[20],Scholar[20],Mesmer[20];
            } Crafts;
            struct {
                char Alchemist[20],Gadgeteer[20],Enchanter[20],Shaman[20],
                Ritualist[20],Tactician[20];
            } Schemes;
            struct {
                char Geomancer[20];
                char Telekinesist[20],Telepath[20],Pyrokinesist[20],
                Electrokinesist[20];
                struct {
                    char Healing[20],War[20],Nature[20],Protection[20],
                    Storm[20],Sea[20],Sun[20],Moon[20],Forge[20],Time[20],
                    Animals[20],Plants[20],Trade[20],Death[20],Light[20],
                    Darkness[20],Travel[20];
                } Domain;
            } Chanels;
        } Classes;
    } Templates;
} C_Altrn;

typedef struct { struct RpgCoreScene *Static; } RpgCoreScene;
struct RpgCoreScene {
    void (*Perform)(RpgCoreScene *this);
    void (*Close)(RpgCoreScene *this);
};

extern RpgCoreScene RpgCoreSceneNull;

typedef struct {
    RpgCoreScene RpgCoreScene;
    struct AltrnFightScene *Static;
} AltrnFightScene;

typedef struct { struct RpgFightFaction *Static; } RpgFightFaction;
typedef struct {
    struct AltrnFightActor *Static;
    struct {
        TlsIntVal *Skill;
        TlsIntVal *Attr;
    } Atk;
    int Faction,Id;
    TlsIntVal *Def,*DR;
    RpgDice *Dmg;
    TlsIntVal *Hp;
} AltrnFightActor;

struct RpgFightFaction {
    int (*Effectif)(RpgFightFaction *this);
    int (*Standing)(RpgFightFaction *this);
    int (*Defeated)(RpgFightFaction *this);
};

struct AltrnFightActor {
    void (*NewRound)(AltrnFightActor *this,AltrnFightScene *scene,
        int round);
    void (*TakeDmg)(AltrnFightActor *this,int Dmg);
    int (*CheckStatus)(AltrnFightActor *this,int Status);
    void (*Act)(AltrnFightActor *this,AltrnFightScene *scene);
};
struct AltrnFightScene {
    void (*FighterAdd)(AltrnFightScene *this,int faction,int Id,
        AltrnFightActor *fighter);
    int (*GetFactionStatus)(AltrnFightScene *this,int State);
};



#endif
