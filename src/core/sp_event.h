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

#ifndef _SPANEL_H_EVENT_
#define _SPANEL_H_EVENT_
#include <const_c.h>

struct sp_event_s {
    void            *data;
    unsigned         write:1;
    unsigned         accept:1;
    /* used to detect the stale events in kqueue, rtsig, and epoll */
    unsigned         instance:1;
    /*
     * the event was passed or would be passed to a kernel;
     * in aio mode - operation was posted.
     */
    unsigned         active:1;
    unsigned         disabled:1;
    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned         ready:1;
    unsigned         oneshot:1;
    /* aio operation is complete */
    unsigned         complete:1;
    unsigned         eof:1;
    unsigned         error:1;
    unsigned         timedout:1;
    unsigned         timer_set:1;
    unsigned         delayed:1;
    unsigned         read_discarded:1;
    unsigned         unexpected_eof:1;
    unsigned         deferred_accept:1;
    /* the pending eof reported by kqueue or in aio chain operation */
    unsigned         pending_eof:1;
    sp_event_handler_pt  handler;
    sp_uint_t       index;
    //sp__log_t       *log;
    //sp__rbtree_node_t   timer;
    unsigned         closed:1;
    /* to test on worker exit */
    unsigned         channel:1;
    unsigned         resolver:1;
    //sp__atomic_t    *lock;
    //sp__atomic_t    *own_lock;
    /* the links of the posted queue */
    //sp__event_t     *next;
    //sp__event_t    **prev;
};

typedef sp_event* sp_event_p;

#endif /*_SPANEL_H_EVENT_*/
