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

#ifndef _SPANEL_H_LOG_
#define _SPANEL_H_LOG_

#include <const_c.h>

class LoggerClass{
	FILE* logfile;
	pthread_mutex_t	action_lock;
public:
	LoggerClass();
	~LoggerClass();

	void Write(const char* ,unsigned long );
	void Write(const std::string& ,unsigned long );
	void Write(const char* );
	void Write(const std::string& );
	void Reopen();
};

//define logger instance

void rep(const char* ,unsigned long );
void rep(const char * );
void rep(const std::string& );
void rep(const std::string&, unsigned long );
void LogReopen();


#endif
