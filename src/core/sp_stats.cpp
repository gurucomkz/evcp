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

#include <time.h>
#include <const_c.h>

static StatCollector __StatCollectorInstance;

StatCollector& Stats(){
	return __StatCollectorInstance;
}

std::string StatCollector::ListAll(){
	std::string ret = "";
	char * buf = (char*)malloc(SMALLBUF);

	for(long i=0; i< __StatEntries.size(); i++){
		snprintf(buf,SMALLBUF, "%s = %d; ", __StatEntries[(StatTokens)i].c_str(),Get((StatTokens)i));
		ret += buf;
	}
	free(buf);
	return ret;
}

StatCollector::StatCollector(){
	launchtime = (long)time(NULL);

	for(long i=0; i<= STAT_TOKEN_COUNT; i++){
		STMap[(StatTokens)i].Lock = inited_mutex;
		STMap[(StatTokens)i].Value = 0;
	}

	__StatEntries[ST_BytesSent] = "BytesSent";
	__StatEntries[ST_BytesRecieved] = "BytesRecieved";
	__StatEntries[ST_Requests] = "Requests";
	__StatEntries[ST_Logins] = "Logins";
	__StatEntries[ST_Uptime] = "Uptime";
	__StatEntries[ST_DBQueries] = "DBQueries";
	__StatEntries[ST_LoginFailures] = "LoginFailures";
	__StatEntries[ST_Connections] = "Connections";

}

void StatCollector::Set(StatTokens tok, long newval = 1){
	if(tok == ST_Uptime) return;
	pthread_mutex_lock(&(STMap[tok].Lock));
	STMap[tok].Value = newval;
	pthread_mutex_unlock(&(STMap[tok].Lock));
}

long StatCollector::Upd(StatTokens tok, long delta){
	long ret = 0;
	if(tok == ST_Uptime) return getUptime();
	pthread_mutex_lock(&(STMap[tok].Lock));
	STMap[tok].Value += delta;
	ret = STMap[tok].Value;
	pthread_mutex_unlock(&(STMap[tok].Lock));
	return ret;
}

long StatCollector::getFromMap(StatTokens tok){
	return STMap[tok].Value;
}

long StatCollector::Get(StatTokens tok){
	switch(tok){
	case ST_Uptime:
		return getUptime();
	default:
		return getFromMap(tok);
	}
}

long StatCollector::getUptime(){
	return (long)time(NULL)-launchtime;
}


//end
