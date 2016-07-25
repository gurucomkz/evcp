#include <const_c.h>

void mgrDesctop(int fd, short evfd, void *arg){
	sp_conn* c = (sp_conn*)arg;
	spXmlParser p((char*)"/home/dev/cpp/docroot/sirius/desktop.xsl");
	char * presult;
	std::string myxml = 
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
		"fixpng=\"no\" localdir=\"local/\" img=\"\" css=\"main.css\" logolink=\"/mgr/\" logo=\"logo-vdsmgr.png\" >"
		"<mainmenu startform=\"\" startpage=\"list\">"
			"<node name=\"sitemonitor\">"
				"<node name=\"sitemon\"  action=\"sitemon\" />"
		
			"</node>"
			"<node name=\"sitemonitorctl\">"
				"<node name=\"sitegroups\"  action=\"sitegroups\" />"
				"<node name=\"sites\"  action=\"sites\" />"
		
			"</node>"
		"</mainmenu>"
		"<messages>"
			"<msg name=\"title\">Главная страница</msg>"
			"<msg name=\"logout\">Выйти</msg>"
			

			"<msg name=\"menu_sitemonitorctl\">Настройки мониторинга</msg>"
			"<msg name=\"menu_sitemonitor\">Мониторинг</msg>"
			"<msg name=\"menu_sitegroups\">Группы сайтов</msg>"
			"<msg name=\"menu_sites\">Сайты</msg>"
			"<msg name=\"menu_sitemon\">Мониторинг</msg>"
			"<msg name=\"help2\">Помощь</msg>"
			"<msg name=\"cpset\">Настройки</msg>"
			"<msg name=\"msg_m0\">ЩИТО?</msg>"
			"<msg name=\"m0\">WAS?</msg>"
		"</messages>"
		"<user level=\"server\" hostname=\"myservername\">admin"
		"</user>"
		"<path level=\"server\">admin</path>"
		"<copyright href=\"http://skilltex.kz/\">SKILLTEX</copyright>"
		"<pwgenlen>'8'</pwgenlen>"
		"<pwgencharacters>1234567890qwertyuiopasdfghjklzxcvbnmASDFGHJKLQWERTYUIOPZXCVBNM</pwgencharacters>"
	"</doc>"
	;
	
	presult = p.parse(myxml);
	
	//fprintf(stderr,"mgrDesctop line %d\n",__LINE__);
	
	if(presult!=NULL){
		c->resp->status=200;
		(*c->resp->h_out)["Content-Type"] = "text/html";
		conn_compile_resp(c);
		conn_fillresp(c,presult,strlen(presult));
	}else
		c->resp->status=500;
	
	c->handler = sp_finish_request;
	sp_next(c, c->handler);
}


void mgrLogin(int fd, short evfd, void *arg){
	sp_conn* c = (sp_conn*)arg;
	spXmlParser p((char*)"/home/dev/cpp/docroot/sirius/login.xsl");
	char * presult = NULL;
	std::string myxml = 
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<doc mtime=\"45gvergv4rg\" binary=\"/mgr/\" theme=\"/sirius/\" "
		"fixpng=\"no\" localdir=\"local/\" img=\"vdsmgr\" css=\"main.css\" logolink=\"/mgr/\" logo=\"logo-vdsmgr.png\" >"
		"<loginform theme=\"/sirius/\" lang=\"ru\" />"
		"<messages>"
			"<msg name=\"title\">Авторизация</msg>"
			"<msg name=\"theme\">Тема оформления</msg>"
			"<msg name=\"passwd\">Ваш пароль</msg>"
			"<msg name=\"user\">Ваш логин</msg>"
			"<msg name=\"newpasswd\">Новый пароль</msg>"
			"<msg name=\"newconfirm\">Подтвердите пароль</msg>"
			"<msg name=\"enter\">Войти</msg>"
		"</messages>"
		"<copyright href=\"http://skilltex.kz/\" >SKILLTEX</copyright>"
	"</doc>"
	;
	
	presult = p.parse(myxml);
	
	if(presult!=NULL){
		c->resp->status=200;
		(*c->resp->h_out)["Content-Type"] = "text/html";
		conn_compile_resp(c);
		conn_fillresp(c,presult,strlen(presult));
	}else
		c->resp->status=500;

	c->handler = sp_finish_request;
	sp_next(c, c->handler);
}

void mgrLogout(int fd, short evfd, void *arg){
	sp_conn* c = (sp_conn*)arg;
	sessionKill(c);
	cookieSet(c,"SPSID","");
	(*c->resp->h_out)["Location"] = "/mgr/";
	c->resp->status=302;
	c->handler = sp_finish_request;
	sp_next(c, c->handler);
}

int mgrAuth(sp_conn_p c)
{
	std::string a,b,d;
	DBRES q;
	MYSQL_ROW r;
	size_t p;
	if(sessionCheck(c)){
		fprintf(stderr,"\nAUTH FOUND SESSION\n");
		return 1;
	}
	if((*c->data->headers.list)["Authorization"] != "") {
		a = (*c->data->headers.list)["Authorization"];
		if(sp_strncmp((u_char*)"Basic",a.c_str(),5)) return 0;
		
		b = a.substr(6);
		a = base64_decode(b);
		p = a.find(':');
		if(p == std::string::npos) return 0;
		b = a.substr(0,p);
		d = a.substr(p+1);
	}else
	if(c->data->post_vars != NULL)
	{
		b = (*c->data->post_vars)["username"];
		d = (*c->data->post_vars)["password"];
	}
	else {
		fprintf(stderr,"\nAUTH FAILED TO FIND CREDENTIALS\n");
		return 0;
	}
	
	fprintf(stderr,"\nAUTH USER: %s , %s\n", b.c_str(),addslashes(b).c_str());
	fprintf(stderr,"\nAUTH PASS: %s , %s\n", d.c_str(),addslashes(d).c_str());
	
	if(b == "" || d == "") {
		fprintf(stderr,"\nAUTH PASS: empty\n");
		return 0;
	}
	
	q = db_query("SELECT user_id, vn_id, user_primary, user_god, user_privs FROM users WHERE userLogin='"+addslashes(b)+"' AND userPasswd='"+addslashes(d)+"' " );
	if(q == NULL || !db_num_rows(q)) {
		fprintf(stderr,"\nAUTH q == NULL\n");
		db_free_result(q);
		return 0;
	}
	r = db_fetch_row(q);
	fprintf(stderr,"\nAUTH USER FOUND\n", d.c_str());
	sessionRegister(c,atoi(r[0]),atoi(r[1]),a.c_str(),r[2][0]=='1',r[3][0]=='1',"ru");
	db_free_result(q);
	return 1;
}

void mgrKeepAlive(int fd, short evfd, void *arg){
	sp_conn* c = (sp_conn*)arg;
	fprintf(stderr,"mgrKeepAlive\n",fd,evfd, arg,c->hndstate);
	c->resp->status=200;
	conn_compile_resp(c);
	sp_next(c, sp_finish_request);
}

