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


/*

		stats module header
	module purpose: collect statistics information about 

*/
#include <map>
#include <string>
#include <vector>
#include <pthread.h>

//	ATT: Always update this value as soon as you change contents of 'StatTokens' enum
#define STAT_TOKEN_COUNT	8

enum StatTokens{
	ST_BytesSent,
	ST_BytesRecieved,
	ST_Requests,
	ST_Logins,
	ST_Uptime,
	ST_DBQueries,
	ST_LoginFailures,
	ST_Connections
};

typedef struct {
	long Value; 
	pthread_mutex_t Lock;
} StatEntry;

typedef std::map<StatTokens,StatEntry> StatTokenMap;
//#define	StatTokenMap	StatEntry[ STAT_TOKEN_COUNT + 1] 

class StatCollector{
	StatTokenMap STMap;
	long launchtime;
	long getFromMap(StatTokens);							//ok
	long getUptime();										//OK
public:
	StatCollector();										//OK
	std::map<StatTokens,std::string> __StatEntries;
	long Get(StatTokens);									//ok
	void Set(StatTokens, long);								//ok
	long Upd(StatTokens, long);								//ok
	std::string ListAll();
};


StatCollector& Stats();										//ok

//end
