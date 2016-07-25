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

#include <const_c.h>


bool firsrunCheckAll()
{
	DBRES q,qq;
	MYSQL_ROW r,rr;
	std::string s;
	db_query(	"CREATE TABLE IF NOT EXISTS vns ("
					"vn_id INTEGER PRIMARY KEY ASC, "
					"vn_hostname, "
					"vn_priip, "
					"vn_created, "
					"vn_expires, "
					"vn_package, "
					"vn_cpu, "
					"vn_disksize, "
					"vn_numfiles, "
					"vn_numproc, "
					"vn_bnc_kmemsize, "
					"vn_bnc_lockedpages, "
					"vn_bnc_privvmpages, "
					"vn_bnc_shmpages, "
					"vn_bnc_numproc, "
					"vn_bnc_physpages, "
					"vn_bnc_vmguarpages, "
					"vn_bnc_oomguarpages, "
					"vn_bnc_numtcpsock, "
					"vn_bnc_numflock, "
					"vn_bnc_numpty, "
					"vn_bnc_numsiginfo, "
					"vn_bnc_tcpsndbuf, "
					"vn_bnc_tcprcvbuf, "
					"vn_bnc_othersockbuf, "
					"vn_bnc_dgramrcvbuf, "
					"vn_bnc_numothersock, "
					"vn_bnc_dcachesize, "
					"vn_bnc_numfile, "
					"vn_bnc_numiptent "
				")");
	//fprintf(stderr,"SQL BEFORE %d in %s: %s\n",__LINE__,__FILE__,db_error());

	db_query(	"CREATE TABLE IF NOT EXISTS vns_ips ("
					"vn_id INTEGER, "
					"ip_addr, "
					"ip_added "
				")");
	db_query(	"CREATE TABLE IF NOT EXISTS remoteoslist ("
					"os_file, "
					"os_size, "
					"os_url, "
					"os_date, "
					"UNIQUE() "
				")");
	//fprintf(stderr,"SQL BEFORE %d in %s: %s\n",__LINE__,__FILE__,db_error());
				
	db_query(	"CREATE TABLE IF NOT EXISTS `users` ("
					"user_id INTEGER PRIMARY KEY ASC, "
					"vn_id DEFAULT 0, "
					"userLogin DEFAULT '', "
					"userPasswd TEXT DEFAULT '', "
					"user_email DEFAULT 'root@localhost', "
					"user_voice DEFAULT '', "
					"user_primary DEFAULT 0, "
					"user_god DEFAULT 0, "
					"user_privs DEFAULT 'USAGE', "
					"user_added INTEGER DEFAULT 0, "
					"UNIQUE(userLogin) "
				")");
	db_query(	"CREATE TABLE IF NOT EXISTS `site_groups` ("
					"groupId INTEGER PRIMARY KEY ASC, "
					"groupName DEFAULT '', "
					"groupOrder INTEGER DEFAULT 0, "
					"UNIQUE(groupName) "
				")");
			
	db_query(	"CREATE TABLE IF NOT EXISTS `sites` ("
					"siteId INTEGER PRIMARY KEY ASC, "
					"siteDomain DEFAULT '', "
					"siteUrl DEFAULT '', "
					"groupId INTEGER DEFAULT 0, "
					"siteStatus INTEGER DEFAULT 0, "
					"siteHttpStatus INTEGER DEFAULT 0, "
					"siteLoadTime INTEGER DEFAULT 0, "
					"siteLastCheck INTEGER DEFAULT 0, "
					"UNIQUE(siteDomain) "
				")");
	db_query(	"CREATE TABLE IF NOT EXISTS `site_pages` ("
					"pageId INTEGER PRIMARY KEY ASC, "
					"pageUrl DEFAULT '', "
					"siteId INTEGER DEFAULT 0, "
					"pageStatus INTEGER DEFAULT 0, "
					"pageHttpStatus INTEGER DEFAULT 0, "
					"pageLoadTime INTEGER DEFAULT 0, "
					"pageSize INTEGER DEFAULT 0, "
					"pageLastCheck INTEGER DEFAULT 0, "
					"UNIQUE(pageUrl) "
				")");
	//fprintf(stderr,"SQL BEFORE %d in %s: %s\n",__LINE__,__FILE__,db_error());
	//BEGIN TRANSACTION; 
	db_query("INSERT INTO users (userLogin,userPasswd,user_god,user_privs) VALUES('admin','eeeee','1','ROOT'); ");
	//COMMIT;
	db_query("INSERT INTO users (userLogin,userPasswd,user_god,user_privs) VALUES('admin1','11111','1','ROOT'); ");
	db_query("INSERT INTO users (userLogin,userPasswd,user_god,user_privs) VALUES('admin2','11111','1','ROOT'); ");
	db_query("INSERT INTO users (userLogin,userPasswd,user_god,user_privs) VALUES('admin3','11111','1','ROOT'); ");
	//fprintf(stderr,"SQL BEFORE %d in %s: %s\n",__LINE__,__FILE__,db_error());
	db_free_result(db_query("CREATE TABLE IF NOT EXISTS ip_pool ("
					"ip_addr, "
					"ip_added, "
					"ip_avail DEFAULT '1', "
					"PRIMARY KEY(ip_addr) "
				")"));
	//fprintf(stderr,"SQL BEFORE %d in %s: %s\n",__LINE__,__FILE__,db_error());
	
	q = db_query("SELECT user_id, userLogin, userPasswd, user_god, user_privs FROM users ");//   AND userPasswd='admin'
	if(db_num_rows(q)){
		//fprintf(stderr,"SQL BEFORE %d in %s: %s\n",__LINE__,__FILE__,db_error());
		//fprintf(stderr,"AAAAA QUERY RESULT: %p %p %p %p %p \n",q, q->tbl,&(q->rows),&(q->cols),q->errmsg);
		//fprintf(stderr,"AAAAA QUERY RESULT: %p %p %d %d %d %p \n",q, q->tbl,q->rows,q->cols,q->crow,q->errmsg);
		//fprintf(stderr,"\ndb_query rows %d: %s\n", q->rows-1,db_error());
		while(r = db_fetch_row(q)){
			//fprintf(stderr,"%s:%d\t%s:%d\t%s:%d\t%s:%d\t%s:%d\t\n", r[0],strlen(r[0]),r[1],strlen(r[1]),r[2],strlen(r[2]),r[3],strlen(r[3]),r[4],strlen(r[4]));
			s = (std::string)"SELECT * FROM users WHERE userLogin='"+(std::string)r[1]+"'";
			qq = db_query(s.c_str());
			if(db_num_rows(qq))
			while(rr = db_fetch_row(qq)){
				fprintf(stderr,"\t%s\t%s\t%s\t%s\t%s\t\n", rr[0],rr[1],rr[2],rr[3],rr[4]);
			}
			else
				fprintf(stderr,"\tUSER %s NOT FOUND\n", r[1]);
				
			db_free_result(qq);
			
		}
	}
	db_free_result(q);
	
	return true;
}

