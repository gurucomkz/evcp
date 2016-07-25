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

/*
		mydb.h


*/
#include <sqlite3.h>
#include <string>
#include <pthread.h>

#ifndef SPANEL_MYDB
#define SPANEL_MYDB

typedef int	MYSQL;
typedef char * char_p;

struct DBRESX_s {
	sqlite3* dbc;
	sqlite3_stmt * stmt;
	char_p* tbl;
	char_p*	tblpos;
	char_p*	tbllast;
	int 	tblsz;
	int 	rows;
	int 	cols;
	int 	crow;
	char*	errmsg;
	int 	lastresult;
};

typedef struct DBRESX_s DBRESX;

typedef  DBRESX* DBRES;

typedef char_p* MYSQL_ROW;
//typedef MYSQL_ROWX* MYSQL_ROW;

unsigned long db_insert_id(); 
#define mysql_insert_id db_insert_id

int db_num_rows(DBRES);
#define mysql_num_rows db_num_rows

void mysql_thread_init();
void mysql_thread_end();

int db_affected_rows();
#define mysql_affected_rows db_affected_rows

MYSQL_ROW db_fetch_row(DBRES);
#define mysql_fetch_row db_fetch_row

const char* db_error();
#define mysql_error db_error

const char* db_errorA(unsigned int );

int db_errno();
#define mysql_errno db_errno

int db_open();
#define mysql_init db_open

void db_close();
DBRES db_query(const char*);
DBRES db_query(std::string);




void db_free_result(DBRES);
#define mysql_free_result db_free_result

#endif
