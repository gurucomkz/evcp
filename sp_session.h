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

#ifndef __SPANEL_H_SESSION_
#define __SPANEL_H_SESSION_

#include <const_c.h>
#include <map>
#include <sys/socket.h>
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN		16
#endif
struct sp_session_s
{
	u_long 		id;
	char*	 	ip[INET_ADDRSTRLEN];
	time_t 		started;
	time_t 		lastuse;
	u_long 		user_id;
	u_long 		user_vn_id;
	char* 		user_login;
	bool 		user_root;
	bool 		user_primary;
	char 		user_lang[3];
	
	
};
typedef struct sp_session_s sp_session_t;
typedef sp_session_t * sp_session_p;

typedef std::map<u_long,sp_session_p> sp_sessmap_t;

void 		sessionRegister(sp_conn*, u_long, u_long, const char*, bool, bool, const char*);
bool		sessionCheck(sp_conn*);
void		sessionKill(u_long);
void		sessionKill(sp_conn*);

#endif /*__SPANEL_H_SESSION_*/

