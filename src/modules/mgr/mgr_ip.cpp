#include <const_c.h>

void mgrIpList(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p("/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	DBRES q;
	MYSQL_ROW r;
	sp_strmap M;
	if(c->data->get_vars) M = *c->data->get_vars;
	fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	for(;;){ //this permits few cycles
		switch(c->hndstate)
		{
		case 0:
			if(M["act"]=="ipdel")
				c->hndstate = 2; 
			else
				c->hndstate = 1; 
			break;
		case 1:
			//prepare output buf
			c->resp->status=200;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			(*c->resp->h_out)["Content-Type"] = "text/html";
			conn_compile_resp(c);
	
			q = db_query("SELECT i.ip_addr, i.ip_added, i.ip_avail "
							"FROM ip_pool i "
							);
			
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			myxml += 
				"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
					"fixpng=\"\" localdir=\"local/\" img=\"\" css=\"main.css\" logolink=\"/mgr/\" logo=\"logo-vdsmgr.png\" >"
					"<p_num>1</p_num>"
					"<p_cnt>30</p_cnt>"
					"<p_sort>''</p_sort>"
					"<p_order>''</p_order>"
					"<plid></plid>"
					"<func>iplist</func>"
					"<slave>0</slave>"
					"<tfilter></tfilter>"
					"<metadata type=\"list\" selficon=\"equipment\" key=\"ipaddr\">"
						"<coldata type=\"list\" key=\"sss\">"
							"<col type=\"data\" name=\"ipaddr\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"hostname\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"avail\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"created\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"assigned\" wrap=\"no\" align=\"left\" />"
						"</coldata>"
						"<toolbar type=\"list\" key=\"sss\">"
							"<toolbtn type=\"func\" name=\"ipcreate\" img=\"t-new\" default=\"yes\" func=\"tb_op_new('ipadd')\" />"
							"<toolbtn type=\"func\" name=\"ipedit\" img=\"t-edit\" default=\"no\" func=\"tb_op_new('ipedit')\" />"
							"<toolbtn type=\"editlist\" name=\"ipdelete\" img=\"t-delete\" func=\"ipdel\" />"
						"</toolbar>"
					"</metadata>"
					;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			while(r = db_fetch_row(q))
			{
				myxml += "<elem>";
					myxml += "<ipaddr>"+(std::string)r[0]+"</ipaddr>";
					myxml += "<created>"+(std::string)r[1]+"</created>";
					myxml += "<avail>"+(std::string)r[2]+"</avail>";
					//myxml += "<hostname>"+(std::string)r[1]+"</hostname>";
					//myxml += "<assigned>"+(std::string)r[4]+"</assigned>";
				
				myxml += "</elem>";
			}
			
			db_free_result(q);
			myxml += 
				"<messages>"
					"<msg name=\"title\">Список IP-адресов</msg>"
					"<msg name=\"ipaddr\">IP-адрес</msg>"
					"<msg name=\"hostname\">Хост</msg>"
					"<msg name=\"avail\">Использовать?</msg>"
					"<msg name=\"created\">Создан</msg>"
					"<msg name=\"assigned\">Присвоен</msg>"
					
					"<msg name=\"hint_ipcreate\">Добавить IP-адрес в список</msg>"
					"<msg name=\"hint_ipedit\">Изменить свойства IP-адреса</msg>"
					"<msg name=\"hint_ipdelete\">Удалить IP-адрес из список используемых</msg>"
					"<msg name=\"msg_nosel\">Ничего не выделено</msg>"
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
		case 2:
			fprintf(stderr,"DELETE FROM ip_pool WHERE ip_addr='%s'\n",addslashes(M["elid"]).c_str());
			db_query("DELETE FROM ip_pool WHERE ip_addr='"+addslashes(M["elid"])+"'");
			c->hndstate = 1; 
			sp_next(c, c->handler);
			return;
		case 10: //error 500
			c->resp->status=500;			
			c->handler = sp_finish_request;
			sp_next(c, c->handler);
			return;
		}
	}
}
void mgrIpAdd(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p("/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	int ic,cip1[4],cip2[4];
	sp_strmap M;
	char cm[16];
	svec ip1,ip2;
	svec::iterator ip1i,ip2i;
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
		case 1:
			myxml += 
			"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
				"fixpng=\"no\" localdir=\"local/\" img=\"\" css=\"main.css\">"
				"<plid></plid>"
				"<func>ipadd</func>"
				"<slave>0</slave>"
				"<metadata type=\"form\" name=\"ipadd\">"
					"<form height=\"200\" maxwidth=\"300\" focus=\"single\" top=\"yes\" extokmsg=\"dddd\">"
						"<page name=\"single\">"
							"<field name=\"singleip\"><input type=\"text\" name=\"singleip\" /></field>"
							"<field name=\"avail\"><input type=\"checkbox\" name=\"avail\" /></field>"
						"</page>"
						"<page name=\"milti\">"
							"<field name=\"multistart\"><input type=\"text\" name=\"multistart\" /></field>"
							"<field name=\"multiend\"><input type=\"text\" name=\"multiend\" /></field>"
						"</page>"							
					"</form>"
						
				"</metadata>"	
				"<messages>"
					"<msg name=\"title\">Добавление IP-адресов</msg>"
					"<msg name=\"single\">Один адрес</msg>"
					"<msg name=\"milti\">Несколько адресов</msg>"
					
					"<msg name=\"pagerok\">Сист. номер</msg>"
					"<msg name=\"pagerline\">Процессов</msg>"
					"<msg name=\"pagershow\">IP-адрес</msg>"
					"<msg name=\"msg_extokmsg\">Создать</msg>"
					"<msg name=\"msg_back\">Отмена</msg>"
					
					"<msg name=\"singleip\">Новый IP-адрес</msg>"
					"<msg name=\"avail\">Доступен к использованию</msg>"
					"<msg name=\"multistart\">Начало диапазона IP-адрес</msg>"
					"<msg name=\"multiend\">Окончание диапазона IP-адрес</msg>"
				"</messages>"
				"<pwgenlen>'8'</pwgenlen>"
				"<pwgencharacters></pwgencharacters>"
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
			fprintf(stderr,"%s:%d '%s' '%s' '%s'",
					__func__,__LINE__,M["singleip"].c_str(),M["multistart"].c_str(),M["multiend"].c_str());
			if(M["singleip"]!="")
			{
				//single
				db_query("INSERT INTO ip_pool (ip_addr,ip_added,ip_avail) "
							"VALUES ('"+addslashes(M["singleip"])+"',datetime('now'),'1')");
				conn_fillresp(c,"IP Added OK");
			}else
			if(M["multistart"] != "" && M["multiend"] != ""){
				//multi
				
				ip1 = explode(M["multistart"].c_str(),'.');
				ip2 = explode(M["multiend"].c_str(),'.');
				fprintf(stderr,"%s:%d SIZES %d,%d\n",__func__,__LINE__,ip1.size(),ip2.size());
				if(ip1.size()==4 && ip2.size()==4)
				{
					for(ic = 0; ic < 4; ic++){
						cip1[ic] = atoi(ip1[ic].c_str());
						cip2[ic] = atoi(ip2[ic].c_str());
						fprintf(stderr,"%s:%d pos=%d: %d,%d\n",__func__,__LINE__,ic,cip1[ic],cip2[ic]);
						if(cip1[ic]<0 || cip1[ic]>255 ||
							cip2[ic]<0 || cip2[ic]>255)
						{	c->hndstate = 1; break; }
					}
					if(c->hndstate != 2) 
						break;
					while(cip1[0] < cip2[0] || cip1[1] < cip2[1] || cip1[2] < cip2[2] || cip1[3] < cip2[3])
					{
						for(ic = 3; ic >=0; ic--)
							if(cip1[ic]<255) {cip1[ic]++; break;}
							else cip1[ic] = 0;
						snprintf(cm,16,"%d.%d.%d.%d",cip1[0],cip1[1],cip1[2],cip1[3]);
						fprintf(stderr,"%s\n",cm);
						db_query("INSERT INTO ip_pool (ip_addr,ip_added,ip_avail) "
							"VALUES ('"+addslashes(cm)+"',datetime('now'),'1')");
							conn_fillresp(c,cm);
							conn_fillresp(c,"<br />\n");
					}
				}else{
					c->hndstate = 1;
					break;
				}
			}else{
				c->hndstate = 1;
				break;
			}
		
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

void mgrIpEdit(int fd, short evfd, void *arg)
{
}
