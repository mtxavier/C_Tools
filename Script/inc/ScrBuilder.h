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

#ifndef _ScrBuilder_h_
#define _ScrBuilder_h_

/*_____________________________________________________
 |
 | We've got a special place where to store vocabulary.
 | Partly because string are basically unaligned. 
 | More importantly: vocabulary is very likely to be
 | shared among many modules; especially when they
 | are linked together.
 |_____________________________________________________
*/

typedef struct { struct ScrVocabulary *Static; } ScrVocabulary;
extern ScrVocabulary ScrVocabularyNull;
struct ScrVocabulary {
	char *(*GetLabel)(ScrVocabulary *this,char *Label);
	char *(*InternalLabel)(ScrVocabulary *this,int Num);
	char *(*CheckLabel)(ScrVocabulary *this,char *Label);
};
ScrVocabulary *ScrVocabularyNew(void);
ScrVocabulary *SrcVocabularyJargon(ScrVocabulary *Language);

/*___________________________________________________________
 |
 | UnkBound array: 
 | Sequence of unknown number elt.
 | Should be packed at the end.
 |___________________________________________________________
*/

typedef struct { struct ScrUnkBound *Static;} ScrUnkBound;
struct ScrUnkBound {
	int (*Size)(ScrUnkBound *this);
	int (*Add)(ScrUnkBound *this,void *Elt);
	void *(*Get)(ScrUnkBound *this,int Idx);
	void *(*Pack)(ScrUnkBound *this); // returns actually a void *[]
};
ScrUnkBound *ScrOneShotUnkBound(void); // !! After packing, intermediate storage is freed !!

/*___________________________________________________________
 |
 | Catalog: Commonly used for creating variable elt number
 | structures, and then packing them in an array.
 | Indexed by integers and strings.
 |___________________________________________________________
*/
typedef struct { void *Data; } ScrCatalogEntry;
typedef struct { struct ScrCatalog *Static; } ScrCatalog;

struct ScrCatalog {
	int (*EltNb)(ScrCatalog *this);

	int (*SetLabel)(ScrCatalog *this,int Dst,char *Label); // if Dst<0, grows instead
	int (*GetNum)(ScrCatalog *this,char *Label);
	char *(*GetLabel)(ScrCatalog *this,int Num);

	int (*SetData)(ScrCatalog *this,int num,ScrCatalogEntry data);
	ScrCatalogEntry (*GetData)(ScrCatalog *this,int Num);

	ScrCatalogEntry (*Data)(ScrCatalog *this,char *Label);
	ScrCatalog *(*Pack)(ScrCatalog *this);
    ScrCatalogEntry *(*StripPack)(ScrCatalog *this,int *EntryNb); // Pack with label removal

	void (*Close)(ScrCatalog *this);
};
ScrCatalog *ScrCatalogNew(void *Void);

/*_______________________________________________________________
 |
 | Our strings.
 | For very long string, management by chunks might be advisable
 |_______________________________________________________________
*/

typedef struct {struct ScrString *Static;} ScrString;
extern ScrString ScrStringNull;
struct ScrString {
	int (*Size)(ScrString *this);
	void (*Copy)(ScrString *this,char *dst,int b,int e); /* as usual, b include, e not; terminal 0 not included */
};

ScrString *ScrStringDirect(char *txt);
ScrString *ScrStringCat(ScrString *a,ScrString *b);
ScrString *ScrStringDecimal(int a);

/*_______________________________________________________________
 |
 | Our arrays come in two flavours: fixed sized and dynamic sized.
 | They are associated with a 'default element', for the elements
 | that haven't been set yet or are out of boundaries.
 | It is expected that indexes start at 0 and end at boundaries-1.
 | As long as the default is set, taking an elt out of boundary
 | shouldn't cause an error.
 |_______________________________________________________________
*/


typedef struct {struct ScrArray *Static;} ScrArray;
ScrArray *ScrArrayNull(void *Default);
struct ScrArray {
	// min, max and EltNb are the measured boundarys, they are set by the elts that value is actually 
	// set in the array. For set sized array, min=0 and max=Size
	void (*Dim)(ScrArray *this,int *min,int *max,int *EltNb);
	void (*SetDefault)(ScrArray *this,ScrCatalogEntry Elt);
	void (*SetElt)(ScrArray *this,int num,ScrCatalogEntry Elt);
	ScrCatalogEntry (*GetDefault)(ScrArray *this);
	ScrCatalogEntry (*GetElt)(ScrArray *this,int num);
	int (*IsSet)(ScrArray *this,int num);
	void (*ForEach)(ScrArray *this,int (*NextElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos);
	ScrArray *(*Translate)(ScrArray *this,ScrCatalogEntry Default,
		ScrCatalogEntry (*TranslateElt)(int num,ScrCatalogEntry Elt,void *Clos),void *Clos);
};

ScrArray *ScrArrayNull(void *Default);
ScrArray *ScrOneEltArray(void *Default,void *Elt);  // useful as an alternative of Null array
ScrArray *ScrSetSizedArray(int Size,void *Default);
ScrArray *ScrUnknownSizedArray(void *Default);
ScrArray *ScrConcatArray(ScrArray *A,ScrArray *B);
ScrArray *ScrAlteredArray(ScrArray *A,ScrArray *B); // Array A is altering Array B.
ScrArray *ScrRegularArray(void *Default,int Size,void *(*Elt)(int idx,void *Clos),void *Clos);

#endif
