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

static bool sp_curl_inited = false;

void sp_curl_init()
{
	if(sp_curl_inited) return;
	curl_global_init(CURL_GLOBAL_ALL);
	sp_curl_inited = true;
}

size_t  sp_curl_callback(  void  *ptr,  size_t  size, size_t nmemb, void *arg){
	rp_data_p rp = (rp_data_p)arg;
	long len,addsz=size*nmemb;
	u_char*newdata;
	if(rp->len > rp->size - addsz || rp->buf==NULL){
		newdata = (u_char*)realloc( rp->buf , sizeof(u_char)*(rp->size + addsz + 1 ));
		if(newdata!=NULL){
			rp->pos = newdata+(rp->pos - rp->buf);
			rp->buf = newdata;
			rp->size += addsz + 1;
		}
	}
	memcpy(rp->pos,ptr,addsz);
	rp->pos+=addsz;
	rp->len+=addsz;
	rp->buf[rp->len] = 0;
	return addsz;
}

bool sp_curl_call(char * url, sp_conn_p c, bool ex){
	CURL* ch;
	sp_curl_init();
	rp_data_p rp = (rp_data_p)malloc(sizeof(rp_data_t));
	if(rp==NULL) 
		return false;
	
	rp->c = c;
	rp->state = 1;
	rp->buf = NULL;
	rp->len = 0;
	rp->size = 0;
	rp->tm = NULL;
	
	ch = curl_easy_init();
	curl_easy_setopt(ch, CURLOPT_URL, url);
	curl_easy_setopt(ch, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, sp_curl_callback);
	curl_easy_setopt(ch, CURLOPT_WRITEDATA, rp);
	curl_easy_setopt(ch, CURLOPT_POST, 0);
	curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(ch, CURLOPT_VERBOSE, 1);
	
	curl_easy_perform(ch);
	curl_easy_cleanup(ch);
	return true;
}

void sp_curl_cleanup(rp_data_p rp)
{
	try{
		if(rp->buf != NULL) free(rp->buf);
		if(rp->tm  != NULL) free(rp->tm);
		free(rp);
	}catch(...){};
}

