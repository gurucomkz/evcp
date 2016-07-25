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

static sp_sessmap_t sessionMap;

void
sessionRegister(sp_conn* c, u_long uid, u_long vnid, const char* uname, bool, bool, const char*)
{
	sp_session * s = (sp_session *)malloc(sizeof(sp_session));
	char sid[SMALLBUF];
	//gen random id
	do{
		s->id = rand() + rand() * 100000;
	}while(sessionMap[s->id]!=NULL);
	sessionMap[s->id] = s;
	
	snprintf(sid,SMALLBUF,"%x",s->id); 
	fprintf(stderr,"\n\nREGISTERED SESSION #%d AS %s FOR IP %s\n",s->id,sid, c->peeraddr); 
	//copy account data
	s->user_id = uid;
	s->user_vn_id = vnid;
	
	memcpy(s->ip, c->peeraddr, INET_ADDRSTRLEN);
	
	s->user_login = (char*)malloc(strlen(uname));
	memcpy(s->user_login, uname, strlen(uname));
	s->started = s->lastuse = sp_time();
	cookieSet(c,"SPSID",sid);
	
	c->sess = s;
}

bool
sessionCheck(sp_conn* c)
{
	u_long i;
	const char * p;
	u_char cpos;
	bool f=false;
	u_long lsid = 0;
	std::string sid = cookieGet(c,"SPSID");
	if(sid == "") return false;
	sp_session_p s;
	//to hex
	if(sid.length()%2!=0) goto errfin;
	cpos = sid.length()/2;
	if(cpos>4) goto errfin; 
	
	for(p=sid.c_str();p<sid.c_str()+sid.length(); p++)
	{
		f=!f;
		if(*p>='0' && *p<='9'){ i = (*p-'0')*16; }else
		if(*p>='a' && *p<='f'){ i = (*p-'a'+10)*16; }else
		if(*p>='A' && *p<='F'){ i = (*p-'A'+10)*16; }else 
			goto errfin; //invalid char
		p++;
		if(*p>='0' && *p<='9'){ i += (*p-'0'); }else
		if(*p>='a' && *p<='f'){ i += (*p-'a'+10); }else
		if(*p>='A' && *p<='F'){ i += (*p-'A'+10); }else 
			goto errfin; //invalid char
		lsid += i << (--cpos*8);
	}
	//fprintf(stderr,"sessionCheck line %d\n",__LINE__);
	//fprintf(stderr,"\n\nsessionCheck found ID = %d\n\n",lsid);
	s = sessionMap[lsid];
	if(s == NULL) goto errfin;
	//fprintf(stderr,"sessionCheck line %d\n",__LINE__);
	s->lastuse = sp_time();
	c->sess = s;
	return true;
errfin:
	cookieDel(c,"SPSID");
	return false;
}

void sessionKill(sp_conn* c)
{
	sessionKill(c->sess->id);
}

void sessionKill(u_long sid)
{
	sessionMap.erase(sid);
}

