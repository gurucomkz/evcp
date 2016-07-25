/* 
   gurucomkz/evcp

   Copyright (C) 2012-2016 Sergey Shevchenko

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   Sergey Shevchenko sergey.aegis@gmail.com

*/

#include <const_c.h>

static bool 			TimerActive = false;
static pthread_mutex_t 	timerLock = PTHREAD_MUTEX_INITIALIZER,
						timerListLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_attr_t	timerThreadAttr;
static pthread_t		timerThread;

static std::vector<TimerEntry> TimerTODO;

static event * timerEvent;

int timerInit(bool killtimer){
	timeval tm;
	tm.tv_sec = 10;
	tm.tv_usec = 0;	
	if(!killtimer){
		if(TimerActive) return 0;
		rep("Activating TIMER Thread");
		
		timerEvent = (event*)malloc(sizeof(event));
		if(timerEvent==NULL) return 0;
		
		evtimer_set(timerEvent,timerEnvoke,NULL);

		evtimer_add(timerEvent,&tm);
		
		//create timer thread
		pthread_attr_init(&timerThreadAttr);
		pthread_attr_setdetachstate(&timerThreadAttr, PTHREAD_CREATE_DETACHED);
		//setup timer
		pthread_create(&timerThread,&timerThreadAttr,timerLoop,NULL);
		
		
		
		TimerActive = true;
		rep("TIMER activated");
	}else{
		if(!TimerActive) return 0;
		TimerActive = false;				//setting termination flag
		rep("Stopping TIMER...");
		evtimer_del(timerEvent);
		free(timerEvent);
		pthread_mutex_unlock(&timerLock);	// to allow thread to read flag
		rep("TIMER stopped");
	}
	return 1;
}
void timerEnvoke(int, short int, void*){
		timeval tm;
	tm.tv_sec = 10;
	tm.tv_usec = 0;	
	if(!TimerActive) return;
	pthread_mutex_unlock(&timerLock);
	evtimer_add(timerEvent,&tm);
}
void *timerLoop(void * a){
	while(1){		
		pthread_mutex_lock(&timerLock);
//		rep("TIMER: entered iteration");
		if(!TimerActive) {
			rep("TIMER: Not active -> terminating");
			return NULL;
		}
		pthread_mutex_lock(&timerLock);
		//do our job...
		for(long i = TimerTODO.size()-1; i>=0; i--){
			if(TimerTODO[i].Moment <= time(NULL)){
				//exec function
//				rep("TIMER: exec function");
				TimerTODO[i].Act(TimerTODO[i].Params);
//				rep("TIMER: i'm alive!!");
				if(TimerTODO[i].Freq)
					TimerTODO[i].Moment = time(NULL)+TimerTODO[i].Freq;
				else{
					if(TimerTODO[i].Params != NULL)
						free(TimerTODO[i].Params);
					TimerTODO.erase(TimerTODO.begin( ) + i);
				}
				
			}//else rep("TIMER: this one not ready yet");
		}
//		rep("TIMER: exiting iteration");
		pthread_mutex_unlock(&timerLock);		
	}
}
void timerSetAction(unsigned long offset,timerAction act, void* params=NULL, unsigned long freq=0){
	time_t tt;
	tt = time(NULL);
	pthread_mutex_lock(&timerLock);
	//do our job...	
	TimerEntry te;
	te.Act = act;
	te.Freq = freq;
	te.Moment = tt+offset;
	te.Params = params;
	TimerTODO.push_back(te);
	pthread_mutex_unlock(&timerLock);
}

