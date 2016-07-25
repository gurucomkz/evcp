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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include <const_c.h>

#include <signal.h>

int pidf = 0;

void finalize(int);
void sig_act(int);

int main(int argc, char* argv[])
{
	sp_bind_p bStd=0, bSsl=0;
	char pidbuf[10], * strbuf;

	if(ConfLoad("/etc/spanel.conf")!=1){
		perror("ERROR! Failed to open config file. Maybe I'm not root?\n");
		exit(1);
	}

//check for pidfile 	
	pidf = open(ConfC("PATH_PIDFILE"),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(flock(pidf,LOCK_EX|LOCK_NB)){
		rep("ERROR: Pidfile is busy");
		exit(1);
	}
	sprintf(pidbuf,"%d",getpid());
	if(-1==write(pidf,pidbuf,strlen(pidbuf))) perror("write (pidf)");
	fsync(pidf);

	signal(SIGINT, sig_act);
	signal(SIGHUP, sig_act);
	signal(SIGKILL, sig_act);
	signal(SIGTERM, sig_act);
	
	srand ( time(NULL) );
	spXmlStartup();
	
	if(!db_open())
	{
		rep("Failed to connect to database");
		finalize(1);
	}else{
		
		//init event library
		event_init();
	
		if(firsrunCheckAll())
			rep("Internal DB verified");
		timerInit();
			
		timerSetAction(1,siteChecker, NULL, 2);
		rep("Connected to database");
		if(ConfI("PortNum"))
			bStd = bind_pool_add(NULL, ConfI("PortNum"), false);
		if(ConfI("PostNumSSL"))
			bSsl = bind_pool_add(NULL, ConfI("PostNumSSL"), true);
		
		if(ConfI("PortNum") && !bStd && ConfI("PostNumSSL") && !bSsl) {
			rep("Failed making socket"); 
			finalize(2); 
		}
	//timer thread
	//	timerInit(0);
		
	//start main loop
		rep("SkillPanel version "SPANEL_VERSION" is up and running");
		bind_one_std(bStd,true);
		bind_one_std(bSsl,true);
		conn_cycle();
	}

	finalize(0);
	return 0;
}

void sig_act(int signum)
{
	if(signum !=SIGHUP){
		rep("Caucht interrupt signal!");
		conn_cycle_break();
	}else{
		rep("Caught SIGHUP! Reloading config and reopen logs...");
		if(ConfLoad("/etc/spanel.conf")!=1){
			perror("ERROR! Failed to reread config file.\n");
			rep("ERROR! Failed to reread config file.\n");
		}
		LogReopen();
	}
}

void finalize(int errcode=0){
	rep ("Started shutdown sequence..");
		
	timerInit(true);
	bind_pool_clear();
	db_close();

	
	xsltCleanupGlobals();
	xmlCleanupParser();
	//close connections
	
	//-----------------
	if(pidf)
		close(pidf);
	unlink(ConfC("PATH_PIDFILE"));
	rep("SkillPanel server turned off");
	exit(errcode);
}







