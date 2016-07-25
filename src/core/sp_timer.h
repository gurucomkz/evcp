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

#ifndef _SPANEL_H_TIMER_
#define _SPANEL_H_TIMER_

#include <const_c.h>

typedef void(*timerAction)(void*);

struct TimerEntry_s{
	timerAction Act;
	void* Params;
	unsigned long Freq;
	unsigned long Moment;
};

typedef struct TimerEntry_s TimerEntry;

int timerInit(bool killtimer = false);
void timerEnvoke(int, short int, void*);
void timerSetAction(unsigned long ,timerAction, void*, unsigned long);
void *timerLoop(void * );

#endif /*_SPANEL_H_TIMER_*/
