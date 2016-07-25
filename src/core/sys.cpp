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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void runProgramRead(int, short, void *);

void runProgramReState(rp_data_p rp, int newstate){
	timeval tm;
	rp->state=newstate;
	
	evtimer_del(rp->c->ev);
	evtimer_set(rp->c->ev, runProgramRead, rp);
	
	tm.tv_sec = 0;
	tm.tv_usec = 10;	
	evtimer_add(rp->c->ev,&tm);
}

void runProgramRead(int fd, short evfd, void *arg)
{
	timeval tm;
	rp_data_p rp = (rp_data_p)arg;
	u_char* newdata;
	long dlen,len;
	//fprintf(stderr,"\nrunProgramRead(%d,%d,%p,%d)...\n",fd,evfd, arg,rp->state);
	switch(rp->state)
	{
	case 1: //ready to read
		if(rp->len > rp->size - REQBUFPART/2 || rp->buf==NULL){
			newdata = (u_char*)realloc( rp->buf , sizeof(u_char)*(rp->size + REQBUFPART + 1 ));
			if(newdata!=NULL){
				rp->pos = newdata+(rp->pos - rp->buf);
				rp->buf = newdata;
				rp->size += REQBUFPART + 1;
			}
		}
		dlen = rp->size - rp->len;
		len = read( rp->fd, rp->buf + rp->len, dlen );
		if (len < 0) {
			runProgramReState(rp,3);
		} else
		if (len == 0) {
			runProgramReState(rp,2); 
		}else{
			rp->len += len;
			event_add(rp->c->ev, rp->tm);
		}
		break;
	case 2: //succ read
		if(rp->f != NULL) 
			pclose(rp->f);
		close(rp->fd);
		
		evtimer_del(rp->c->ev);
		evtimer_set(rp->c->ev, rp->c->handler, rp->c);
		
		tm.tv_sec = 0;
		tm.tv_usec = 10;	
		evtimer_add(rp->c->ev,&tm);
		break;
	case 3: //error
		rp->result = -1;
		sp_next(rp->c, rp->c->handler);
		break;
	}
	
}

void runProgramFinish(rp_data_p rp)
{
	try{
		if(rp->buf != NULL) free(rp->buf);
		if(rp->tm  != NULL) free(rp->tm);
		free(rp);
	}catch(...){};
}

bool sysRunProgram(char * path, sp_conn_p c, bool ex)
{
	rp_data_p rp = (rp_data_p)malloc(sizeof(rp_data_t));
	if(rp==NULL) 
		return false;
		
	rp->c = c;
	if(ex){
		rp->f = popen(path,"r");
		if(rp->f==NULL) {
			perror("popen");
			free(rp);
			return false;
		}
		rp->fd = fileno(rp->f);
	}else{
		rp->f = NULL;
		rp->fd = open(path, O_RDONLY|O_NONBLOCK|O_NOATIME);
		if(rp->fd < 1){
			perror("open");
			free(rp);
			return false;
		}
	}
	c->rp = rp;
	rp->state = 1;
	rp->buf = NULL;
	rp->len = 0;
	rp->size = 0;
	
	event_del(c->ev);
	if(ex){
		rp->tm = NULL;
		event_set(c->ev, rp->fd, EV_READ, runProgramRead, rp);
		event_add(c->ev, NULL);
	}else{
		rp->tm = (timeval*)malloc(sizeof(timeval));
		memset(rp->tm,0,sizeof(timeval));
		evtimer_set(c->ev, runProgramRead, rp);
		
		rp->tm->tv_sec = 0;
		rp->tm->tv_usec = 10;	
		evtimer_add(c->ev,rp->tm);
	}
	return true;
}
