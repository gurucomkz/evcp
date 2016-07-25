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

#ifndef _SPANEL_SYS_INCLUDED_
#define _SPANEL_SYS_INCLUDED_

#include <const_c.h>

struct rp_data_s 
{
	sp_conn_s* 	c;
	FILE*		f;
	int			fd;
	u_char*		buf;
	u_long		len;
	u_char*		pos;
	u_long		size;
	int 		state;
	int			result;
	timeval*	tm;
};

typedef rp_data_s * rp_data_p;

bool sysRunProgram(char *, sp_conn_s*, bool ex = true );
#define runProgram sysRunProgram

#define sysGetFile(a,b) sysRunProgram(a,b,false)

//bool sysGetFile(char *, sp_conn_s* );

void runProgramFinish(rp_data_p);

#endif /*_SPANEL_SYS_INCLUDED_*/

