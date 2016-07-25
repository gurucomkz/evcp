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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <getopt.h>  
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <fcntl.h>

using namespace std;
static SSL_CTX*  sp_ssl_ctx;
static int  ngx_ssl_server_conf_index;
static int  ngx_ssl_session_cache_index;

static sp_bind_p * bind_pool = NULL;
static long bind_pool_len = 0;
static sp_strmap types_map;
static char *sp_client_errors[] = {

    /* NGX_HTTP_PARSE_INVALID_METHOD */
    "client sent invalid method",

    /* NGX_HTTP_PARSE_INVALID_REQUEST */
    "client sent invalid request",

    /* NGX_HTTP_PARSE_INVALID_09_METHOD */
    "client sent invalid method in HTTP/0.9 request"
};

static long  ngx_ssl_protocols[] = {
    SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1,
    SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1,
    SSL_OP_NO_SSLv2|SSL_OP_NO_TLSv1,
    SSL_OP_NO_TLSv1,
    SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3,
    SSL_OP_NO_SSLv3,
    SSL_OP_NO_SSLv2,
    0,
};

void conn_cycle(){

	event_dispatch();

}

void conn_cycle_break(){
	event_loopbreak();
}

void sp_next_state(sp_conn_p c, int newstate, u_int sec, u_int usec){
	c->hndstate=newstate;
	sp_next(c, c->handler, sec, usec);
}
void sp_next(sp_conn_p c, sp_handler_t h, u_int sec, u_int usec){
	timeval tm;
	
	evtimer_del(c->ev);
	evtimer_set(c->ev, h, c);
	
	tm.tv_sec = sec;
	tm.tv_usec = usec;	
	evtimer_add(c->ev,&tm);
}

void sp_evreset(sp_conn_p c,int flags,int fd,sp_handler_t h)
{
	if(h==NULL) h = c->handler;
	if(fd==0) fd = c->fd;
	event_del(c->ev);
	event_set(c->ev, fd, flags, h, c);
	event_add(c->ev, NULL);
}

void sp_timer_reset(sp_conn_p c, sp_handler_t h, u_int sec = 2, u_int usec = 0){
	timeval tm;
	
	if(c->timeoutev!=NULL) evtimer_del(c->timeoutev);
	evtimer_set(c->timeoutev, h, c);
	
	tm.tv_sec = sec;
	tm.tv_usec = usec;	
	evtimer_add(c->ev,&tm);
}

void conn_read (int fd, short evfd, void *arg){	
	u_char* newdata;
	sp_conn_p c = (sp_conn_p)arg;
	sp_request_p r = c->data;
	c->fd = fd;
	char	*s,*ss;
	long cn,len,dlen,n,err, minsparebuf;
	u_char* cc;
	char warn = 0;
	//prepare buffer
	minsparebuf = c->ssl != NULL?4096:REQBUFPART/2;
	s = (char*)malloc(SMALLBUF);
	ss = (char*)malloc(SMALLBUF);
	//fprintf(stderr, "conn_read called with fd: %d, event: %d, arg: %p\n", fd, evfd, arg);
	for(;;)
	{
	//	fprintf(stderr, "reader iteration\n");
		
		if(r->len > r->size - REQBUFPART/2)
		{
			newdata = (u_char*)realloc( r->buf , sizeof(u_char)*(r->size + REQBUFPART + 1 ));
			if(newdata!=NULL){
				r->pos = newdata+(r->pos - r->buf);
				r->buf = newdata;
				r->size += REQBUFPART + 1;
			}
		}
		dlen = r->size - r->len;
		if(c->ssl != NULL)
		{
			//do{
				fprintf (stderr, "\nWILL SSL_read\n");
				n = SSL_read (c->ssl, r->buf + r->len, minsparebuf);
				fprintf (stderr, "\nWAS SSL_read\n");
				err = SSL_get_error (c->ssl, n);
				if (err == SSL_ERROR_WANT_READ) {
					goto again;
				}
				if (err != SSL_ERROR_NONE) {
					if (err == SSL_ERROR_ZERO_RETURN){
						fprintf (stderr, "The TLS/SSL connection has been closed\n");
					}else{
						fprintf(stderr, "%s: bad SSL_read on %d: %s\n", __func__, fd, ERR_error_string(ERR_get_error(), NULL));
					}
					goto finish;
				}
				
			// }while(SSL_pending(c->ssl)>0);
			len = n;
		}else{
			len = read( fd, r->buf + r->len, dlen );
		
			
			if (len == -1) {
				perror("read");
				goto finish;
			} else
			if (len == 0) {
				fprintf(stderr, "Remote Connection closed\n");
				goto finish;
			}
		}
	//	fprintf(stderr,"PASSED READER\n");
		r->len += len;
		
		r->buf[r->len] = 0;
		
		r->last = r->buf + r->len;
		
		if(!r->request_end) {
			cn = sp_parse_request_line(r);
			
			if(cn == NGX_AGAIN) goto again;
			
			
			if(cn != NGX_OK) {
				c->resp->status = NGX_HTTP_BAD_REQUEST;
				goto procnrespond;
			}
			
			r->headers.list = new sp_strmap;
			
			r->request_line.len = r->request_end - r->request_start;
			r->request_line.data = r->request_start;
			ngx2spstrcpy(s,r->request_line.data,r->request_line.len);

			if (r->args_start) 
				r->uri.len = r->args_start - 1 - r->uri_start;
			 else 
				r->uri.len = r->uri_end - r->uri_start;
			
			r->uri.data = r->uri_start;
				
				
			if(r->complex_uri){
				warn = 0;
				for(cc = r->uri_start; cc < r->uri_end; cc++){
					switch(*cc){
						case '/':
						case '.': warn++; break;
						case '\\':break;
						default: warn=0; break;
					}
					if( warn > 2 ){
						c->resp->status = NGX_HTTP_BAD_REQUEST;
						goto procnrespond;
					}
				}
			}
			
			r->unparsed_uri.len = r->uri_end - r->uri_start;
			r->unparsed_uri.data = r->uri_start;
						
			r->method_name.len = r->method_end - r->request_start + 1;
			r->method_name.data = r->request_line.data;

			if (r->http_protocol.data) 
				r->http_protocol.len = r->request_end - r->http_protocol.data;

			if (r->uri_ext) 
			{
				if (r->args_start) 
					r->exten.len = r->args_start - 1 - r->uri_ext;
				else 
					r->exten.len = r->uri_end - r->uri_ext;
				
				r->exten.data = r->uri_ext;
			}

			
			if (r->args_start && r->uri_end > r->args_start) {
				r->args.len = r->uri_end - r->args_start;
				r->args.data = r->args_start;
			
				r->get_vars = new sp_strmap;
				sp_read_http_values(r, r->get_vars, r->args.data, r->args.data + r->args.len);
			}
			else
				r->get_vars = NULL;
		}
		
		for(;;)
		{
			cn = sp_parse_header_line(r,1);
			//fprintf(stderr, "sp_parse_header_line= %d; length=%d\n",cn,r->header_name_end - r->header_name_start);
			if(cn == NGX_HTTP_PARSE_HEADER_DONE) break;
			if(cn == NGX_AGAIN) goto again;
		
			if(cn != NGX_HTTP_PARSE_INVALID_HEADER ){
				if(	r->header_name_end - r->header_name_start < SMALLBUF &&
					r->header_end - r->header_start < SMALLBUF)
				{
					memcpy(s,r->header_name_start, r->header_name_end - r->header_name_start);
					s[ r->header_name_end - r->header_name_start ] = 0;
					
					memcpy(ss,r->header_start, r->header_end - r->header_start);
					ss[ r->header_end - r->header_start ] = 0;
					//fprintf( stderr, "HEADER(%s)=\"%s\"\n", s,ss );
					(*(r->headers.list))[s] = ss; 
					//fprintf( stderr, "HEADER SAVED\n", s,ss );
				}//else
					//fprintf( stderr, "HEADER DISCARDED\n", s,ss );
					
			}
			
		}
		
		if(r->method == NGX_HTTP_POST)
		{
			//seek for post data
			for(;r->header_end < r->last;r->header_end++)
				if(r->header_end[0]!='\n' && r->header_end[0] != '\r') break;
			//and parse post_data
			r->post_vars = new sp_strmap;
			sp_read_http_values(r, r->post_vars, r->header_end, r->last);
		}
		//decode cookies
		sp_decode_cookies(r);
		goto procnrespond;
	}
again:
	event_add(c->ev, NULL);
	free(s); 	free(ss);
	return;
procnrespond:	
	event_del(c->ev);
	
	if(c->resp->status < 299)
	{
		sp_next(c, sp_process_request);
	}else{
		sp_next(c, sp_finish_request);
	}

	free(s); 	free(ss);
	return;
finish:
	//fprintf(stderr,"conn_read FINISH\n");
	free(s); 	free(ss);
	conn_close(c);
}

void sp_decode_cookies(sp_request_p r)
{
	sp_strmap::iterator it;
	enum {
		start = 0,
		name,
		param,
		equal,
		pequal,
		semicolon,
		psemicolon,
		space,
		value,
		pvalue,
		fin,
		err
	} state;
	std::string K = "", V = "", N = "", E = "", P = "", D = "", *O;
	const char * p;
	char c;
	
	r->headers.cookies = new sp_cookiemap();
	for(it = r->headers.list->begin(); it!=r->headers.list->end(); it++){
		if(it->first!="Cookie") continue;
		state = start;
		for(p = it->second.c_str(); p < it->second.c_str()+it->second.length(); p++)
		{
			c = *p;
		//	fprintf(stderr,"Cookie read '%c',K='%s',N='%s',V='%s',E='%s',D='%s',P='%s'\n",c,K.c_str(),N.c_str(),V.c_str(),E.c_str(),D.c_str(),P.c_str());
			switch(state)
			{
			case start:
				K = V = N = E = P = D = "";
				state = name;
			case name:
				switch(c){
					case '=': state = equal; break;
					case ';': state = err; return;
					default:  N += c; break;
				}
				break;
			case value:
				switch(c){
					case ';': state = semicolon; break;
					default: V += c; break;
				}
			break;
			case equal:
				switch(c){
					case ';': state = err; break;
					default: V = c; state = value;
				}
			break;
			case semicolon:
				switch(c){
					case ';': break;
					case '=': state = equal; break;
					default: K = ""; state = param;
				}
			break;
			case param:
				switch(c){
					case '=': 
						state = pequal;
						if(K == "expires") 	O = &E; else
						if(K == "path") 	O = &P; else
						if(K == "domain") 	O = &D; else
						{
						//it is new cookie description
							state = equal; N = K; V = "";
						}
						break;
					case ';': state = err; return;
					default:  K += c; break;
				}
				break;
			case pequal:
				switch(c){
					case ';': state = err; break;
					default: *O = c; state = pvalue;
				}
			break;
			case pvalue:
				switch(c){
					case ';': state = psemicolon; break;
					default: *O += c; break;
				}
			break;
			case psemicolon:
				switch(c){
					case ';': break;
					case '=': state = pequal; break;
					default: K = ""; state = param;
				}
			break;
			case err:	//jump out
				p+=it->second.length(); break;
			case fin:
				Cookie CC(V,E,D,P);
				(*r->headers.cookies)[N] = CC;
				state = start;
			break;
			}
		}
		if(N!="" && V!=""){
			Cookie NC(V,E,D,P);
			(*r->headers.cookies)[N] = NC;
		}
	}
	//fprintf(stderr,"sp_decode_cookies line %d\n",__LINE__);

	sp_cookiemap::iterator cit;
	for(cit = r->headers.cookies->begin();cit != r->headers.cookies->end(); cit++){
		fprintf(stderr,"Extracted cookie %s",(*cit).first.c_str());
		(*cit).second.Print();
	}
}

char sp_strncmp(u_char* s1, u_char *s2, long l)
{
	for(l-=1;l>=0;l--)
		if(s1[l]!=s2[l]) return 1;
	return 0;
}

char sp_strncmp(u_char* s1, const char *s2, long l)
{
	for(l-=1;l>=0;l--)
		if(s1[l]!=s2[l]) 
			return 1;
	return 0;
}


void sp_finish_request(int fd, short evfd, void *arg){
	sp_conn_p c = (sp_conn_p)arg;
	sp_responce_p r = c->resp;
	event_del(c->ev);
	
	
	//fprintf(stderr,"sp_finish_request(%d,%d,%p) STATUS=%d \n",fd, evfd, arg,r->status);
	conn_compile_resp(c);	
	
	//fprintf(stderr,"sending to conn_respond...\n");
	c->handler = conn_respond;
	event_set(c->ev, c->fd, EV_WRITE, c->handler, c);
	event_add(c->ev, NULL);
}

void sp_responce_senfile(int fd, short evfd, void *arg){
	sp_conn_p c = (sp_conn_p)arg;
	sp_responce_p r = c->resp;
	long s, dlen;
	bool drain = false;
	conn_compile_resp(c);
	
	r->needrefill = true;
	//event_del(c->ev);
	//fprintf(stderr,"sp_responce_senfile(%d,%d,%p)\n",fd, evfd, arg);
	for(;;){
		dlen = r->size - r->len;
		if(dlen > r->fsize - r->fread) 
			dlen = r->fsize - r->fread; //do not request more than needed
		s = read( c->resp->fsend, r->buf + r->len, dlen );
		if(s < 0){
			//we got error
			//fprintf(stderr, "sp_responce_senfile -- wrong file descriptor\n",s);
			conn_close(c);
			return;
		}
		if(s > 0){
			r->fread += s;
			r->len += s;
		}/*else 
		if(s == 0 ){
			if(r->fread < r->fsize){
				fprintf(stderr, "sp_responce_senfile -- didn't read all the file\n");
				sp_next(c,c->handler);
			}else{
				fprintf(stderr, "sp_responce_senfile -- no more data from file\n");
				close(c->resp->fsend);
				event_del(c->ev);
				conn_close(c);
				return;				
			}
		}*/
		//fprintf(stderr, "sp_responce_senfile -- read %d bytes (total %d of %d)\n",s, r->fread , r->fsize);
		if(r->fread == r->fsize) {
			r->needrefill = false;
		//	fprintf(stderr, "sp_responce_senfile -- no more data from file\n");
			event_del(c->ev);
			drain = true;
		}
		if(r->size == r->len) drain = true;
		if(drain){
		//	fprintf(stderr, "will drain buffer\n");
			
			event_set(c->ev, c->fd, EV_WRITE, conn_respond, c);
			event_add(c->ev, NULL); 
			return;
		}
	}
	
}

void fill_types_map(){
	types_map["html"] = "text/html";
	types_map["htm"] = "text/html";
	types_map["shtml"] = "text/html";
	types_map["css"] = "text/css";
	types_map["xml"] = "text/xml";
	types_map["gif"] = "image/gif";
	types_map["jpeg"] = "image/jpeg";
	types_map["jpg"] = "image/jpeg";
	types_map["js"] = "application/x-javascript";
	types_map["atom"] = "application/atom+xml";
	types_map["rss"] = "application/rss+xml";

	types_map["mml"] = "text/mathml";
	types_map["txt"] = "text/plain";
	types_map["jad"] = "text/vnd.sun.j2me.app-descriptor";
	types_map["wml"] = "text/vnd.wap.wml";
	types_map["htc"] = "text/x-component";

	types_map["png"] = "image/png";
	types_map["tiff"] = "image/tiff";
	types_map["tif"] = "image/tiff";
	types_map["wbmp"] = "image/vnd.wap.wbmp";
	types_map["ico"] = "image/x-icon";
	types_map["jng"] = "image/x-jng";
	types_map["bmp"] = "image/x-ms-bmp";
	types_map["svg"] = "image/svg+xml";

	types_map["war"] = "application/java-archive";
	types_map["ear"] = "application/java-archive";
	types_map["jar"] = "application/java-archive";
	types_map["hqx"] = "application/mac-binhex40";
	types_map["doc"] = "application/msword";
	types_map["pdf"] = "application/pdf";
	types_map["ps"] = "application/postscript";
	types_map["eps"] = "application/postscript";
	types_map["ai"] = "application/postscript";
	types_map["rtf"] = "application/rtf";
	types_map["xls"] = "application/vnd.ms-excel";
	types_map["ppt"] = "application/vnd.ms-powerpoint";
	types_map["wmlc"] = "application/vnd.wap.wmlc";
	types_map["xhtml"] = "application/vnd.wap.xhtml+xml";
	types_map["kml"] = "application/vnd.google-earth.kml+xml";
	types_map["kmz"] = "application/vnd.google-earth.kmz";
	types_map["cco"] = "application/x-cocoa";
	types_map["jardiff"] = "application/x-java-archive-diff";
	types_map["jnlp"] = "application/x-java-jnlp-file";
	types_map["run"] = "application/x-makeself";
	types_map["pm"] = "application/x-perl";
	types_map["pl"] = "application/x-perl";
	types_map["pdb"] = "application/x-pilot";
	types_map["prc"] = "application/x-pilot";
	types_map["rar"] = "application/x-rar-compressed";
	types_map["rpm"] = "application/x-redhat-package-manager";
	types_map["sea"] = "application/x-sea";
	types_map["swf"] = "application/x-shockwave-flash";
	types_map["sit"] = "application/x-stuffit";
	types_map["tk"] = "application/x-tcl";
	types_map["tcl"] = "application/x-tcl";
	types_map["der"] = "application/x-x509-ca-cert";
	types_map["pem"] = "application/x-x509-ca-cert";
	types_map["crt"] = "application/x-x509-ca-cert";
	types_map["xpi"] = "application/x-xpinstall";
	types_map["zip"] = "application/zip";

	types_map["bin"] = "application/octet-stream";
	types_map["exe"] = "application/octet-stream";
	types_map["dll"] = "application/octet-stream";
	types_map["deb"] = "application/octet-stream";
	types_map["dmg"] = "application/octet-stream";
	types_map["eot"] = "application/octet-stream";
	types_map["iso"] = "application/octet-stream";
	types_map["img"] = "application/octet-stream";
	types_map["msi"] = "application/octet-stream";
	types_map["msp"] = "application/octet-stream";
	types_map["msm"] = "application/octet-stream";

	types_map["mid"] = "audio/midi";
	types_map["midi"] = "audio/midi";
	types_map["kar"] = "audio/midi";
	types_map["mp3"] = "audio/mpeg";
	types_map["ra"] = "audio/x-realaudio";

	types_map["3gpp"] = "video/3gpp";
	types_map["3gp"] = "video/3gpp";
	types_map["mpeg"] = "video/mpeg";
	types_map["mpg"] = "video/mpeg";
	types_map["mov"] = "video/quicktime";
	types_map["flv"] = "video/x-flv";
	types_map["mng"] = "video/x-mng";
	types_map["asx"] = "video/x-ms-asf";
	types_map["asf"] = "video/x-ms-asf";
	types_map["wmv"] = "video/x-ms-wmv";
	types_map["avi"] = "video/x-msvideo";
}

const char* getmime(sp_request_p r){
	if(!types_map.size()) fill_types_map();
	u_char* x;
	long dx;
	sp_strmap::iterator it;
	if(!r->uri_ext) goto default_type;

	for(it = types_map.begin(); it!=types_map.end(); it++){
		if(it->first.length() != r->exten.len) continue;
		if(!sp_strncmp(r->exten.data, it->first.c_str(), r->exten.len)) {
			return it->second.c_str();
		}
	}
	
	
default_type:
	return "text/html";
}

std::string get_etag(char* s, long a = 0, long b = 0, long c = 0, long d = 0){
	char r[60];
	long x=0;
	for(;*s!=0; s++)
		x+=*s;
	snprintf(r,60,"\"%d%d%d%d%d\"",x,a,b,c,d);
	return r;
}

std::string sp_get_filetime(time_t t){
	char r[60];
	char * d[7] = {"Sun","Mon","Tue","Wed","Thd","Fri","Sat"};
	char * m[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	tm tv;
	t-=6*3600; //FIXME: timezone
	localtime_r( &t, &tv );
				 //Sun, 06 Nov 1994 08:49:37 GMT",
	snprintf(r, 60, "%s, %.2d %s %d %.2d:%.2d:%.2d GMT", d[tv.tm_wday], tv.tm_mday , m[tv.tm_mon], 1900+tv.tm_year, tv.tm_hour,tv.tm_min,tv.tm_sec);
	return r;
}

void sp_process_request(int fd, short evfd, void *arg){
	sp_conn_p c = (sp_conn_p)arg;
	sp_request_p r = c->data;
	bool sendfull = true;
	
	//fprintf(stderr, "sp_process_request(%d,%d,%p)\n", fd, evfd, arg);
	event_del(c->ev);
	
	(*c->resp->h_out)["Server"] = "SKILLPANEL "SPANEL_VERSION;
	(*c->resp->h_out)["Connection"] = "close";
	(*c->resp->h_out)["Date"] = sp_get_filetime(time(NULL));
	
	c->handler = sp_finish_request;
	c->resp->status = 200;
	
	//check for command
	if(r->uri.len>4) 
	if(!sp_strncmp((u_char*)"/mgr/",r->uri.data,5)){
		switch(mgrProcess(c))
		{
			case 1:
				fprintf(stderr, "mgrProcess returned OK\n");
				return;
			case -1:
				fprintf(stderr, "mgrProcess AUTH FAILED\n");
				break;
			default:
				c->resp->status = NGX_HTTP_NOT_IMPLEMENTED;
		}
		sp_next(c,sp_finish_request);
		return;
	}
		
	//fprintf(stderr,"Will process static file. Alloc %d bytes\n", strlen(DOCROOT) + r->uri.len + 1);
	//else, check for static file
	c->resp->fsendpath = (char *) malloc(strlen(DOCROOT) + r->uri.len + 1);
	//fprintf(stderr,"URI realpath = %s\n");
	char* termc = stpncpy(stpcpy(c->resp->fsendpath, DOCROOT), (char*)r->uri.data, r->uri.len);
	
	termc[0] = 0;
	//fprintf(stderr,"URI realpath = %s\n",c->resp->fsendpath);
	bool file_exists = true;  
	struct stat st;  
	if(stat(c->resp->fsendpath, &st) == -1) {  
		file_exists = false;
		c->resp->status = NGX_HTTP_NOT_FOUND;
	}
	else {  
		if(S_ISDIR(st.st_mode)) {
			file_exists = false;
			c->resp->status = NGX_HTTP_NOT_ALLOWED;
		}
	}
	if(file_exists) {
		//determine file type
		(*c->resp->h_out)["Content-Type"] = getmime(r);
		char lnbuf[30]; snprintf( lnbuf, 20, "%d", st.st_size);
		(*c->resp->h_out)["Content-Length"] = lnbuf;
		(*c->resp->h_out)["ETag"] = get_etag(c->resp->fsendpath,st.st_size,st.st_mode,st.st_mtime,st.st_ino);
		(*c->resp->h_out)["Last-Modified"] = sp_get_filetime(st.st_mode);
		
		//determine caching params
	//	if( (*c->data->headers.list)["If-Modified-Since"] == (*c->resp->h_out)["Last-Modified"] ) senfull = false;
		//else
		if( (*c->data->headers.list)["If-None-Match"] == (*c->resp->h_out)["ETag"] ) sendfull = false;
		
		//send file
		if(sendfull){
		//	fprintf(stderr,"File exist. size is %d\n",st.st_size);
			c->resp->fsize = st.st_size;  
			c->resp->fread = 0;  
			c->handler = sp_responce_senfile;
			c->resp->fsend = open(c->resp->fsendpath, O_RDONLY, 0);
			if(c->resp->fsend < 1){
				c->handler = sp_finish_request;
				c->resp->status = NGX_HTTP_NOT_ALLOWED;
		//		fprintf(stderr,"Sending to sp_finish_request...\n");
				free(c->resp->fsendpath); 
				c->resp->fsendpath = NULL;
			}else
			if(c->resp->fsize != 0) {
		//		fprintf(stderr,"Sending to sp_responce_senfile...\n");
			}else
				c->handler = sp_finish_request;
		}else{
			c->handler = sp_finish_request;
			c->resp->status = NGX_HTTP_NOT_MODIFIED;
			free(c->resp->fsendpath); 
			c->resp->fsendpath = NULL;
		}
	}
	sp_next(c, c->handler);
}

char* ngx2spstrcpy(char* dest, void*buf, u_long len){
	memcpy(dest,buf, len);
	dest[ len ] = 0;
	return dest;
}

void 
sp_read_http_values(sp_request_p r,sp_strmap_p M, u_char* from, u_char* to){
	enum {
		start = 0,
		name,
		equal,
		amp,
		value,
		value_percent,
		value_percent2,
		fin
	} state;
	u_char * p;
	u_char c, h;
	
	std::string K = "",V = "";
	state = name;
	for(p = from; p < to; p++){
		c = *p;
		if(c == CR || c == LF || c == ' ') { fprintf(stderr, "SP_READ_HTTP_VALUES WARN at pos %d", p - from); break; }
		switch(state){
		case name:
			switch(c){
				case '=': state = equal; break;
				case '&': if(K != "") (*M)[K]=""; state = amp; break;
				case '+': K += " "; break;
				default:  K += c; break;
			}
			break;
		case value:
			switch(c){
				case '%': state = value_percent; break;
				case '&': (*M)[K] = V; K = ""; V = ""; state = amp; break;
				case '+': V += " "; break;
				default: V += c; break;
			}
			break;
		case value_percent:
			if(c>='0' && c<='9'){ h = (c-'0')*16;  state = value_percent2; }else
			if(c>='a' && c<='f'){ h = (c-'a'+10)*16;  state = value_percent2;}else
			if(c>='A' && c<='F'){ h = (c-'A'+10)*16;  state = value_percent2;}
			else{
				V += "%";
				switch(c){
					case '=': state = equal; break;
					case '&': (*M)[K] = V; K = ""; V = ""; state = amp; break;
					case '%': break; //repeat this
					default: V += c;
				}
			}
			break;
		
		case value_percent2:
			if(c>='0' && c<='9'){ h += (c-'0'); V+=h; state = value; }else
			if(c>='a' && c<='f'){ h += (c-'a'+10); V+=h; state = value; }else
			if(c>='A' && c<='F'){ h += (c-'A'+10); V+=h; state = value; }
			else{
				switch(c){
					case '=': state = equal; break;
					case '&': (*M)[K] = V; K = ""; V = ""; state = amp; break;
					case '%': state = value_percent; break; //repeat this
					default: V += c;
				}
			}
				
			break;
		case equal:
			switch(c){
				case '&': (*M)[K] = ""; state = name; K=""; break;
				case '%': state = value_percent; break;
				default: V = c; state = value;
			}
			break;
		case amp:
			switch(c){
				case '&': break;
				case '=': state = equal; break;
				default: K = c; state = name;
			}
			break;		
		
		}
	}
	
	if((state == value || state == name || state == equal )&& K!="")
		(*M)[K] = V;
	sp_strmap::iterator it;	
	for(it = M->begin();it != M->end(); it++)
		fprintf(stderr,"Extracted param [%s = %s]\n",(*it).first.c_str(),(*it).second.c_str());
}

char* conn_stdresp(int code){
	switch(code){
	case 505: return "HTTP/1.1 505 HTTP Version Not Supported\r\n";
	case 504: return "HTTP/1.1 504 Gateway Timeout\r\n";
	case 503: return "HTTP/1.1 503 Service Unavailable\r\n";
	case 502: return "HTTP/1.1 502 Bad Gateway\r\n";
	case 500: return "HTTP/1.1 500 Server Error\r\n";
	
	case 417: return "HTTP/1.1 417 Expectation Failed\r\n";
	case 416: return "HTTP/1.1 416 Requested Range Not Satisfiable\r\n";
	case 415: return "HTTP/1.1 415 Unsupported Media Type\r\n";
	case 414: return "HTTP/1.1 414 Request-URI Too Long\r\n";
	case 413: return "HTTP/1.1 413 Request Entity Too Large\r\n";
	case 412: return "HTTP/1.1 412 Precondition Failed\r\n";
	case 411: return "HTTP/1.1 411 Length Required\r\n";
	case 410: return "HTTP/1.1 410 Gone\r\n";
	case 409: return "HTTP/1.1 409 Conflict\r\n";
	case 408: return "HTTP/1.1 408 Request Timeout\r\n";
	case 406: return "HTTP/1.1 406 Not Acceptable\r\n";
	case 404: return "HTTP/1.1 404 Not Found\r\n";
	case 403: return "HTTP/1.1 403 Forbidden\r\n";
	case 401: return "HTTP/1.1 401 Unauthorized\r\n";
	case 400: return "HTTP/1.1 400 Bad Request\r\n";
	
	case 304: return "HTTP/1.1 304 Not Modified\r\n";
	case 302: return "HTTP/1.1 302 Found\r\n";
	
	case 200: return "HTTP/1.1 200 OK\r\n";
	default:  return "HTTP/1.1 501 Not Implemented\r\n";
	}
}

bool conn_compile_resp(sp_conn_p c){
	if(c->resp->h_sent) return true;
	sp_strmap::iterator i;
	//add encoding
	sp_strmap::iterator it = c->resp->h_out->find("Content-Type");
	if(it != c->resp->h_out->end()){
//fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
		it->second += "; charset=utf8";
	}
	
	conn_fillresp(c,conn_stdresp(c->resp->status));
//fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	for( i = c->resp->h_out->begin(); i != c->resp->h_out->end(); i++ ){
		conn_fillresp(c,i->first.c_str());
		conn_fillresp(c,": ");
		conn_fillresp(c,i->second.c_str());
		conn_fillresp(c,"\r\n");
	//	fprintf(stderr,"ADDHEADER: %s: %s\n",i->first.c_str(),i->second.c_str());
	}
	conn_fillresp(c,"\r\n");
	c->resp->h_sent = true;
}

bool conn_fillresp(sp_conn_p c, const char* rp){
	return conn_fillresp(c, (char*) rp, strlen(rp));
}
bool conn_fillresp(sp_conn_p c, char* rp){
	return conn_fillresp(c, rp, strlen(rp));
}

bool conn_fillresp(sp_conn_p c, char* rp, long l){
//fprintf(stderr,"%s line %d: %p %d...\n",__func__,__LINE__,rp, l);
	sp_responce_p r = c->resp;
	if(r->size < r->len + l){
	//fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
		char* newdata = (char*)realloc( r->buf , r->len + l + 1 );
		if(newdata!=NULL){
			//fprintf(stderr, "conn_fillresp: Expanding output buffer\n");
			r->buf = newdata;
			r->size = r->len + l + 1;
		}else return false;
	}
	memcpy(r->buf+r->len, rp, l);
	r->len+=l;
	return true;
}

void conn_respond(int fd, short evfd, void *arg){
	sp_conn_p c = (sp_conn_p)arg;
	sp_responce_p r = c->resp;
	long w,err;
	//fprintf(stderr, "conn_respond(%d,%d,%p): pos %d of %d\n", fd, evfd, arg, r->sent, r->len);
	if(r){
		for(;;){
			//more control over sending
			if(r->len == r->sent){
				if(r->needrefill){
					r->len = 0; r->sent = 0;
		//			fprintf(stderr, "conn_respond: requesting refill\n");
					//event_del(c->ev);
					sp_next(c,c->handler);
				}else{
					event_del(c->ev);
					conn_close(c);
				}
				return;
			}
			if(c->ssl!=NULL){
				w = SSL_write(c->ssl, r->buf , r->len - r->sent);
				err = SSL_get_error(c->ssl, w);
				if (err == SSL_ERROR_WANT_READ) {
				}
				else if (err == SSL_ERROR_WANT_WRITE) {
					fprintf(stderr,"SSL_write WANT_WRITE\n");
					event_add(c->ev,NULL);
					return;
				}
				else if (err != SSL_ERROR_NONE) {
					fprintf(stderr,"%s: bad SSL_write on %d: %d: %s\n",
					__func__, c->fd, err, ERR_error_string(ERR_get_error(), NULL));
					conn_close(c);
				} 
				else {
					r->sent += w;
					if (event_pending (c->ev, EV_READ|EV_WRITE|EV_TIMEOUT, NULL)) {
						printf("Write event pending after completion!\n");
						event_del(c->ev);
					}
				}
			}else{
				w = write( c->fd , r->buf , r->len - r->sent );
				if( w < 1 ){
					if(w<0) perror("write");
					conn_close(c);
					break;
				}else
				{
			//		fprintf(stderr, "conn_respond(%d,%d,%p): sent %d more bytes\n", c->fd, evfd, arg, w);
					r->sent += w;
					event_add(c->ev, NULL);
				}	
			}
		}
	}
}

bool sp_close_request(sp_conn_p c, int e){
}

bool sp_finalize_request(sp_conn_p c, int e){
	fprintf(stderr,"FIXME: sp_finalize_request\n");
	//conn_close(c);
}

void conn_close(sp_conn_p c){
	//fprintf(stderr, "conn_close: WILL close connection\n");
	
	close(c->fd);
	free(c->peer);
	event_del(c->ev);
	if(c->timeoutev != NULL){ event_del(c->timeoutev); free(c->timeoutev); c->timeoutev=NULL; }
	free(c->ev);
	free(c->peeraddr);
	if(c->ssl != NULL) SSL_shutdown( c->ssl );
	if(c->hndinfo != NULL) free( c->hndinfo );
	if(c->data->buf != NULL) free(c->data->buf);
	if(c->resp->buf != NULL) free(c->resp->buf);
	if(c->data->headers.list!=NULL) delete c->data->headers.list;
	if(c->data->headers.cookies!=NULL) delete c->data->headers.cookies;
	if(c->data->post_vars != NULL) delete c->data->post_vars;
	if(c->data->get_vars != NULL) delete c->data->get_vars;
	if(c->resp->fsendpath != NULL) free(c->resp->fsendpath);
	if(c->resp->h_out!=NULL) delete c->resp->h_out;
	if(c->data != NULL) free(c->data);
	if(c->resp != NULL) free(c->resp);
	free(c);
	fprintf(stderr, "conn_close: Connection closed\n");
}

void conn_init (int fd, short evfd, void *arg){
	
	u_long s,e;
	sp_bind_p b = (sp_bind_p)arg; 
	sp_conn_p c;
	sockaddr_in peertmp;
	socklen_t psz = sizeof(peertmp); //important
	
	fprintf(stderr, "conn_init called with fd: %d, event: %d, arg: %p\n",fd, evfd, arg);
	
	event_add(b->ev, NULL);
	memset(&peertmp,0,sizeof(peertmp));
	s = accept(fd, (struct sockaddr*)&peertmp, &psz);
	if(s < 1) {
		perror("accept");
		return;
	}
	fprintf(stderr, "ACCEPTED\n");
	
	c = (sp_conn_p)malloc(sizeof(sp_conn));

	c->secure = b->secure;
	if(c->secure){
		c->ssl = SSL_new(sp_ssl_ctx);
		fprintf(stderr,"SSL_new RETURNED %p\n",c->ssl);
		if(c->ssl==NULL) return;
	}
	else 
		c->ssl = NULL;
		
	c->sock = s;
	
	if (c->ssl != NULL) 
    { 
		SSL_set_accept_state (c->ssl); 
		if(!SSL_set_fd(c->ssl, c->sock)){
			fprintf(stderr,"SSL_set_fd IS FALSE\n");
			free(c);
			return;
		}
		e = SSL_accept(c->ssl);
		if(e<1){
			fprintf(stderr,"SSL_accept RETURNED %d: ",e);
			fprintf(stderr,"SSL_accept RETURNED %d: ", (e = SSL_get_error(c->ssl,e)));
			switch(e){
				case SSL_ERROR_ZERO_RETURN: 	fprintf(stderr,"SSL_ERROR_ZERO_RETURN"); break;
				case SSL_ERROR_WANT_READ: 		fprintf(stderr,"SSL_ERROR_WANT_READ"); break;
				case SSL_ERROR_WANT_WRITE: 		fprintf(stderr,"SSL_ERROR_WANT_WRITE"); break;
				case SSL_ERROR_WANT_CONNECT: 	fprintf(stderr,"SSL_ERROR_WANT_CONNECT"); break;
				case SSL_ERROR_WANT_ACCEPT: 	fprintf(stderr,"SSL_ERROR_WANT_ACCEPT"); break;
				case SSL_ERROR_WANT_X509_LOOKUP:fprintf(stderr,"SSL_ERROR_WANT_X509_LOOKUP"); break;
				case SSL_ERROR_SYSCALL: 		fprintf(stderr,"SSL_ERROR_SYSCALL"); break;
				
				default: fprintf(stderr,"UNKNOWN"); break;
			}
			fprintf(stderr,"\n");
			SSL_shutdown( c->ssl );
			free(c);
			return;
		}
		fprintf(stderr,"\nSSL READY 1\n");
	}
	
	
	c->peer_sz = psz;
	c->peer = (sockaddr_in*) malloc(c->peer_sz);
	memcpy(c->peer, &peertmp, c->peer_sz);
	
	c->bind = b;
	c->handler = conn_read;
	
	c->peeraddr = (char*)malloc(INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &c->peer->sin_addr, c->peeraddr, INET_ADDRSTRLEN);
	
	fprintf(stderr,"\n\nPEER ADDR = %s\n", c->peeraddr); 
	
	//prepare input buffer
	c->data = (sp_request_p)malloc(sizeof(sp_request));
	memset(c->data,0,sizeof(sp_request));
	c->data->size = MYBUFSZ;
	c->data->buf = (u_char*)malloc(c->data->size);
	c->data->pos = c->data->buf;
	c->data->state = 0;
	c->data->len = 0;
	c->data->args_start = 0;
	c->data->uri_ext = 0;
	c->data->request_end = NULL;
	c->data->post_vars = NULL;
	c->data->get_vars = NULL;
	c->data->headers.list = NULL;
	c->data->headers.cookies = NULL;
	
	//prepare response buffer
	c->resp = (sp_responce_p)malloc(sizeof(sp_responce));
	c->resp->size = MYBUFSZ;
	c->resp->buf = (char*)malloc(c->resp->size);
	c->resp->len = 0;
	c->resp->sent = 0;
	c->resp->needrefill = false;
	c->resp->status = 200;
	c->resp->fsendpath = NULL;
	c->resp->h_out = new sp_strmap;
	c->resp->h_sent = false;

	c->rp = NULL;
	c->sess = NULL;
	c->hndinfo = NULL;
	//event handling
	c->timeoutev=NULL;
	c->ev = (event*)malloc(sizeof(event));
	event_set(c->ev, c->sock, EV_READ, c->handler, c);
	
	event_add(c->ev, NULL);

}

sp_bind_p bind_pool_add(char* addr,int port, bool use_ssl = false){
	int sock_id;
	sp_bind_p b;
	void* newbp;
	printf("bind_pool_add\n");
	sock_id = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_id==-1) {perror("socket");return NULL;}
	
	int yes = 1;
	
	
	if(sp_ssl_ctx == NULL && use_ssl == true)
	{
		SSL_load_error_strings(); 
		SSL_library_init();
		sp_ssl_ctx = SSL_CTX_new(SSLv23_method()); 
		if(sp_ssl_ctx!=NULL){
			SSL_CTX_set_default_verify_paths(sp_ssl_ctx); 
			//SSL_CTX_set_mode(sp_ssl_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER); 
		
			if (SSL_CTX_use_certificate_file (sp_ssl_ctx, "/etc/ssl/certs/spanel.pem", SSL_FILETYPE_PEM) == 1 
				&& SSL_CTX_use_PrivateKey_file (sp_ssl_ctx, "/etc/ssl/private/spanel.pem", SSL_FILETYPE_PEM) == 1 
				&& SSL_CTX_check_private_key (sp_ssl_ctx) == 1)
			{ 
			/* XXX - negotiate to only allow TLSv1 */ 
				fprintf(stderr,"\n\nSSL -- FALLBACK TO TLSv1 \n\n");
				SSL_CTX_set_options(sp_ssl_ctx, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3); 
				SSL_CTX_set_mode(sp_ssl_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER); 
			}
		}
	}
	
	//add entry into pool
	newbp=realloc(bind_pool,sizeof(sp_bind_p)*(bind_pool_len+1));
	if(newbp==NULL) return NULL;
	bind_pool=(sp_bind_p *)newbp;
	b = (sp_bind_p)malloc(sizeof(sp_bind));
	bind_pool[bind_pool_len++]=b;
	b->secure = use_ssl;
	//copy data
	b->sock = sock_id;
	b->port = port;
	printf(addr);
	if(addr==NULL) {printf("Addr supplied empty. using general\n"); addr = "0.0.0.0"; }
	printf("11 %d\n",strlen(addr));
	b->host = (char*)malloc(sizeof(char)*(strlen(addr)+1));
	memcpy(b->host,addr,strlen(addr)+1);
	printf(b->host);
	b->hp = gethostbyname(b->host);
	//prepare socket
	memset((void*)&(b->saddr),0,sizeof(b->saddr));
	memcpy(&(b->saddr.sin_addr), b->hp->h_addr, b->hp->h_length);
	b->saddr.sin_port = htons(b->port);
	b->saddr.sin_family	= AF_INET;
	//event handling
	b->ev = (event*)malloc(sizeof(event));
	//return NULL;
	//usability
	b->handler = conn_init;
	b->usable=true;
	b->binded=false;
	return b;
}

bool bind_pool_reuse(sp_bind_p b){

}

bool bind_pool_unbind(sp_bind_p b){
	if(b->binded && b->usable){
		try{
			shutdown(b->sock,SHUT_RDWR);
			close(b->sock);
			if(b->ev!=NULL){
				event_del(b->ev);
			}
			b->binded=false;
		}catch(...){return false;}
	}
	return true;
}
bool bind_pool_delete(sp_bind_p b){
	b->usable = false;
	if(b->ev!=NULL){ free(b->ev); b->ev = NULL; }
	free(b->host);
}

bool bind_pool_clear(){
	sp_bind_p b;
	for(; bind_pool_len>0; bind_pool_len--){
		b = bind_pool[bind_pool_len-1];
		if(b->usable){
			if(b->binded)
				bind_pool_unbind(b);
			if(bind_pool_delete(b)){
				
			}
		}
	}
	
	return true;
}

int bind_one_std(sp_bind_p b, bool use_ssl = false)
{
	if(!b->usable) return -1;
	if(b->handler==NULL) return -1;
	int bret=1,i=0;

	do{
		if(i) 
		{ 
			perror("bind");
			sleep(5);
		}
		bret=bind(b->sock, (const sockaddr*)&(b->saddr),sizeof(b->saddr));

		i++;	
	}while (bret && i<10);
	perror("bind");
	if (bret)return -1; 
	
	if(listen(b->sock,BACKLOG)) return -1;
	
	event_set(b->ev, b->sock, EV_READ, b->handler, b);
	event_add(b->ev,NULL);
	return b->sock;
}

//////////////////////////////////////////////////////////
///////////// 			COOKIES				//////////////
//////////////////////////////////////////////////////////

void
cookieSet(sp_conn_p c, std::string name, std::string sid)
{
	(*c->resp->h_out)["Set-Cookie"] = name+"="+sid;
	(*c->resp->h_out)["Set-Cookie"] += "; expires=0; path=/;";
//	if((*c->data->headers.list)["Host"]!="")
//		(*c->resp->h_out)["Set-Cookie"] += " domain="+(*c->data->headers.list)["Host"];
}

void
cookieDel(sp_conn_p c, std::string name)
{
	(*c->resp->h_out)["Set-Cookie"] = name+"=";
	(*c->resp->h_out)["Set-Cookie"] += "; expires=Mon, 01-Jan-2001 00:00:00 GMT; path=/;";
	if((*c->data->headers.list)["Host"]!="")
		(*c->resp->h_out)["Set-Cookie"] += " domain="+(*c->data->headers.list)["Host"];
}

std::string
cookieGet(sp_conn_p c, std::string name)
{
	fprintf(stderr,"LOOKUP FOR COOKIE %s returns \"%s\"",name.c_str(),(*c->data->headers.cookies)[name].V.c_str());
	return (*c->data->headers.cookies)[name].V;
}

//////////////////////////////////////////////////////////


