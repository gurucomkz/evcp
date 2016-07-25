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

/****************************************************
file:		const_c.h

Varios constants
****************************************************/

#ifndef __SP_CONSTS_INCLUDE__
#define __SP_CONSTS_INCLUDE__

#include <string>
#include <vector>
#include <map>
#include <pthread.h>

#define DOCROOT "/home/dev/cpp/docroot"  

#define SPANEL_VERSION		"0.2-0"
#define BLKSIZE				1024
#define BLKINMEG			1024
#define MYBUFSZ				10240
#define SMALLBUF			1024
#define HOSTLEN				256
#define	BACKLOG				1

#define NGX_SSL_BUFFER   1
#define NGX_SSL_CLIENT   2

#define NGX_SSL_BUFSIZE  16384

#define NGX_LOG_STDERR            0
#define NGX_LOG_EMERG             1
#define NGX_LOG_ALERT             2
#define NGX_LOG_CRIT              3
#define NGX_LOG_ERR               4
#define NGX_LOG_WARN              5
#define NGX_LOG_NOTICE            6
#define NGX_LOG_INFO              7
#define NGX_LOG_DEBUG             8

#define NGX_LOG_DEBUG_CORE        0x010
#define NGX_LOG_DEBUG_ALLOC       0x020
#define NGX_LOG_DEBUG_MUTEX       0x040
#define NGX_LOG_DEBUG_EVENT       0x080
#define NGX_LOG_DEBUG_HTTP        0x100
#define NGX_LOG_DEBUG_MAIL        0x200
#define NGX_LOG_DEBUG_MYSQL       0x400

typedef unsigned int sp_uint_t ;
typedef unsigned int sp_int_t ;
typedef unsigned int ngx_array_t ;
typedef unsigned long ngx_ulong_t ;
typedef unsigned long sp_ulong_t ;

typedef struct sp_conn_s  	sp_conn;
typedef struct sp_event_s  	sp_event;
typedef struct sp_request_s sp_request;
typedef struct rp_data_s  	rp_data_t;
typedef struct sp_session_s	sp_session;

typedef unsigned char u_char;
typedef struct {
    size_t      len;
    u_char     *data;
} sp_str_t;
typedef sp_str_t* sp_str_p;

typedef void (*sp_event_handler_pt)(sp_conn_s *c);
typedef void (*sp_connection_handler_pt)(sp_conn_s *c);



typedef int               ngx_err_t;
#define ngx_errno                  errno
#define ngx_socket_errno           errno
#define ngx_set_errno(err)         errno = err
#define ngx_set_socket_errno(err)  errno = err

#define NGX_HTTP_LC_HEADER_LEN             32

#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6

#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"


#define ngx_strncmp(s1, s2, n)  strncmp((const char *) s1, (const char *) s2, n)



#include <config.h>
#include <sp_timer.h>
#include <sp_xml.h>
#include <base64.h>
#include <mydb.h>
#include <sp_sysuctrl.h>
#include <splog.h>
#include <sp_session.h>
#include <sp_stats.h>
#include <sp_common.h>
#include <sp_firstrun.h>
#include <sp_event.h>
#include <connection.h>
#include <request.h>
#include <mgr.h>
#include <sys.h>
#include <sp_curl.h>
#include <sp_cronjobs.h>
#include <sp_urlchecker.h>

#endif /* __SP_CONSTS_INCLUDE__ */


