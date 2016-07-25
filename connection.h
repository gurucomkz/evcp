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

#ifndef _SPANEL_H_INCLUDED_
#define _SPANEL_H_INCLUDED_

#include <const_c.h>

#include <event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <map>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/evp.h>

class Cookie
{
public:
	std::string V/*alue*/,E/*expires*/,D/*omain*/,P/*Path*/;
	Cookie(){V=E=D=P="";}
	Cookie(std::string V0,std::string E0,std::string D0,std::string P0){V=V0;E=E0;D=D0;P=P0;}
	void Print(){fprintf(stderr,"=%s; domain=%s; path=%s; expires=%s\n",V.c_str(),D.c_str(),P.c_str(),E.c_str());}
};

typedef std::map<std::string,std::string> sp_strmap;
typedef sp_strmap * sp_strmap_p;
typedef std::map<std::string,Cookie> sp_cookiemap;
typedef sp_cookiemap * sp_cookiemap_p;

typedef struct sockaddr_in 	sp_sockaddr;
typedef struct hostent 		sp_hostent;
typedef void(*sp_handler_t) (int, short int, void*);
#define REQBUFPART	1024
#define	MAXREQBUFSZ	10000


#define sp_ssl_conn_t			SSL
#define sp_ssl_session_t       	SSL_SESSION
#define sp_ssl_conn_t          	SSL

#define sp_ssl_get_connection(ssl_conn) \
    SSL_get_ex_data(ssl_conn, ngx_ssl_connection_index)
	
typedef struct {
    sp_ssl_conn_t             *connection;

    sp_int_t                   last;
    u_char                  	*buf;


    unsigned                    handshaked:1;
    unsigned                    renegotiation:1;
    unsigned                    buffer:1;
    unsigned                    no_wait_shutdown:1;
    unsigned                    no_send_shutdown:1;
} sp_ssl_connection_t;

typedef sp_ssl_connection_t * sp_ssl_connection_p;

typedef struct {
    sp_uint_t        hash;
    sp_str_t         key;
    sp_str_t         value;
    sp_str_t         lowcase_key;
} sp_table_elt_t;

typedef sp_table_elt_t * sp_table_elt_p;
typedef struct {
    sp_uint_t                        status;
    char*                         	status_line;
    
    off_t                             content_length_n;
    time_t                            date_time;
    time_t                            last_modified_time;
} sp_headers_out_t;

struct sp_bind
{
	sp_sockaddr			saddr;
	sp_hostent*			hp;
	char* 				host;
	int 				port;
	bool				binded;
	bool				usable;
	int					fd;
	sp_handler_t		handler;
	event*				ev;
	event*				timeoutev;
	int					sock;
	int					evfd;
	bool 				secure;
};
typedef sp_bind* sp_bind_p;

typedef struct {
	sp_strmap_p						list;	
    sp_cookiemap_p                  cookies;

    sp_str_t                         server;
    off_t                             content_length_n;
    time_t                            keep_alive_n;

    unsigned                          connection_type:2;
    unsigned                          msie:1;
    unsigned                          msie4:1;
    unsigned                          msie6:1;
    unsigned                          opera:1;
    unsigned                          gecko:1;
    unsigned                          chrome:1;
    unsigned                          safari:1;
    unsigned                          konqueror:1;
} sp_headers_in_t;

struct sp_request_s{
	u_char* 			buf;
	u_char*			pos;
	u_char*			last;
	unsigned long 	size;
	unsigned long 	len;
	
	sp_headers_in_t	headers;
	int				proto;
	char*			host;
	u_long	 		state;	
	u_int			method;
	
	u_int 		lowcase_index;
	u_char                            lowcase_header[NGX_HTTP_LC_HEADER_LEN];
	sp_str_t						http_protocol;
	u_int                        	http_version;
	
	sp_strmap_p						get_vars;
	sp_strmap_p						post_vars;
	
	unsigned						http_state:4;
	
	/* URI with "/." and on Win32 with "//" */
    unsigned                          complex_uri:1;
	
	 /* URI with "%" */
    unsigned                          quoted_uri:1;

    /* URI with "+" */
    unsigned                          plus_in_uri:1;

    unsigned                          invalid_header:1;

    unsigned                          valid_location:1;
    unsigned                          valid_unparsed_uri:1;
    unsigned                          uri_changed:1;
    unsigned                          uri_changes:4;
	
	sp_str_t						request_line;
	sp_str_t						unparsed_uri;
	sp_str_t						method_name;
	sp_str_t						exten;
	sp_str_t						args;
	
	sp_str_t						uri;
	
	u_char                           *uri_start;
    u_char                           *uri_end;
    u_char                           *uri_ext;
    u_char                           *args_start;
    u_char                           *request_start;
    u_char                           *request_end;
    u_char                           *method_end;
    u_char                           *schema_start;
    u_char                           *schema_end;
    u_char                           *host_start;
    u_char                           *host_end;
    u_char                           *port_start;
    u_char                           *port_end;
    u_char                           *header_name_start;
    u_char                           *header_name_end;
    u_char                           *header_start;
    u_char                           *header_end;

    unsigned                          http_minor:16;
    unsigned                          http_major:16;
};
typedef sp_request* sp_request_p;

struct sp_responce{
	char* 				buf;
	int					status;
	unsigned long 		size;
	unsigned long 		len;
	unsigned long 		sent;
	char*				fsendpath;
	int					fsend;
	unsigned long		fsize;
	unsigned long		fread;
	bool 				needrefill;
	bool 				h_sent;
	sp_strmap_p 		h_out;
};
typedef sp_responce* sp_responce_p;


typedef sp_conn* sp_conn_p;

typedef ssize_t (*sp_recv_pt)(sp_conn_p c, u_char *buf, size_t size);
typedef ssize_t (*sp_send_pt)(sp_conn_p c, u_char *buf, size_t size);

struct sp_conn_s{
	u_long		 	id;
	int				sock;
	sockaddr_in *	peer;
	socklen_t 		peer_sz;
	char*			peeraddr;
	sp_bind_p		bind;
	int				tmp;
	int				fd;
	/*
	//moved to sys.h
	int				pfd;
	FILE*			pf;
	u_char*			pfdbuf;
	u_long			pfdbuflen;
	u_char*			pfdbufpos;
	u_long			pfdbufsize;*/
	rp_data_s*		rp;
	event*			ev;
	sp_handler_t	handler;
	int				hndstate;
	event*			timeoutev;
	sp_request_p	data;
	sp_responce_p	resp;
	sp_ssl_connection_p	bssl;
	sp_ssl_conn_t*	ssl;
	bool			secure;

	bool 			timedout;

	sp_session_p	sess;
	void*			hndinfo;
};


void sp_next(sp_conn_p c, sp_handler_t h, u_int sec = 0, u_int usec = 10);
void sp_next_state(sp_conn_p c, int newstate, u_int sec = 0, u_int usec = 10);
void sp_evreset(sp_conn_p c,int flags,int fd = 0,sp_handler_t h = NULL);

char sp_strncmp(u_char*, u_char *, long );
char sp_strncmp(u_char*, const char *, long );
char* ngx2spstrcpy(char*, void*, u_long );

sp_bind_p 	bind_pool_add(char*,int,bool);
bool 		bind_pool_delete(sp_bind_p);
bool 		bind_pool_unbind(sp_bind_p);
bool 		bind_pool_reuse(sp_bind_p );
bool 		bind_pool_clear();
int 		bind_one_std(sp_bind_p,bool);

void conn_cycle		();
void conn_cycle_break();
void conn_init 		(int, short, void *);
void conn_read 		(int, short, void *);
void conn_respond 	(int, short, void *);
void conn_close		(sp_conn_p);
bool conn_process	(sp_conn_p);

void sp_process_request (int, short, void *);
bool sp_close_request(sp_conn_p,int);
bool sp_finalize_request(sp_conn_p,int);

char* conn_stdresp(int);

bool conn_compile_resp(sp_conn_p);
void sp_finish_request(int, short, void *);
bool conn_fillresp(sp_conn_p, const char*);
bool conn_fillresp(sp_conn_p, char*);
bool conn_fillresp(sp_conn_p, char*, long);
void sp_read_http_values(sp_request_p, sp_strmap_p, u_char*, u_char*);
void sp_decode_cookies(sp_request_p);

void cookieSet(sp_conn_p, std::string, std::string);
std::string cookieGet(sp_conn_p, std::string);
void cookieDel(sp_conn_p, std::string);

#endif 

