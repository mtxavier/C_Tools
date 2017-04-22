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

/*_______________________________________________________________
 |
 | This should be included  in Thd.h
 |
 | It is the Os dependent part.
 |
 | !!!! We rely heavily on an architecture dependant trick:
 | !!!! which is that any thread can lock, unlock and destroy a
 | !!!! mutex, even when it doesn't own it.
 |_______________________________________________________________
*/

#include <pthread.h>
#include <bits/local_lim.h>

typedef struct {
	ThdItf *Desc;
	pthread_t thread;
	struct {
	    ThdThread ThdThread;
	    pthread_mutex_t Block;
		int Blocked,Acked;
	} Parent,Child;
	int End,Dead;
} OsThread;

static inline void OsFuneral(OsThread *that) {
	if (!that->Dead) {
		that->Dead = (0==0);
		pthread_join(that->thread,NULL);
		pthread_mutex_unlock(&that->Parent.Block);
		pthread_mutex_unlock(&that->Child.Block);
		pthread_mutex_destroy(&that->Parent.Block);
		pthread_mutex_destroy(&that->Child.Block);
	}
}
static int OsParentWait(ThdThread *this) {
	ThisToThat(OsThread,Parent.ThdThread);
	if (!that->End) {
		that->Parent.Blocked = (0==0);
		if (that->Child.Blocked) {
			that->Child.Blocked = (0!=0);
			pthread_mutex_unlock(&that->Child.Block); 
		}
	    pthread_mutex_lock(&that->Parent.Block);
	} else {
		OsFuneral(that);
	}
	return (!that->Dead);
}
static int OsParentAck(ThdThread *this) {
	ThisToThat(OsThread,Parent.ThdThread);
	if (!that->End) {
		if (that->Child.Blocked) {
			that->Child.Blocked = (0!=0);
			pthread_mutex_unlock(&that->Child.Block); 
		} else {
			OsParentWait(this);
		}
	} else {
		OsFuneral(that);
	}
	return (!that->Dead);
}
static int OsChildWait(ThdThread *this) {
	ThisToThat(OsThread,Child.ThdThread);
	that->Child.Blocked = (0==0);
	if (that->Parent.Blocked) {
		that->Parent.Blocked = (0!=0);
		pthread_mutex_unlock(&that->Parent.Block); 
	}
	pthread_mutex_lock(&that->Child.Block);
	return (0==0);
}
static int OsChildAck(ThdThread *this) {
	ThisToThat(OsThread,Child.ThdThread);
	if (that->Parent.Blocked) {
		that->Parent.Blocked = (0!=0);
		pthread_mutex_unlock(&that->Parent.Block);
	} else {
		OsChildWait(this);
	}
	return (0==0);
}
static void *OsThreadMain(void *this) {
	OsThread *that;
	that = this;
	that->End = (0!=0);
	Call(that->Desc,Main,1(&that->Child.ThdThread));
	that->End = (0==0);
	if (that->Parent.Blocked) {
         that->Parent.Blocked = (0!=0);
	     pthread_mutex_unlock(&that->Parent.Block);
	}
	return NULL;
}

ThdThread *ThdThreadLaunch(ThdItf *Main,int StackSize) {
	OsThread *r;
	static struct ThdThread Parent = { OsParentWait,OsParentAck };
	static struct ThdThread Child = { OsChildWait,OsChildAck };
	rPush(r);
	r->Parent.ThdThread.Static = &Parent;
	r->Child.ThdThread.Static = &Child;
	r->Desc = Main;
	r->End = r->Dead = (0!=0);
	pthread_mutex_init(&r->Parent.Block,NULL);
	pthread_mutex_lock(&r->Parent.Block);
	pthread_mutex_init(&r->Child.Block,NULL);
	pthread_mutex_lock(&r->Child.Block);
	r->Child.Blocked = (0!=0);
	r->Parent.Blocked = (0==0);
	{
	    pthread_attr_t attr;
		if (StackSize<PTHREAD_STACK_MIN) StackSize = PTHREAD_STACK_MIN;
		pthread_attr_init(&attr);
	    pthread_attr_setstacksize(&attr,StackSize);
	    pthread_create(&r->thread,&attr,OsThreadMain,r);
		pthread_attr_destroy(&attr);
	}
	pthread_mutex_lock(&r->Parent.Block);
	return &r->Parent.ThdThread;
}

