#include <const_c.h>


void mgrOsList(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p("/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	svec d,i;
	svec::iterator di,ii;
	fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	for(;;){ //this permits few cycles
		switch(c->hndstate)
		{
		case 0:
		fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			c->hndstate = 1; //nextstate
			if(!runProgram("/bin/ls -1Uoglh --color=never --time-style=+%s /vz/template/cache/*.tar.gz | sed -e \"s/\\/.*\\///\"",c)){
		fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
				c->hndstate = 2; //nextstate
				break; //next cycle
			}
		fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			return;
		case 1:
			//prepare output buf
			c->resp->status=200;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			(*c->resp->h_out)["Content-Type"] = "text/html";
			conn_compile_resp(c);
			
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			//process retrieved buffer
			d = explode(c->rp->buf,c->rp->len,'\n');
			//free run result
			runProgramFinish(c->rp);
			
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			myxml += 
				"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
					"fixpng=\"\" localdir=\"local/\" img=\"\" css=\"main.css\" logolink=\"/mgr/\" logo=\"logo-vdsmgr.png\" >"
					"<p_num>1</p_num>"
					"<p_cnt>1</p_cnt>"
					"<p_sort>''</p_sort>"
					"<p_order>''</p_order>"
					"<plid></plid>"
					"<func>vds</func>"
					"<slave>0</slave>"
					"<tfilter></tfilter>"
					"<metadata type=\"list\" selficon=\"main\" key=\"ostempl\">"
						"<coldata type=\"list\">"
							"<col type=\"data\" name=\"ostempl\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"mtime\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"size\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"status\" wrap=\"no\" align=\"left\" />"
						"</coldata>"
						"<toolbar type=\"list\">"
							"<toolbtn type=\"func\" name=\"create\" img=\"t-new\" default=\"yes\" func=\"tb_op_new('osadd')\" />"
							"<toolbtn type=\"groupedit\" img=\"t-delete\" default=\"no\" func=\"osdel\" />"
						"</toolbar>"
					"</metadata>"
					;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			for(di = d.begin(); di < d.end(); di++)
			{
				i = explode(di->c_str());
				myxml += "<elem>";
					myxml += "<ostempl>"+i[4]+"</ostempl>";
					myxml += "<mtime>"+i[3]+"</mtime>";
					myxml += "<size>"+i[2]+"</size>";
				myxml += "</elem>";
			}
			
			myxml += 
				"<messages>"
					"<msg name=\"title\">Шаблоны ОС</msg>"
					"<msg name=\"ostempl\">Шаблон ОС</msg>"
					"<msg name=\"status\">Состояние</msg>"
					"<msg name=\"size\">Размер</msg>"
					"<msg name=\"mtime\">Дата создания</msg>"
					"<msg name=\"hint_create\">Добавить новый шаблон ОС из репозитория</msg>"
				"</messages>"
				"</doc>";
			fprintf(stderr,"%s line %d... %s\n",__func__,__LINE__,myxml.c_str());
			presult = p.parse(myxml);
			
			if(presult != NULL){
				conn_fillresp(c,presult);
			}else{
				conn_fillresp(c,"Template parse error");
			}
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		case 2: //error 500
			c->resp->status=500;			
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		}
	}
}

void mgrOsAdd(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p("/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	sp_strmap M;
	char cm[16];
	fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	if(c->data->post_vars) M = *c->data->post_vars;
	for(;;){ //this permits few cycles
		fprintf(stderr,"%s line %d state %d...\n",__func__,__LINE__,c->hndstate);
		switch(c->hndstate)
		{
		case 0:
			//prepare output buf
			c->resp->status=200;
			(*c->resp->h_out)["Content-Type"] = "text/html";
			conn_compile_resp(c);
			if(M["sok"]!="ok")
				c->hndstate = 1;
			else
				c->hndstate = 2;
			break;
		case 1: //collect data
			myxml += 
			"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
				"fixpng=\"no\" localdir=\"local/\" img=\"\" css=\"main.css\">"
				"<plid></plid>"
				"<func>osadd</func>"
				"<slave>0</slave>"
				"<metadata type=\"form\" name=\"osadd\">"
					"<form height=\"200\" maxwidth=\"300\" focus=\"single\" top=\"yes\" extokmsg=\"dddd\">"
						"<page>"
							"<field name=\"ostemplate\"><select name=\"ostemplate\" /></field>"
						"</page>"					
					"</form>"
						
				"</metadata>"	
				"<messages>"
					"<msg name=\"title\">Добавление Шаблонов ОС</msg>"
					
					"<msg name=\"msg_extokmsg\">Создать</msg>"
					"<msg name=\"msg_back\">Отмена</msg>"
					
					"<msg name=\"ostemplate\">Новый IP-адрес</msg>"
				"</messages>"
				"<pwgenlen>'8'</pwgenlen>"
				"<pwgencharacters></pwgencharacters>"
				"<slist name=\"ostemplate\">";
			
				
			myxml += 
				"</slist>"
			"</doc>";
			
			presult = p.parse(myxml);
			if(presult != NULL){
				conn_fillresp(c,presult);
			}else{
				conn_fillresp(c,"Template parse error");
			}
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		case 2: //execute
			
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		case 500: //error 500
			c->resp->status=500;			
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		}
	}
}

