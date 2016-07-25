#include <const_c.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <openssl/md5.h>


int mgrProcess(sp_conn_p c)
{
	sp_strmap M,*P;
	sp_strmap::iterator Mi;
	if(c->data->get_vars) M = *c->data->get_vars;
	if(c->data->post_vars) {
		P = c->data->post_vars;
		for(Mi = P->begin();Mi!=P->end(); Mi++)
			M[Mi->first] = Mi->second;
	}
	
	c->hndstate = 0;
	c->handler = NULL;
	
		//(*c->resp->h_out)["WWW-Authenticate"] = "Basic realm=\"SkillServer Authzz\"";
		//(*c->resp->h_out)["Location"] = "/login.html";
	(*c->resp->h_out)["Content-Type"] = "text/html; charset=UTF-8";
		//c->resp->status=302;
		//return -1;

	//actions
	if(mgrAuth(c)!=1)			c->handler = mgrLogin; else 
	if(M["act"]=="keepalive") 	c->handler = mgrKeepAlive; else
//	if(M["act"]=="list") 	 	c->handler = mgrCtsList; else
//	if(M["act"]=="vds.add") 	c->handler = mgrCtsCreate; else
//BEGIN	SITE GROUPS
	if(M["act"]=="sitegroups") 			c->handler = mgrSiteGroupList; else
	if(M["act"]=="sitegroupadd" ) 		c->handler = mgrSiteGroupAdd; else
	if(M["act"]=="sitegroupdel") 		c->handler = mgrSiteGroupList; else
//	if(M["act"]=="sitegroupedit") 		c->handler = mgrSiteGroupAdd; else
//END	SITE GROUPS

//BEGIN	SITES
	if(M["act"]=="sites") 			c->handler = mgrSiteList; else
	if(M["act"]=="siteadd" ) 		c->handler = mgrSiteAdd; else
	if(M["act"]=="sitedel") 		c->handler = mgrSiteList; else
//	if(M["act"]=="siteedit") 		c->handler = mgrSiteAdd; else
	if(M["act"]=="siteurls") 		c->handler = mgrSiteUrls; else
	if(M["act"]=="siteurladd") 		c->handler = mgrSiteUrlAdd; else
	if(M["act"]=="siteurldel") 		c->handler = mgrSiteUrls; else
//	if(M["act"]=="siteurledit") 	c->handler = mgrSiteUrlEdit; else
//END	SITES

//BEGIN	OS TEMPLATES //ls -1Uoglh --color=never --time-style=+%s *.tar.gz
//	if(M["act"]=="oslist") 		c->handler = mgrOsList; else
//	if(M["act"]=="osadd" ) 		c->handler = mgrOsAdd; else
//	if(M["act"]=="osdel") 		c->handler = mgrOsList; else
//END	OS TEMPLATES

//	if(M["act"]=="ostempl") 	c->handler = mgrOsTemp; else
//	if(M["act"]=="procstat") 	c->handler = mgrPS; else
	if(M["act"]=="logout") 		c->handler = mgrLogout; else
	//if(M["act"]=="edit") 		c->handler = mgrEdit; else
	if(M["act"]=="") {
		c->handler = mgrDesctop;
	}else{
		c->resp->status=501;
		return -1;
	}

	sp_next(c, c->handler);
	return 1;
}


void mgrCreate(int fd, short evfd, void *arg){
	sp_conn_p c = (sp_conn_p)arg;
	svec d,i;
	svec::iterator di,ii;
	char * cmdbuf;
	//internal data
	createInfo_p I;
	///////////////
	fprintf(stderr,"mgrCreate(%d,%d,%p,%d)...\n",fd,evfd, arg,c->hndstate);
	for(;;){ //this permits few cycles
		switch(c->hndstate)
		{
		case 0:
			c->hndstate = 1; //succ state
			I = (createInfo_p)malloc(sizeof(createInfo));
			
			cmdbuf = (char*)malloc(SMALLBUF);
			
			snprintf(cmdbuf,SMALLBUF,"vzctl create %d --ostemplate=%s --ipadd %s --hostname %s");
			if(!runProgram(cmdbuf,c)){
				c->hndstate = 2; //fail state
				break; //next cycle
			}
			return;
		case 1:
			//prepare output buf
			c->resp->status=200;
			(*c->resp->h_out)["Content-Type"] = "application/x-javascript";
			conn_compile_resp(c);
			
			//process retrieved buffer
			d = explode(c->rp->buf,c->rp->len,'\n');
			//free run result
			runProgramFinish(c->rp);
			
			conn_fillresp(c,"[\"");
			for(di = d.begin(); di < d.end(); di++)
			{
				if(di != d.begin()) conn_fillresp(c,"\",\"");
				conn_fillresp(c,di->c_str());
			}
			conn_fillresp(c,"\"]");
			sp_next(c, sp_finish_request);
			return;
		case 2: //error 500
			c->resp->status=500;			
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		}
	}
}
void mgrAddIp(int fd, short evfd, void *arg){return ;}
void mgrOsTemp(int fd, short evfd, void *arg){
	sp_conn_p c = (sp_conn_p)arg;
	
	c->handler = sp_finish_request;
	sp_next(c, c->handler);
}

void mgrPS(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	svec d;
	char x=0;
	svec::iterator di;
	fprintf(stderr,"mgrPS(%d,%d,%p,%d)...\n",fd,evfd, arg,c->hndstate);
	for(;;){ //this permits few cycles
		switch(c->hndstate)
		{
		case 0:
			c->hndstate = 1; //nextstate
			if(!sysGetFile((char*)"/proc/loadavg",c)){
				c->hndstate = 2; //nextstate
				break; //next cycle
			}
			return;
		case 1:
			//prepare output buf
			c->resp->status=200;
			(*c->resp->h_out)["Content-Type"] = "application/x-javascript";
			conn_compile_resp(c);
			
			//process retrieved buffer
			d = explode(c->rp->buf,c->rp->len);
			//free run result
			runProgramFinish(c->rp);
			
			conn_fillresp(c,"[\"");
			for(di = d.begin(); di < d.end() && x++<3; di++)
			{	
				if(di != d.begin()) conn_fillresp(c,"\",\"");
				conn_fillresp(c,di->c_str());
			}
			conn_fillresp(c,"\"]");
			sp_next(c, sp_finish_request);
			return;
		case 2: //error 500
			c->resp->status=500;			
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		}
	}
}
void mgrEdit(int fd, short evfd, void *arg){return ;}

void mgrTplGoto(sp_conn_p c, char * one, char * two)
{
	conn_fillresp(c,"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">");
	conn_fillresp(c,"<script type=\"text/javascript\">");
	conn_fillresp(c,"top.mn_do_action('");
		conn_fillresp(c,one);
			conn_fillresp(c,"','");
		conn_fillresp(c,two);
			conn_fillresp(c,"');");

	conn_fillresp(c,"</script>");

}



////
