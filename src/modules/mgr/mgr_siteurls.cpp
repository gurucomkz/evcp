#include <const_c.h>

void mgrSiteUrls(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p((char*)"/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	DBRES q,qCount,qTrouble;
	MYSQL_ROW r,rCount,rTrouble;
	sp_strmap M,P;
	if(c->data->get_vars) M = *c->data->get_vars;
	if(c->data->post_vars) P = *c->data->post_vars;

	fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	for(;;){ //this permits few cycles
		switch(c->hndstate)
		{
		case 0:
			if(P["act"]=="siteurldel")
				c->hndstate = 2; 
			else
				c->hndstate = 1; 
			break;
		case 1:
			//prepare output buf
			c->resp->status=200;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			conn_compile_resp(c);
	
			q = db_query("SELECT s.pageId, s.pageUrl, s.pageStatus, s.pageHttpStatus, "
								"s.pageLoadTime, s.pageLastCheck, s.pageSize "
							"FROM site_pages s "
							);
			
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			myxml += 
				"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
					"fixpng=\"\" localdir=\"local/\" img=\"\" css=\"main.css\" logolink=\"/mgr/\" logo=\"logo-vdsmgr.png\" >"
					"<p_num>1</p_num>"
					"<p_cnt>30</p_cnt>"
					"<p_sort>''</p_sort>"
					"<p_order>''</p_order>"
					"<plid>"+M["elid"]+"</plid>"
					"<func>siteurls</func>"
					"<slave></slave>"
					"<tfilter></tfilter>"
					"<metadata type=\"list\" selficon=\"equipment\" key=\"spid\">"
						"<coldata type=\"list\" key=\"spid\">"
							"<col type=\"data\" name=\"spid\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"purl\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"pstatus\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"phttpstatus\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"ploadtime\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"plastcheck\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"psize\" wrap=\"no\" align=\"left\" />"
						"</coldata>"
						"<toolbar type=\"list\" key=\"spid\">"
							"<toolbtn type=\"back\" name=\"backtosites\" img=\"t-back\" func=\"sites\" />"
							
							"<toolbtn type=\"func\" name=\"urladd\" img=\"t-new\" default=\"no\" func=\"tb_op_new('siteurladd')\" />"
							"<toolbtn type=\"func\" name=\"urledit\" img=\"t-edit\" default=\"yes\" func=\"tb_op_new('siteurledit')\" />"
						
							"<toolbtn type=\"group\" name=\"urldelete\" img=\"t-delete\" func=\"siteurldel\" />"
						"</toolbar>"
					"</metadata>"
					;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			while(r = db_fetch_row(q))
			{
				myxml += "<elem>";
					myxml += "<spid>"+(std::string)r[0]+"</spid>";
					myxml += "<purl>"+(std::string)r[1]+"</purl>";
					myxml += "<pstatus>"+(std::string)r[2]+"</pstatus>";
					myxml += "<phttpstatus>"+(std::string)r[3]+"</phttpstatus>";
					myxml += "<ploadtime>"+(std::string)r[4]+"</ploadtime>";
					myxml += "<plastcheck>"+(std::string)r[5]+"</plastcheck>";
					myxml += "<psize>"+(std::string)r[6]+"</psize>";
					
					
					
				
				myxml += "</elem>";
			}
			
			db_free_result(q);
			myxml += 
				"<messages>"
					"<msg name=\"title\">Проверяемые URLы</msg>"
					"<msg name=\"spid\">ID</msg>"
					"<msg name=\"purl\">URL</msg>"
					"<msg name=\"pstatus\">Состояние</msg>"
					"<msg name=\"phttpstatus\">HTTP-статус</msg>"
					"<msg name=\"ploadtime\">Скорость загрузки</msg>"
					"<msg name=\"plastcheck\">Посл. проверка</msg>"
					"<msg name=\"psize\">Вес</msg>"
					"<msg name=\"m0\">WAS?</msg>"
					
					"<msg name=\"hint_urladd\">Добавить URL</msg>"
					"<msg name=\"hint_urledit\">Изменить URL</msg>"
					"<msg name=\"hint_urldelete\">Удалить URL</msg>"
					"<msg name=\"msg_nosel\">Ничего не выделено</msg>"
					"<msg name=\"msg_siteurldel\">Удалить URL</msg>"
					"<msg name=\"msg_disabled\">Отключено</msg>"
					"<msg name=\"msg_m0\">ЩИТО?</msg>"
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
			fprintf(stderr,"%s line %d... %s\n",__func__,__LINE__,P["elid"].c_str());
			db_query("DELETE FROM site_pages WHERE pageId='"+addslashes(P["elid"])+"'");
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
void mgrSiteUrlAdd(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p((char*)"/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	DBRES qGroups;
	MYSQL_ROW rGroups;
	int ic;
	sp_strmap M;
	unsigned long newSiteId = 0;
	char cm[32];
	fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	if(c->data->post_vars) M = *c->data->post_vars;
	for(;;){ //this permits few cycles
		fprintf(stderr,"%s line %d state %d...\n",__func__,__LINE__,c->hndstate);
		switch(c->hndstate)
		{
		case 0:
			//prepare output buf
			c->resp->status=200;
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
				"<func>siteadd</func>"
				"<slave>0</slave>"
				"<metadata type=\"form\" name=\"siteurladd\">"
					"<form height=\"200\" maxwidth=\"300\" focus=\"single\" top=\"yes\" extokmsg=\"m0\">"
						"<page name=\"single\">"
							"<field name=\"newurl\"><input type=\"text\" name=\"newurl\" /></field>"
							"<field name=\"sid\"><select name=\"sid\" /></field>"
						"</page>"							
					"</form>"
						
				"</metadata>"	
				"<messages>"
					"<msg name=\"title\">Добавление сайта</msg>"
					
					"<msg name=\"msg_extokmsg\">Создать</msg>"
					"<msg name=\"msg_back\">Отмена</msg>"
					"<msg name=\"m0\">WAS?</msg>"
					
					"<msg name=\"newdomain\">Домен сайта</msg>"
					"<msg name=\"newurl\">URL для проверки</msg>"
					"<msg name=\"multistart\">Начало диапазона IP-адрес</msg>"
					"<msg name=\"multiend\">Окончание диапазона IP-адрес</msg>"
					"<msg name=\"msg_m0\">ЩИТО?</msg>"
				"</messages>"
				"<pwgenlen>'8'</pwgenlen>"
				"<pwgencharacters></pwgencharacters>";
			myxml +=
				"<slist name=\"sid\">";
			qGroups = db_query("SELECT i.siteId, i.siteDomain FROM sites i  ORDER BY siteDomain");
			while(rGroups = db_fetch_row(qGroups))
			{
				myxml += "<val key=\""+(std::string)rGroups[0]+"\">"+(std::string)rGroups[1]+"</val>";
			}
			db_free_result(qGroups);
			myxml += 
				"</slist>";
			
			myxml += "</doc>";
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
			fprintf(stderr,"%s:%d '%s' '%s' '%s' ",
					__func__,__LINE__,M["gid"].c_str(),M["newdomain"].c_str(),M["newurl"].c_str());
			if((M["gid"] != "") && (M["newdomain"] != "") && (M["newurl"] != ""))
			{
				//single
				db_query("INSERT INTO sites (groupId,siteDomain,siteUrl) "
							"VALUES ('"+addslashes(M["gid"])+"','"+addslashes(M["newdomain"])+"','"+addslashes(M["newurl"])+"')");
				newSiteId = db_insert_id();
				if(newSiteId){
					snprintf(cm,31,"%l",newSiteId);
					db_query("INSERT INTO site_pages (siteId,pageUrl) "
								"VALUES ('"+((std::string)cm)+"','"+addslashes(M["newurl"])+"')");
				}
				mgrTplGoto(c,(char*)"",(char*)"sites");
			}
			else{
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

void mgrSiteUrlEdit(int fd, short evfd, void *arg)
{
}
