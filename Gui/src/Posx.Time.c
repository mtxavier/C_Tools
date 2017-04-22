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

#include <time.h>
#include <GuiTime.h>

/*_____________________________________________________________
 |
 | You might need to include the rt library at the link stage.
 |_____________________________________________________________
*/

static struct {
	int org,pause;
} lTime;

#define UnMillion     1000000
#define UnMilliard 1000000000

int GuiTimeGetTime() {
	struct timespec t;
	int r,div,err;
	err = clock_gettime(CLOCK_REALTIME,&t);
	div = UnMilliard/GuiTimPrec;
    r = t.tv_nsec/div;
	r = r+((t.tv_sec%GuiTimSWrapAround)*GuiTimPrec);
	r = lTime.org+r;
	if (r>=GuiTimWrapAround) { r-= GuiTimWrapAround; }
	return r;
}
int GuiTimeSleep(int *WrapAround,int usec) {
	struct timespec delay,remain;
	delay.tv_nsec = (usec%UnMillion)*1000;
	delay.tv_sec = usec/UnMillion;
	nanosleep(&delay,&remain);
	*WrapAround = 0;
	return GuiTimeGetTime();
}
int GuiTimePause(void) {
	lTime.pause = GuiTimeGetTime();
	return lTime.pause;
}
int GuiTimeGo(void) {
	int start;
	start = GuiTimeGetTime();
	lTime.org = lTime.org + lTime.pause - start;
	if (lTime.org<0) lTime.org += GuiTimWrapAround;
	if (lTime.org>=GuiTimWrapAround) lTime.org -= GuiTimWrapAround;
	return lTime.pause;
}

