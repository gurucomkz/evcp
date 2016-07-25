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


pthread_mutex_t __pmt_init = PTHREAD_MUTEX_INITIALIZER;

LoggerClass::LoggerClass(){
	logfile = NULL;
	action_lock = __pmt_init;
}

LoggerClass::~LoggerClass(){
	fclose(logfile);	
}


//delete those logs that don't fit into a set of DEFAULT_LOGSNUM files
void DeleteOldLogs(){
	char * buf1,* c;
	FILE* f1;
	long logiter;

	logiter = 0;
	buf1 = (char*)malloc(MYBUFSZ);
	snprintf(buf1,MYBUFSZ,"ls -1Ar --color=none %s/log/*.log",ConfC("PATH_OWNROOT"));
	if(f1 = popen(buf1,"r")) {
		rep("LOGGER: Start deleting old logs");
		while(fgets(buf1,MYBUFSZ,f1))
		{
			if(++logiter <= ConfI("DEFAULT_LOGSNUM")) 
				continue;
			c = strstr(buf1,"\n");//delete tail
			if(c!=NULL) c[0]=0;
			rep(buf1);
			if(unlink(buf1)) rep("failed");
		}
		rep("LOGGER: Done deleting old logs");
		pclose(f1);
	}
	free(buf1);
}

void LoggerClass::Write(const char* st,unsigned long th_id) {
	tm tv;
	time_t tt;
	struct timeval tmv;
	static char dom = 0;
	char cdom, * fnameb,*replacebuf;
	long millisec, srclen, targetpos,xpos;
	bool delold = false;

	tt = time(NULL);
	localtime_r(&tt,&tv);
	cdom = tv.tm_mday;

	gettimeofday (&tmv, NULL);
	millisec = tmv.tv_usec / 1000;

	pthread_mutex_lock(&action_lock);

	if(cdom != dom && logfile != NULL) {	//if day has changed, close current log and start new one
		fprintf(logfile,"*END*\n");
		fflush(logfile);
		fclose(logfile);
		logfile=NULL;
		delold = true;
	}else
		delold = false;

	if(logfile==NULL) {
		fnameb = (char*)malloc(MYBUFSZ);
		snprintf(fnameb,300,"%s/log/%d-%.2d-%.2d.log",ConfC("PATH_OWNROOT"),1900+tv.tm_year,tv.tm_mon+1,tv.tm_mday);
		logfile = fopen(fnameb,"a");
		
		if(logfile == NULL) {
			perror(fnameb);
			pthread_mutex_unlock(&action_lock);
			return;
		}
		free(fnameb);
		fprintf(logfile,"\n");
		dom = cdom;
	}
//convert newlines to "\n" sequence
	srclen = strlen(st);
	if(srclen>SMALLBUF) srclen = SMALLBUF;
	replacebuf = (char*)malloc(srclen*2+1);
	targetpos = 0;
	for(xpos=0; xpos<srclen; xpos++){
		switch(st[xpos]){
		case '\n':
			replacebuf[targetpos++] = '\\';
			replacebuf[targetpos++] = 'n';
		break;
		case '\r':
			replacebuf[targetpos++] = '\\';
			replacebuf[targetpos++] = 'r';
		break;
		default:			
			replacebuf[targetpos++] = st[xpos];		
		}
	}
	replacebuf[targetpos]=0;

	if(th_id)
		fprintf(logfile,"%.2d:%.2d:%.2d.%.3d [ %.6d ] %s\n", 
			tv.tm_hour,tv.tm_min,tv.tm_sec,millisec, th_id, replacebuf);
	else
		fprintf(logfile,"%.2d:%.2d:%.2d.%.3d %s\n", 
			tv.tm_hour,tv.tm_min,tv.tm_sec,millisec, replacebuf);
	free(replacebuf);
	fflush(logfile);
	pthread_mutex_unlock(&action_lock);

	if(delold) 
		DeleteOldLogs();	
	//do this AFTER reopen. Otherwise action leads to eternal circuit
}

void LoggerClass::Write(const std::string& st,unsigned long th_id)
{	Write(st.c_str(),th_id); 	}

void LoggerClass::Write(const char* st)
{	Write(st,0);	}
void LoggerClass::Write(const std::string& st)
{	Write(st.c_str(),0); 	}

void LoggerClass::Reopen(){
	if(logfile != NULL){
		fclose(logfile);
		logfile = NULL;
	}
	Write("*** Log reopened!",0);
	DeleteOldLogs();
}

//	INSTANCE
static LoggerClass	Log;


void rep(const char* st,unsigned long th_id) 
{	Log.Write(st,th_id);	}

void rep(const char * st) 
{	rep(st, 0);	}

void rep(const std::string& st)
{	Log.Write(st.c_str(),0); 	}

void rep(const std::string& st,unsigned long th_id)
{	Log.Write(st.c_str(),th_id); 	}

void LogReopen()
{	Log.Reopen();	}

