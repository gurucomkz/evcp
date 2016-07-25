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
#include <string>
#include <map>
#include <stdlib.h>

#include "config.h"
#include "splog.h"


int ConfigClass::ci(const char *key){	
	int x = 0;
	x = atoi(data[key].c_str());
	return x;
}
long ConfigClass::cl(const char *key){	
	long x = 0;
	x = atol(data[key].c_str());
	return x;
}
float ConfigClass::cf(const char *key){	
	float x = 0;
	x = atof(data[key].c_str());
	return x;
}
double ConfigClass::cd(const char *key){	
	double x = 0;
	x = atof(data[key].c_str());
	return x;
}
const char* ConfigClass::cc(const char * key)
{	return data[key].c_str();	}
std::string ConfigClass::cs(const char * key)
{	return data[key];	}

ConfigClass::ConfigClass(){	//here we define defaults
	data["PATH_PIDFILE"]="/var/run/spanel.pid";
	data["PATH_OWNROOT"]="/var/spanel/";
	data["PATH_USERSKEL"]="/var/spanel/skel";
	data["PATH_NAMED_STORE"]="/var/named/chroot/var/named/";
	data["PATH_NAMED_SUFFIX"]="spanel/";
	data["PATH_VHOST_CONF"]="/var/spanel/vhost/";

	data["PATH_NAMED_PIDFILE"]="/var/named/chroot/var/run/named/named.pid";
	data["DNS_1"]="ns1";
	data["DNS_2"]="ns2";

	data["CGATE_PORT"]="106";
	data["SOCK_RETRY"]="5";

	data["CGATE_HOST"]="localhost.localdomain";

	data["SQL_USER"]="root";
	data["SQL_DB"]="skillpanel";
	data["SQL_HOST"]="localhost";
	data["SQL_PASS"]="";

	data["UID_MIN"]="500";
	data["MYSQL_DATA"]="/var/lib/mysql/";
	data["MAIL_DATA_PRE"]="/var/CommuniGate/Domains/";
	data["MAIL_DATA_SUF"]=".macnt";
	data["DEFAULT_LOGSNUM"]="10";
	data["PortNum"]="400";

	/*
	data["USED_FS"]="/dev/VolGroup00/LogVol00";
	data["CGATE_PASSWD"]="La0IvR87p3C";
	*/

	data["CGATE_USER"]="skillpanel";
	data["CGATE_PASSWD"]="";

	data["DNS_1_IP"]="127.0.0.1";
	data["DNS_2_IP"]="127.0.0.1";
	data["USED_FS"]="/";
}
void ConfigClass::dump(){
	confmap::const_iterator cIter;
	cIter = this->data.begin();
	while(cIter!=data.end()){
		printf("%s = %s\n",cIter->first.c_str(),cIter->second.c_str());
		cIter++;
	}
}

int ConfigClass::LoadFromFile(const char* fname){
	FILE* f;
	char * buf = (char*)malloc(10240), c;
	long i;
	//declare states
	enum {sLFD,	/*lookForDirective*/
		 sRD,	/*reading directive*/
		 sLFV,	//look for value
		 sRV,	//reading value
		 sRC	//reading comment
	}state;		
	std::string k,v;
	bool inq;

	snprintf(buf,10240,"Reading config from file: %s",fname);
	rep(buf);
	f = fopen(fname,"r");
	if(f==NULL) {
		free(buf);
		return 0;
	}
	while(fgets(buf,10239,f)){
		//printf("NL: %s\n",buf);
		k = v = "";
		inq = false;
		state = sLFD;
		for(i=0;buf[i];i++){
			c = buf[i];
			switch(state){
			case sLFD:
				if(c == '#' ) {buf[i+1]=0; break;};
				if(c>32){
					k += c;
					state = sRD;
				}
				break;
			case sRD:
				if(c == '#' ) {buf[i+1]=0; break;} ;
				if(c>32){
					k += c;
				}else
					state = sLFV;
				break;
			case sLFV:
				if(c == '#' ) {buf[i+1]=0; break;} ;
				if(c < 33)
					break;
				else
					state = sRV;					
			case sRV:
				if(c == '#' ) {buf[i+1]=0; break;} ;
				if(c > 32 ){
					if(!inq){
						if(c=='"')	
							inq = true;
						else
							v += c;
					}else{
						if(c=='"')	{
							inq = false;
							buf[i+1]=0; break;
						}else
							v += c;
					}
				//	printf("V = %s\n",v.c_str());
				}else{
					buf[i+1]=0; break;
				}
				break;
			default:
				free(buf);
				return -1;
			}
		}
		if(k!=""){
			//snprintf(buf,10240,"Config[%s] = %s;",k.c_str(),v.c_str());
			//rep(buf);
			data[k] = v; 
		}
		
	}
	snprintf(buf,10239,"Configuration loaded: %d keys in configuration",data.size());
	rep(buf);
	free(buf);
	fclose(f);
	return 1;
}

static ConfigClass Config;

int ConfI(const char *key) { return Config.ci(key); }
long ConfL(const char *key) { return Config.cl(key); }
float ConfF(const char *key){ return Config.cf(key); }
double ConfD(const char *key){ return Config.cd(key); }
const char* ConfC(const char * key){ return Config.cc(key); }
std::string ConfS(const char * key){ return Config.cs(key); }
int ConfLoad(const char *fname) { return Config.LoadFromFile(fname); }


