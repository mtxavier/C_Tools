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

#ifndef _Browser_h_
#define _Browser_h_

typedef struct { struct BrwPos *Static; } BrwPos;
struct BrwPos {
	BrwPos *(*Duplicate)(BrwPos *this);
	void (*Move)(BrwPos *this,int dMantisse,int dLg2exp);
	// For numerical value: value is stored with less significant bit first
	// anyway: the result gives the number of bytes left. It can be <0 if the buffer provided is too long.
	int (*PartVal)(BrwPos *this,void *b,void *e,int idx);
};

extern BrwPos BrwPosNull;

BrwPos *BrwPosInt(int oMantisse);
BrwPos *BrwPosInt32(unsigned int oMantisse,int oLg2exp);
BrwPos *BrwPosInt64(unsigned int oMantisse,int oLg2exp);

/*----------------*/

/*________________________________________________________________
 |
 | We try to keep Stream as simple as possible (only one method)
 | because it will probably be specialized a lot. However, a close
 | function is often mandatory for ressource cleaning purpose, so
 | we also provide a BrwStreamCtrl class.
 | The normal behaviour of a stream would be to autoclose as soon
 | as it has signaled the end of file. (there's no point keeping
 | ressource since we can't use them anyway).
 | Streams shouldn't rely on a call to 'Close' for cleaning 
 | ressources. The purpose of 'Close' is to clean under 
 | exceptional circumstances (read: error management or 
 | process interruption).
 |________________________________________________________________
*/

typedef struct { struct BrwStream *Static; } BrwStream;
typedef struct { struct BrwRSerial *Static; } BrwRSerial;
typedef struct { struct BrwXport *Static; } BrwXport;
typedef struct {
	BrwStream BrwStream;
	struct BrwStreamCtrl *Static; 
} BrwStreamCtrl;

struct BrwStreamCtrl {
	void (*Close)(BrwStreamCtrl *this);
};
struct BrwStream {
	void *(*Read)(BrwStream *this,void *b,void *e);
};
#define BrwPosSet 0
#define BrwPosCur 1
#define BrwPosEnd 2
struct BrwRSerial {
	BrwPos *(*GetPos)(BrwRSerial *this,int From); // By default, GetPos allocates a new pos on the rStack.
	void (*Seek)(BrwRSerial *this,BrwPos *Idx);
	void *(*Read)(BrwRSerial *this,void *b,void *e); // Returns pointer on end of data or BrwEof on a end of file.
};

struct BrwXport {
	void (*Xport)(BrwXport *this,void *b,void *e);
};

// There is no 'Close' method. The standard way of closing a file would be:
//  (1) Seek to end
//  (2) Read -> returns b. (that is 0 bytes read).
//  (3) Read -> Close the file and returns BrwEof.
//  When writing your own serial stream, be sure to stick to that behaviour and you will be
//  able to use:

void BrwRSerialClose(BrwRSerial *S);

extern BrwStream BrwStreamNull;
extern BrwStreamCtrl BrwStreamCtrlNull;
extern BrwRSerial BrwRSerialNull;
extern BrwXport BrwXportNull;
extern void *BrwEof;

/*-----------------*/

BrwStream *BrwSerialStream(BrwRSerial *s);

BrwStreamCtrl *BrwSerialUnziped(BrwStreamCtrl *Org);

BrwRSerial *BrwRSerialRaw(void *b,void *e);
BrwRSerial *BrwRSerialFile(char *Name);


/*-----------------*/


// Random files are files of int.
// SerialRandom and Serial Random1 gives result in the range [0..255]
unsigned int BrwRandomSeed(void);
BrwRSerial *BrwSerialRandom(unsigned int Seed);
BrwRSerial *BrwSerialRandom1(unsigned int Seed);

// Capped : results will be in the range [0..Sides-1];
// Rnd must give result in the range [0..RndCap-1];
// Uniform distribution should translate to uniform distribution, don't expect anything otherwise.
BrwRSerial *BrwCappedDice(int Sides,int RndCap,BrwRSerial *Rnd);

// Shuffled Dice: you must provide the result distribution. 
BrwRSerial *BrwShuffledDice(unsigned char *bDistr,unsigned char *eDistr,int RndCap,BrwRSerial *Rnd);

// With Mixed dices, you initiate reading buffer with sides nb.
BrwRSerial *BrwMixedDice(int RndCap,BrwRSerial *Rnd);

/*________________________________
 |
 |________________________________
*/

typedef struct { struct BrwType *Static; } BrwType;
typedef struct { struct BrwNod *Static; } BrwNod;
typedef struct { struct BrwNodEntries *Static; } BrwNodEntries;

struct BrwType {
	int (*IsLeaf)(BrwType *this);
	BrwNod *(*Fields)(BrwType *this);
};
struct BrwNodEntries {
	int (*Elt)(BrwNodEntries *this,BrwType *type,int idx,char *Label);
};
struct BrwNod {
	int (*EntriesNb)(BrwNod *this);
	void (*Browse)(BrwNod *this,BrwNodEntries *Tgt,int b,int e);
	BrwNod *(*Entry)(BrwNod *this,BrwType **type,char *Label);
	BrwRSerial *(*Leaf)(BrwNod *this,BrwType **type,char *Label);
};

extern BrwType BrwTypeNull;
extern BrwNod BrwNodNull;
BrwNod *BrwNewDir(char *Path);
BrwNod *BrwUntaredFile(BrwRSerial *Ar);

#endif
