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

#ifndef _Schedule_h_
#define _Schedule_h_

/*_____________________________________________________________________________
 |
 | Schedule.h offers facilities to manage multiple objects 'actors' that act
 | independently.
 | Actors are introduced on 'Stages' that will direct the way actors are 
 | allowed to interact.
 |
 | 'Schedulers' are a type of stages. They order actors performance along a 
 | common timeline.
 | 
 |-----------------------------------------------------------------------------
 |
 | The most generic version of actors are simply object with a 'Perform()' 
 | method. An usual implementation of complex actors is via fibers or threads,
 | But it is in no way mandatory.
 |
 |-----------------------------------------------------------------------------
 |
 | 'RoundRobin' is a scheduler that allows actors to advance along a time axis 
 | in a round robin fashion. Each step 'RoundRobin' is given a deadline. It
 | will then order its actors, one at a time, to advance to the deadline.
 | Actors can wait others midway for communication purpose. In that case, 
 | 'RoundRobin' will advance all threads that lag behind up to the earliest 
 | break point. Before continuing.
 |
 | It is allowed for an actor to perform 0 ticks. It will be reinserted at the
 | end of the current date queue. This might give an opportunity for other 
 | actors stuck at the same date to act before the him. However, once all the 
 | eventual interdependencies are resolved, all actors must move on, or the
 | stage will be stuck at the date forever.
 | This practice is frowned upon. The prefered method to solve actors 
 | communication is to use messages and priority levels.
 |
 |-----------------------------------------------------------------------------
 |
 | Real time: Real time can only trully be achieved if the stage as a whole
 | consumes Ticks faster than their real time equivalent.
 | There are some levy that allow this to be 'on average'. Mainly, Input/Output
 | with the environment (real world) are only done within a certain periods at 
 | set dates. The system has only to be 'fast enough on average' between those
 | dates.
 | To do real time, we suppose that our hardware is good enough to achieve
 | real time. It means that at Input/Output (internal) date, we will have to
 | wait for the real time to catch up. This can be achieved 
 |     * Either outside of the stage.
 |     * Either by inserting an actor that has access to the hardware 
 | clock and will wait for it to catch-up on given dates. 
 | If you intend to be nice on your OS, waiting between synchronisation dates 
 | would be a good time to call a OS 'sleep()' or equivalent.
 | The longer the space between Input/Output the better levy the system will
 | have to achieve on average real time. It means that you should never try
 | to synching your internal computations with a real time clock until you are
 | actually waiting for an input/issuing an output.
 | 
 | Real times operations are often related to the interaction with a human 
 | being. Useful considerations:
 |    - Video: the brain seems to be able to analyze images at the rate of 
 | about 10 images/sec. There are caveats:
 |     * Analysing (fully recognising) an image that is not related to a 
 | previous image is MUCH slower. In the order of 1s, probably more. It is 
 | possible to catch glimpse of the content of an image in less time, but these 
 | are only glimpses. The test to perform for measuring this would be to watch
 | a slideshow of unrelated images and see at which rate they start to blend.
 |     * Fluant perception of movement might require a higher frame rate. My
 | gut feeling for an exact measurement of brain capacity in that regard would
 | be to study high level ping pong players. Their capacity at evaluating the 
 | ball trajectory in a short time frame would be what we're looking at. The 
 | data needed prior to moving for intercepting the ball would be: initial 
 | position, speed vector and spin. Those data are gathered by recording the ball 
 | position at given frames, which means at least 3 frames prior to acting.
 | The question is: at what distance from your adversary hand is the ball when
 | you know what you must do ? Between the time the ball hit the hand and the
 | time it reaches that point, your brain has perceived 3-4 frames. (It doesn't
 | mean that it has 'seen' 3-4 images ! Also, you will probably not see the
 | opponent hitting the ball, you will more proabably hear the event). 
 |     * Commonly advanced figures for 'fluant perception' are usually between
 | 24 to 85 images/sec. The real value is probably in the lower range (16-30 
 | frames/sec) with artifacts like motion blurs or motion lines.
 |    - Sound: the human audio perception seem to cover a range between 10Hz 
 | to 44khz. This set a upper limit on the synchronisation rate of an audio
 | thread: 44000 synchronisation/sec. However what a real time application 
 | might want to do would be to managing sound events. Due to the speed of sound, 
 | at 44000 event/s, a distance variation of a few cms from the audio source 
 | is enough to distort the result and reorder the events, obviously we want 
 | to aim for a lower value. A 100 event/s would be a more reasonable upper 
 | limit. EXCEPT that human brains seems to be specially expert at measuring
 | time intervals between sound events. It seems that musicians are able to
 | discern rythm variation (jiterring) with a precision ranging from 1-5 ms.
 | As a result, a 1ms syncing seems to be necessary for accurate sound management.
 |    - Input: What applies to sound, applies to input. While humans probably 
 | won't be able to perform more than a few 10th operations/second (if that much),
 | timing between those operations, especially if they are chained in sequence
 | have to be accurate with a 1ms precision. {Note: 1ms is about the time 
 | a nervous signal takes to move about 1cm, about the space betweent 2 fingers.
 | It sort of makes sense that such a precision is needed for operating a 
 | hand.}.
 |     * Syncing Input with video: delay between Input and output might be accurate
 | to the frame rate. However delay between various Inputs event should be
 | accurate at the ms. So, buffering input might be used, but time information 
 | must be part of the input.
 |     * Syncing Input with sound: checking articles on the MIDI standard should
 | convince you it has been thoroughly studied. A precision on the order of the
 | ms (2-5ms) seems to be required. 
 |_____________________________________________________________________________
*/

/*_____________________________________________________________________________
 |
 | Actors:
 |    Actors are given time slice (in tick number) in which they are allowed
 | to act. They mustn't go past the deadline (end of the time slice), but can
 | do less.
 |    When returning, they must give the time slice left.
 |_____________________________________________________________________________
*/

typedef struct { 
	struct SchActor *Static;
	char *Id;   /* Id is very useful for debugging purpose. 
				   You want to know in which object your program is stuck! */
} SchActor;
struct SchActor {
	int (*Perform)(SchActor *this,int Tick); /* Act for up to tick numbers. 
												Returns the tick numbers left */
};
extern SchActor SchActorNull;

/*___________________________________________________________________________
 |
 | StageName: Stage component
 |     When an actor enters the stage, he is given a stage name. He should
 | keep track of this stage name as it can be used to perform various stage 
 | operations. Stage Name are also publics, and they might be used to send
 | messages to other threads.
 |
 |--------------------------------------------------------------------------
 |
 | Priority management.
 | Actors that lag behind (date wise) will still be performed first, until 
 | their date matches a higher priority actor. 
 | When awakening a higher priority actor, it is the responsibility of the 
 | current actor to hang.
 |
 | For hardware emulation purpose, chipset interruption is simulated by 
 | awakening an actor of higher priority than the actor simulating the chipset.
 |
 |--------------------------------------------------------------------------
 |
 | Hanged and sleeping actors are stored in the 'sleeping' queue. It means that
 | they won't receive time slice event until they are reinserted in the normal 
 | queues.
 | The Sleep method move the actor in the sleeping queue and set an
 | event that will reinsert it in the normal queue when sleeping delay will 
 | expire.
 |
 | When a Sleeping actor is awakened; the scheduler will do as if it never had 
 | been slept.
 | That is: the delay during which the thread was asleep wont be substracted 
 | from the 
 | time sliced given... Awakened actors might use this to test if their sleeping
 | period had been interrupted (if the timesliced provided is lower than the 
 | sleeping delay, it has been interrupted.)
 |
 | The only difference between an asleep actor and an hanged actor is that the 
 | date origin of a hanged actor will be reset at the time of its awakening; 
 | while that of a sleeping thread will remain.
 |
 | Note also that Sleep, Hang, End are asynchronous (that is
 | they will take effect only after the 'Perform' method has ended).
 | For other actors:
 | Sleep and Hang won't remove pending event, so you will have to
 | do it manually with EventFlush, if needed. (Pending events might awaken 
 | the target before the sleep period has ended which might be undesirable
 | some times.)
 | Exception: Hanging a sleeping actor will remove the associated waking event.
 |___________________________________________________________________________
*/

typedef struct {
	SchActor SchActor;  /* The manager is bound to be managed by a higher 
						   up entity, possibly the root thread.*/
	struct SchStage *Static; 
} SchStage;

typedef struct {struct SchStageName *Static; } SchStageName;
struct SchStageName {
	SchStage *(*GetStage)(SchStageName *this);
	void (*AlterPriority)(SchStageName *this,SchStageName *ref,int offset);
	/* dates expressed as tick left to current deadline. */
    int (*GetDate)(SchStageName *this,int date); 
	/* date in ticks to the current deadline */
	void (*Sleep)(SchStageName *this,int date,int delay); 
	void (*Hang)(SchStageName *this);
	void (*End)(SchStageName *this);
	/* date in ticks to current deadline */
	void (*Awaken)(SchStageName *this,int date); 
	void (*FlushEvents)(SchStageName *this);
};

/*_____________________________________________________________________________
 |
 | SchEvent are simple actors. At the time of their activation, they perform 
 | a simple task (given by the 'Interpret' method of their tied message) and 
 | are then removed from the queue. 
 | They are also tied to a target actor, and are inserted with a priority 
 | directly superior to their target priority.
 |______________________________________________________________________________
*/

typedef struct {struct SchMsg *Static;} SchMsg;
struct SchMsg {
	void (*Interpret)(SchMsg *this,SchActor *Tgt);
	void (*Abort)(SchMsg *this);
};
typedef struct {struct SchEvent *Static; } SchEvent;
struct SchEvent {
	void (*Remove)(SchEvent *this);
};

struct SchStage {
	SchStageName *(*EnterActor)(SchStage *this,SchActor *Actor);
	SchEvent *(*InsertEvent)(SchStage *this,int Date,SchMsg *Msg,SchStageName *Tgt); // date is relative to the deadline
};
extern SchStage SchStageNull;

SchStage *SchRoundRobin();

/*_____________________________________________________________________________
 |
 | Specialisation: Fiber actors
 | This provides Actors specialisation as fibers actors.
 | Fibers have a main function and dedicated stack.
 |
 | The fiber actors main task receive SchStageName as a parameter. They must
 | use the 'GetDate(0)' method to get the allocated timeslice.
 | Each time they use the 'GetDate(...)' method, they hand the program flow to
 | the stage. The parameter given to the GetDate() method is 0 if they have
 | reached the current deadline, or what's left of the allocated timeslice since
 | they last used 'GetDate(...)'.
 |
 | To activate every actor each time a communication occurs might be 
 | ineffective.
 | That's why Threads that don't expect to interact with other for a long 
 | period should use 'Sleep()' instead of 'GetDate()'. 'Sleep()' move the actor
 | to the sleeping actors queue for a certain period of subjective time.
 | Actors lying on that queue won't be activated even if they lag behind other 
 | Actors timewise. If another actor needs information belonging to a sleeping 
 | actor, they should awaken him first before using 'GetDate()'.
 | Actors that don't expect to perform any action at all should use 'Hang()' 
 | instead.
 | Hanged actors are put on the sleeping actor queue, and won't move from it 
 | ever; unless they are explicitely awakened by another actor.
 |
 | Priority/Event/Interrupt:
 | Priority of a actor might be increased or decreased. It is expected that it
 | won't be changed dynamically.
 |
 | Priority gives constraint to timeslice allocation:
 |    (1) If a time slice would allow a lower priority actor to advance beyond
 | a higher priority actor, it will be truncated so the actor is stopped at the 
 | higher priority actor date.
 |    (2) However no timeslice will be allocated to Higher priority actor 
 | until all running lower priority task have reached its date.
 |
 | Event are one-shot actors attached to other actors. An event has a target 
 | actors, a customed message and an activation date. Once activated it launch 
 | the 'Interpret' method of the message and is then removed from the queues. 
 | An event might be aborted
 | before it occurs; either directly or because it's target has been removed...
 | In that case, the Abort method of the message will be invoked.
 | Usually Event are of higher priority than their actor, so they might 
 | interrupt them.
 |
 | The standard method of synchronising two actor, would be to issue a null 
 | event at the given time. The problem, is that the order in which the actor 
 | will get their time slice
 | after the interrupt is not known. It is therefore advised to attach a custom
 | message to the event; that will perform the operation before either thread 
 | continues.
 | The message attached to the event will be interpreted in the stage 
 | environment.
 | Usually, it is the responsability of the receiving actor to define the 
 | messages it will respond to.
 |_____________________________________________________________________________
*/

typedef struct { struct SchThread *Static; } SchThread;
struct SchThread { 
	void (*Main)(SchThread *this,SchStageName *Name); 
};

void SchFiberLaunch(
	SchStage *Stage,char *Label,SchThread *Main,
	int StackSize,int pGrowth,int rGrowth
);

/*___________________________________________________________
 |
 | Some generic messages.
 | Message creation/management might be tedious to implement.
 | Here are some templates.
 | A memory pool is reserved in the scheduler for their 
 | management.
 | All the datas that might be referenced by the various parameters 
 | should however be managed by the threads.
 | For instance make sure that they still exist when the 
 | message is interpreted...
 |___________________________________________________________
*/

/* Only present to allow timesync between threads; Not terribly useful. */
extern SchMsg SchNullMsg; 
/* The target of this message will be called with a 0 timesliced allocated.
   The other thread should be stuck at the interruption date (if they haven't 
   already gone beyond that point when the interruption was issued). So the 0 
   Timeslice should be used to recover data from other threads at this point 
   in time...  
*/
extern SchMsg SchMsgGoFirst; 
/* Useful message to access specific pieces of data */
SchMsg *IrqMsgCharPtr(SchStageName *t,void (*Interpret)(char *dst),char *dst);
SchMsg *IrqMsgChar(SchStageName *t,void (*Interpret)(char value),char Value);

SchMsg *IrqMsgIntPtr(SchStageName *t,void (*Interpret)(int *dst),int *dst);
SchMsg *IrqMsgInt(SchStageName *t,void (*Interpret)(int value),int Value);

SchMsg *IrqMsgPtrPtr(SchStageName *t,void (*Interpret)(void **dst),void **dst);
SchMsg *IrqMsgPtr(SchStageName *t,void (*Interpret)(void *Value),void *Value);

SchMsg *IrqMsgIdxCharPtr(SchStageName *t,void (*Interpret)(int Idx,char *dst),int Idx,char *dst);
SchMsg *IrqMsgIdxChar(SchStageName *t,void (*Interpret)(int Idx,char value),int Idx,char value);

SchMsg *IrqMsgIdxIntPtr(SchStageName *t,void (*Interpret)(int Idx,int *dst),int Idx,int *dst);
SchMsg *IrqMsgIdxInt(SchStageName *t,void (*Interpret)(int Idx,int Value),int Idx,int Value);

SchMsg *IrqMsgIdxPtrPtr(SchStageName *t,void (*Interpret)(int Idx,void **dst),int Idx,void **dst);
SchMsg *IrqMsgIdxPtr(SchStageName *t,void (*Interpret)(int Idx,void *Value),int Idx,void *Value);

SchMsg *IrqMsgSD(SchStageName *t,void (*Interpret)(void *dst,void *src),void *dst,void *src);
SchMsg *IrqMsgSDS(SchStageName *t,void (*Interpret)(void *dst,void *src,int siz),void *dst,void *src,int siz);

#endif

