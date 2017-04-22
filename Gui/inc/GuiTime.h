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

#ifndef _GuiTime_h_
#define _GuiTime_h_

/*_________________________________________________
 |
 | Front end for os dependant real time access
 | These should only be used as backend, never
 | directly by the application.
 |
 | You should use this if you are suspecting 
 | that your favourite Graph library handles
 | the real time badly (or that your os is better
 | at it).
 |_________________________________________________
*/

/*_____________________________________________
 |
 | These are not general purpose primitive
 | They should be used only for processes with
 | a time period ranging between 1/100s and
 | 10s.
 |_____________________________________________
*/

#define GuiTimPrec 1000000
#define GuiTimSWrapAround 180
#define GuiTimWrapAround (GuiTimSWrapAround*GuiTimPrec)
/* 
   Returns a value between 0 and WrapAround-1 in usec. 
   The application should expect WrapAround to occure (once about every 3 min for instance).
*/
int GuiTimeGetTime(void);
/* 
   Returns the actual date at wake up time. 
       * Behaviour is unspecified for delay above TimWrapAround/2. 
       * The os might suspend the task and restart it within the sleep period. 
	   It is possible (if unlikely) that many WrapAround period occur during that delay.
	   * In any case, it is required that you check the return date value. It will NEVER
	   be the time of suspension+delay.
*/
int GuiTimeSleep(int *WrapAround,int usec); 

/*
   When the system suspends the application and you are aware of it, you might want
   to use the following primitives to stop and start the internal clock.
   Both return the time at interruption point.
*/

int GuiTimePause(void);
int GuiTimeGo(void);

#endif
