#include <const_c.h>



void mgrCtsCreate(int fd, short evfd, void *arg)
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
			//fetch os list
			c->hndstate = 1; //nextstate | sed -e \"s/\\.tar\\.gz//\"
			if(!runProgram("/bin/ls -1Uoglh --color=never --time-style=+%s /vz/template/cache/*.tar.gz | sed -e \"s/\\/.*\\///\" ",c)){
				fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
				c->hndstate = 2; //nextstate
				break; //next cycle
			}
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
			fprintf(stderr,"%s line %d... D SIZE = %d (of %d)\n",__func__,__LINE__,d.size(),c->rp->len);
			runProgramFinish(c->rp);
			
			myxml += 
				"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
					"fixpng=\"no\" localdir=\"local/\" img=\"\" css=\"main.css\" logolink=\"/mgr/\" logo=\"logo-vdsmgr.png\" >"
					"<p_num>1</p_num>"
					"<p_cnt>1</p_cnt>"
					"<p_sort>''</p_sort>"
					"<p_order>''</p_order>"
					"<plid></plid>"
					"<func>vds</func>"
					"<slave>0</slave>"
					"<tfilter></tfilter>"
					"<metadata type=\"form\" onsubmit=\"dddd\">"
						"<form height=\"200\" maxwidth=\"300\" focus=\"maindata\" top=\"yes\" extokmsg=\"dddd\">"
							"<page name=\"maindata\">"
								"<field name=\"sysid\" ><input type=\"text\" name=\"sysid\" /></field>"
								"<field name=\"ipaddr\" ><input type=\"text\" name=\"ipaddr\" /></field>"
								"<field name=\"hostname\" ><input type=\"text\" name=\"hostname\" /></field>"
								"<field name=\"pw\" ><input type=\"password\" name=\"pw\" /></field>"
								"<field name=\"procnum\" ><input type=\"text\" name=\"procnum\" /></field>"
								"<field name=\"memsize\" ><input type=\"text\" name=\"memsize\" /></field>"
								"<field name=\"ostmpl\" ><select type=\"text\" name=\"ostmpl\" /></field>"
								"<field name=\"diskspace\" ><input type=\"text\" name=\"name\" /></field>"
							"</page>"
							"<page name=\"notedata\">"
							"</page>"
							"<page name=\"auxdata\">"
								"<field name=\"inodes\" ><input type=\"text\" name=\"name\" /></field>"
							"</page>"
							
						"</form>"
							
					"</metadata>"	
					;
			myxml += "<slist name=\"ostmpl\">";
				myxml += "<option value=\"\">Please, select</option>";
			for(di = d.begin(); di < d.end(); di++)
			{
				i = explode(di->c_str());
				myxml += "<option value=\""+i[4]+"\">"+i[4]+"</option>";
			}
			myxml += "</slist>";
			myxml += 
				"<messages>"
					"<msg name=\"title\">Создаём контейнер</msg>"
					"<msg name=\"maindata\">Основные настройки</msg>"
					"<msg name=\"notedata\">Заметки</msg>"
					"<msg name=\"auxdata\">Подробные настройки</msg>"
					
					"<msg name=\"sysid\">Сист. номер</msg>"
					"<msg name=\"hostname\">Имя сервера</msg>"
					"<msg name=\"procnum\">Процессов</msg>"
					"<msg name=\"pw\">ROOT-Пароль</msg>"
					"<msg name=\"memsize\">ОЗУ</msg>"
					"<msg name=\"ipaddr\">IP-адрес</msg>"
					"<msg name=\"ostmpl\">Шаблон ОС</msg>"
					"<msg name=\"diskspace\">Размер диска</msg>"
					"<msg name=\"inodes\">Кол-во Inodes</msg>"
					"<msg name=\"msg_extokmsg\">Создать</msg>"
					"<msg name=\"msg_back\">Отмена</msg>"
					
					"<msg name=\"msg_id\">Внутренний служебный идентификатор сервера</msg>"
					"<msg name=\"id\">Идентификатор</msg>"
					
					"<msg name=\"msg_name\">Отмен2</msg>"
					"<msg name=\"name\">Отмен2f</msg>"
					"<msg name=\"hint_id\">Отмена</msg>"
					"<msg name=\"hint_name\">Отмена</msg>"
					"<msg name=\"hint_ip\">Отмена</msg>"
					"<msg name=\"hint_ip1\">Отмена</msg>"
				"</messages>"
				"<pwgenlen>'8'</pwgenlen>"
				"<pwgencharacters>1234567890qwertyuiopasdfghjklzxcvbnmASDFGHJKLQWERTYUIOPZXCVBNM</pwgencharacters>"
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

void mgrCtsList(int fd, short evfd, void *arg)
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
			if(!runProgram("vzlist -H -a -o ctid,hostname,ostemplate,ip,status,numproc,diskspace,diskspace.s,laverage,cpulimit,cpuunits,ioprio,onboot",c)){
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
					"<metadata type=\"list\" selficon=\"main\">"
						"<coldata type=\"list\" key=\"sss\">"
							"<col type=\"data\" name=\"elid\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"host\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"ipaddr\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"ostempl\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"numproc\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"status\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"laverage\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"space\" wrap=\"no\" align=\"left\" />"
						"</coldata>"
						"<toolbar type=\"list\" key=\"sss\">"
							"<toolbtn type=\"func\" name=\"create\" img=\"t-new\" default=\"yes\" func=\"tb_op_new('vds.add')\" />"
							"<toolbtn type=\"edit\" img=\"t-edit\" default=\"no\" func=\"vds.edit\" />"
							//"<toolbtn type=\"edit\" img=\"t-on\" func=\"vds.start\" />"
							//"<toolbtn type=\"edit\" img=\"t-off\" func=\"vds.stop\" />"
						"</toolbar>"
					"</metadata>"
					;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			for(di = d.begin(); di < d.end(); di++)
			{
				i = explode(di->c_str());
				//conn_fillresp(c,di->c_str());
				myxml += "<elem>";
					myxml += "<elid>"+i[0]+"</elid>";
					myxml += "<host>"+i[1]+"</host>";
					myxml += "<ostempl>"+i[2]+"</ostempl>";
					myxml += "<ipaddr>"+i[3]+"</ipaddr>";
					myxml += "<status>"+i[4]+"</status>";
					myxml += "<numproc>"+i[5]+"</numproc>";
					myxml += "<space>"+ltos(atol(i[6].c_str())/1024)+(std::string)"/"+ltos(atol(i[7].c_str())/1024)+"</space>";
					myxml += "<laverage>"+i[8]+"</laverage>";
				
				myxml += "</elem>";
			}
			
			myxml += 
				"<messages>"
					"<msg name=\"title\">Список контейнеров</msg>"
					
					"<msg name=\"elid\">Сист. номер</msg>"
					"<msg name=\"numproc\">Процессов</msg>"
					"<msg name=\"ostempl\">Шаблон ОС</msg>"
					"<msg name=\"laverage\">Нагрузка</msg>"
					"<msg name=\"status\">Состояние</msg>"
					"<msg name=\"ipaddr\">IP-адрес</msg>"
					"<msg name=\"space\">Использование диска</msg>"
					"<msg name=\"host\">Хост</msg>"
					"<msg name=\"hint_create\">create111</msg>"
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

