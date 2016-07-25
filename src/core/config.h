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

#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>

#ifndef __CONFIG_DEF
#define __CONFIG_DEF
typedef std::map<std::string,std::string> confmap;

class ConfigClass{
	confmap data;
public:
	
	int ci(const char *);
	long cl(const char *);
	float cf(const char *);
	double cd(const char *);
	const char* cc(const char * );
	std::string cs(const char * );

	ConfigClass();
	void dump();
	int LoadFromFile(const char* );

};


int ConfI(const char *) ;
long ConfL(const char *);
float ConfF(const char *);
double ConfD(const char *);
const char* ConfC(const char * );
std::string ConfS(const char * );
int ConfLoad(const char *);



#endif
//
