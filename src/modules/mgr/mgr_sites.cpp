#include <const_c.h>

void mgrSiteList(int fd, short evfd, void *arg)
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
			if(P["act"]=="sitedel")
				c->hndstate = 2; 
			else
				c->hndstate = 1; 
			break;
		case 1:
			//prepare output buf
			c->resp->status=200;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			conn_compile_resp(c);
	
			q = db_query("SELECT s.siteId, s.siteDomain, s.siteUrl, s.siteStatus, s.siteHttpStatus, "
								"s.siteLoadTime, s.siteLastCheck, g.groupName "
							"FROM sites s "
							"LEFT JOIN site_groups g ON g.groupId=s.groupId "
							"ORDER BY s.siteDomain"
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
					"<func>sitegroups</func>"
					"<slave></slave>"
					"<tfilter></tfilter>"
					"<metadata type=\"list\" selficon=\"equipment\" key=\"sid\">"
						"<coldata type=\"list\" key=\"sid\">"
							"<col type=\"data\" name=\"sid\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"sdomain\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"surl\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"sstatus\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"shttpstatus\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"sloadtime\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"slastcheck\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"gname\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"pagecount\" wrap=\"no\" align=\"left\" stat=\"stat\" />"
							"<col type=\"data\" name=\"problems\" wrap=\"no\" align=\"left\" stat=\"stat\"  />"
						"</coldata>"
						"<toolbar type=\"list\" key=\"sss\">"
							"<toolbtn type=\"func\" name=\"siteadd\" img=\"t-new\" default=\"no\" func=\"tb_op_new('siteadd')\" />"
							"<toolbtn type=\"func\" name=\"siteedit\" img=\"t-edit\" default=\"no\" func=\"tb_op_new('siteedit')\" />"
							"<toolbtn type=\"editlist\" name=\"pagelist\" img=\"t-iplist\" default=\"yes\" func=\"siteurls\" />"
							
							"<toolbtn type=\"group\" name=\"sitedelete\" img=\"t-delete\" func=\"sitedel\" />"
						"</toolbar>"
					"</metadata>"
					;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			while(r = db_fetch_row(q))
			{
				myxml += "<elem>";
					myxml += "<sid>"+(std::string)r[0]+"</sid>";
					myxml += "<sdomain>"+(std::string)r[1]+"</sdomain>";
					myxml += "<surl>"+(std::string)r[2]+"</surl>";
					myxml += "<sstatus>"+(std::string)r[3]+"</sstatus>";
					myxml += "<shttpstatus>"+(std::string)r[4]+"</shttpstatus>";
					myxml += "<sloadtime>"+(std::string)r[5]+"</sloadtime>";
					myxml += "<slastcheck>"+(std::string)r[6]+"</slastcheck>";
					myxml += "<gname>"+(std::string)r[7]+"</gname>";
					
					qCount = db_query("SELECT count(pageId) FROM sites_pages WHERE siteId="+(std::string)r[0]);
					if(rCount = db_fetch_row(qCount))
						myxml += "<pagecount>"+(std::string)rCount[0]+"</pagecount>";
					db_free_result(qCount);
					
					qTrouble = db_query("SELECT count(pageId) FROM sites_pages WHERE page_status!='0' AND siteId="+(std::string)r[0]);
					if(rTrouble = db_fetch_row(qTrouble))
						myxml += "<problems>"+(std::string)rTrouble[0]+"</problems>";
					db_free_result(qTrouble);
					
				
				myxml += "</elem>";
			}
			
			db_free_result(q);
			myxml += 
				"<messages>"
					"<msg name=\"title\">Сайты</msg>"
					"<msg name=\"sid\">ID</msg>"
					"<msg name=\"sdomain\">Домен</msg>"
					"<msg name=\"surl\">URL</msg>"
					"<msg name=\"sstatus\">Состояние</msg>"
					"<msg name=\"shttpstatus\">HTTP-статус</msg>"
					"<msg name=\"sloadtime\">Скорость загрузки</msg>"
					"<msg name=\"slastcheck\">Посл. проверка</msg>"
					"<msg name=\"gname\">Имя группы</msg>"
					"<msg name=\"pagecount\">Страниц</msg>"
					"<msg name=\"problems\">Проблем</msg>"
					"<msg name=\"m0\">WAS?</msg>"
					
					"<msg name=\"hint_ipcreate\">Добавить сайт</msg>"
					"<msg name=\"hint_ipedit\">Изменить свойства сайта</msg>"
					"<msg name=\"hint_ipdelete\">Удалить сайт</msg>"
					"<msg name=\"msg_nosel\">Ничего не выделено</msg>"
					"<msg name=\"msg_sitedel\">Удалить сайт</msg>"
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
			db_query("DELETE FROM sites WHERE siteId='"+addslashes(P["elid"])+"'");
			db_query("DELETE FROM site_pages WHERE siteId='"+addslashes(P["elid"])+"'");
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
void mgrSiteAdd(int fd, short evfd, void *arg)
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
				"<slave></slave>"
				"<metadata type=\"form\" name=\"siteadd\">"
					"<form height=\"200\" maxwidth=\"300\" focus=\"single\" top=\"yes\" extokmsg=\"dddd\">"
						"<page name=\"single\">"
							"<field name=\"newdomain\"><input type=\"text\" name=\"newdomain\" /></field>"
							"<field name=\"newurl\"><input type=\"text\" name=\"newurl\" /></field>"
							"<field name=\"gid\"><select name=\"gid\" /></field>"
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
				"<slist name=\"gid\">";
			qGroups = db_query("SELECT i.groupId, i.groupName FROM site_groups i  ORDER BY groupName");
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

void mgrSiteEdit(int fd, short evfd, void *arg)
{
}
