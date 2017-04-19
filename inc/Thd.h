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

#ifndef _All_Thd_h_
#define _All_Thd_h_

typedef struct { struct ThdThread *Static; }  ThdThread;
struct ThdThread { 
	int (*Wait)(ThdThread *this);
	int (*Ack)(ThdThread *this);
};

typedef struct { struct ThdItf *Static;} ThdItf;
struct ThdItf { void (*Main)(ThdItf *this,ThdThread *Mngr); };


//
// Fibers offer no parallelism.
// Fiber switching is quiet fast. As there is no concurrent access to
// datas, all datas can be shared without sync mechanism.
// Using Fibers is for pure stylistic reason. There is very little performance 
// hit, and the resulting code might be much much more clearer.
// If you have trouble organising complex automatons, you should really
// give thoughts in using Fibers instead.
//
ThdThread *ThdFiberLaunch(ThdItf *Main,int StackSize);


// The same with Private env switching.
// This doesn't apply to Thread and Task. Due to their inherent asynchroneous
// behaviour, env stacks must be strictly separated.
//     * Thread: the Env variable belongs to the thread local segment
//     * Process: Proces do not share memory at all.
ThdThread *ThdEnvFiberLaunch(ThdItf *Main,int StackSize,int pGrowth,int rGrowth);

// Don't use this if you can avoid it. It uses the main stack to allocates
// fiber stacks. Somehow portable, but slow and sooo ugly.
// The MainStackSize is only used on the first call, to warrant your main task
// will still be able to run.
ThdThread *ThdNativeFiberLaunch(ThdItf *Main,int MainStackSize,int StackSize,int pGrowth,int rGrowth);

//
// Threads use your OS threading capacity. This might be true parallelism,
// with all entailed problems. Threading might increase your application
// performance (depending on your hardware threading capacity).
//
ThdThread *ThdThreadLaunch(ThdItf *Main,int StackSize);

//
// Tasks are used when you need to distribute your application components
// between potential remote platforms. As such, their main way of communication
// is via streaming and file sharing.
//
ThdThread *ThdProcessLaunch(ThdItf *Main);


#endif
