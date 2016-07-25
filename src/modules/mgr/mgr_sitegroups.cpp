#include <const_c.h>

void mgrSiteGroupList(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p((char*)"/home/dev/cpp/docroot/sirius/modules.xsl");
	std::string myxml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char * presult;
	DBRES q,qCount,qTrouble;
	MYSQL_ROW r,rCount,rTrouble;
	sp_strmap M,P;
	if(c->data->get_vars) M = *c->data->get_vars;
	if(c->data->post_vars) M = *c->data->post_vars;
	fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
	for(;;){ //this permits few cycles
		switch(c->hndstate)
		{
		case 0:
			if(M["act"]=="sitegroupdel")
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
	
			q = db_query("SELECT i.groupId, i.groupName "
							"FROM site_groups i "
							"ORDER BY groupName"
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
					"<metadata type=\"list\" selficon=\"equipment\" key=\"gid\">"
						"<coldata type=\"list\" key=\"gid\">"
							"<col type=\"data\" name=\"gid\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"gname\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"sites\" wrap=\"no\" align=\"left\" />"
							"<col type=\"data\" name=\"problems\" wrap=\"no\" align=\"left\" />"
						"</coldata>"
						"<toolbar type=\"list\" key=\"sss\">"
							"<toolbtn type=\"func\" name=\"sitegroupcreate\" img=\"t-new\" default=\"no\" func=\"tb_op_new('sitegroupadd')\" />"
							"<toolbtn type=\"func\" name=\"sitegroupedit\" img=\"t-edit\" default=\"yes\" func=\"tb_op_new('sitegroupedit')\" />"
							"<toolbtn type=\"editlist\" name=\"sitegroupdelete\" img=\"t-delete\" func=\"sitegroupdel\" />"
						"</toolbar>"
					"</metadata>"
					;
			fprintf(stderr,"%s line %d...\n",__func__,__LINE__);
			while(r = db_fetch_row(q))
			{
				myxml += "<elem>";
					myxml += "<gid>"+(std::string)r[0]+"</gid>";
					myxml += "<gname>"+(std::string)r[1]+"</gname>";
					
					qCount = db_query("SELECT count(siteId) FROM sites WHERE groupId="+(std::string)r[0]);
					if(rCount = db_fetch_row(qCount))
						myxml += "<sites>"+(std::string)rCount[0]+"</sites>";
					db_free_result(qCount);
					qTrouble = db_query("SELECT count(siteId) FROM sites WHERE siteStatus!='0' AND groupId="+(std::string)r[0]);
					if(rTrouble = db_fetch_row(qTrouble))
						myxml += "<problems>"+(std::string)rTrouble[0]+"</problems>";
					db_free_result(qTrouble);
					//myxml += "<assigned>"+(std::string)r[4]+"</assigned>";
				
				myxml += "</elem>";
			}
			
			db_free_result(q);
			myxml += 
				"<messages>"
					"<msg name=\"title\">Группы сайтов</msg>"
					"<msg name=\"gid\">ID</msg>"
					"<msg name=\"gname\">Имя группы</msg>"
					"<msg name=\"sites\">Всего сайтов</msg>"
					"<msg name=\"problems\">Проблем</msg>"
					
					"<msg name=\"hint_ipcreate\">Добавить группу в список</msg>"
					"<msg name=\"hint_ipedit\">Изменить свойства группы</msg>"
					"<msg name=\"hint_ipdelete\">Удалить группу</msg>"
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
			fprintf(stderr,"DELETE FROM site_groups WHERE groupId='%s'\n",addslashes(P["elid"]).c_str());
			db_query("DELETE FROM site_groups WHERE groupId='"+addslashes(P["elid"])+"'");
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
void mgrSiteGroupAdd(int fd, short evfd, void *arg)
{
	sp_conn_p c = (sp_conn_p)arg;
	spXmlParser p((char*)"/home/dev/cpp/docroot/sirius/modules.xsl");
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
				"<func>sitegroupadd</func>"
				"<slave>0</slave>"
				"<metadata type=\"form\" name=\"sitegroupadd\">"
					"<form height=\"200\" maxwidth=\"300\" focus=\"single\" top=\"yes\" extokmsg=\"dddd\">"
						"<page name=\"single\">"
							"<field name=\"newname\"><input type=\"text\" name=\"newname\" /></field>"
						"</page>"							
					"</form>"
						
				"</metadata>"	
				"<messages>"
					"<msg name=\"title\">Добавление группы сайтов</msg>"
					"<msg name=\"single\">По одной</msg>"
					
					"<msg name=\"msg_extokmsg\">Создать</msg>"
					"<msg name=\"msg_back\">Отмена</msg>"
					
					"<msg name=\"newname\">Имя новой группы</msg>"
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
			fprintf(stderr,"%s:%d '%s' ",
					__func__,__LINE__,M["newname"].c_str());
			if(M["newname"]!="")
			{
				//single
				db_query("INSERT INTO site_groups (groupName) "
							"VALUES ('"+addslashes(M["newname"])+"')");
				mgrTplGoto(c,(char*)"",(char*)"sitegroups");
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

void mgrSiteGroupEdit(int fd, short evfd, void *arg)
{
}
