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

#include <pthread.h>
#include "mydb.h"
#include "splog.h"
#include <sqlite3.h>
#include <ctype.h>


pthread_mutex_t	mydb_query_lock = PTHREAD_MUTEX_INITIALIZER;
static int sqlite_errs_c = 26;
static char* sqlite_errs[]={
	"Successful result",
/* beginning-of-error-codes */
"SQL error or missing database",
"Internal logic error in SQLite",
"Access permission denied",
"Callback routine requested an abort",
"The database file is locked",
"A table in the database is locked",
"A malloc() failed",
"Attempt to write a readonly database",
"Operation terminated by sqlite3_interrupt()",
"Some kind of disk I/O error occurred",
"The database disk image is malformed",
"NOT USED. Table or record not found",
"Insertion failed because database is full",
"Unable to open the database file",
"NOT USED. Database lock protocol error",
"Database is empty",
"The database schema changed",
"String or BLOB exceeds size limit",
"Abort due to constraint violation",
"Data type mismatch",
"Library used incorrectly",
"Uses OS features not supported on host",
"Authorization denied",
"Auxiliary database format error",
"2nd parameter to sqlite3_bind out of range",
"File opened that is not a database file"

};

static sqlite3* DBCONN = NULL;
void mysql_thread_init(){}
void mysql_thread_end(){}

int db_open(){
	int r = sqlite3_open_v2("/home/dev/spanel.db",&DBCONN,/*SQLITE_OPEN_FULLMUTEX|*/SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,NULL);
	if(r==SQLITE_OK) return 1;
	return 0;
}
void db_close(){
	if(DBCONN==NULL) return;
	if(sqlite3_close(DBCONN) == SQLITE_OK){
		DBCONN = NULL;
	}
}

const char* db_error(){
	return sqlite3_errmsg(DBCONN);
}
const char* db_errorA(unsigned int e){
	return e<sqlite_errs_c?sqlite_errs[e]:"";
}

int db_errno(){
	return sqlite3_errcode(DBCONN);
}

MYSQL_ROW db_fetch_row(DBRES r){
	if(r==NULL) return NULL;
	//fetches current row and increments pointer
	//fprintf(stderr,"fetching row # %d of %d with %d columns. tbl %p-%p size %d (%d): %p \n",
	//			r->crow,r->rows,r->cols,r->tbl, r->tbllast, r->tblsz, (r->tbllast - r->tbl)/sizeof(r->tbl), r->tbl+ r->cols * r->crow);
		
	if(r->crow+1 < r->rows){
		//r->crow++;
		MYSQL_ROW x = (r->tbl + r->cols * ++r->crow );
	//	fprintf(stderr,"RETURNING COLS: ");
	//	for(int i = 0; i < r->cols; i++)
	//	{	
	//		fprintf(stderr,"\t%d/%d: %p:%p:%s \n",i,r->cols-1,&x[i],x[i],x[i]); 
	//	}
		return x;
	}
	return NULL;
}
int db_num_rows(DBRES r){
	if(r==NULL) return 0;
	return r->rows > 1 ? r->rows-1 : 0;
}

int db_affected_rows(){return sqlite3_changes(DBCONN);}

unsigned long db_insert_id(){ return sqlite3_last_insert_rowid(DBCONN); }

DBRES db_query(std::string sql){
	return db_query(sql.c_str());
}

bool db_save_row(DBRES r, void* dPtr)
{
	char** data = (char**)dPtr, **x;
	int newsize = r->tblsz + r->cols;
	//fprintf(stderr,"tbl = %p, tblpos = %p, tbllast = %p\n", r->tbl,r->tblpos,r->tbllast);
	if((r->tbllast - r->tblpos)/sizeof(char*) < newsize){
		char** np = (char**)realloc(r->tbl, newsize*sizeof(char*));
		//fprintf(stderr,"reallocing %p(%d) to %p(%d)\n", r->tbl, r->tblsz, np, newsize);
		
		if(np!=NULL)
		{
			//fprintf(stderr,"oldpos diff = %d\n", r->tblpos - r->tbl);
			//r->tblpos = np + (r->rows * r->cols)*sizeof(char*);
			r->tblpos = np + (r->tblpos - r->tbl);
			r->tblsz = newsize;
			//fprintf(stderr,"\nr->tbllast = %d + %d * %d \n\n", np , r->tblsz,  sizeof(char*));
			r->tbllast = np + r->tblsz;
			r->tbl = np;
			//fprintf(stderr,"tbl = %p, tblpos = %p, tbllast = %p, LASH SHOULD BE AT %p . CLANUP OF %d bytes\n", 
			//				r->tbl,	r->tblpos,		r->tbllast, 	r->tbl+newsize*sizeof(char*),	(r->tbllast - r->tblpos));
			//fprintf(stderr,"\tZEROING FROM %p TO %p\n", r->tblpos, r->tbllast);
			for(x=r->tblpos; x<r->tbllast; x++) {
				
				//fprintf(stderr,"\t\tZEROING POS %p\n", x);
				*x = NULL;
			}
		}else return false;
	}
	//fprintf(stderr,"SAVING COLS: ");
	for(int i = 0; i < r->cols; i++)
	{
		u_long sl = strlen(data[i]);
		//fprintf(stderr,"\t%p:%s -> ",data[i],data[i]);
		*r->tblpos = (char*)malloc(sl+1);
		if(*r->tblpos != NULL){
			//fprintf(stderr,"%p",*r->tblpos);
			strcpy(*r->tblpos,data[i]);
			//fprintf(stderr,":%s\n",*r->tblpos);
			r->tblpos++;
		}else {
			//fprintf(stderr," ERR! \n",*r->tblpos);
			return false;
		}
	}
	r->rows++; 
	//fprintf(stderr,"\n");
	return true;
}
DBRES db_query(const char* sql){
	DBRES r;
	
	rep(sql);
	sqlite3_stmt *pStmt = NULL;     /* Statement to execute. */
	const char *zLeftover;          /* Tail of unprocessed SQL */
	
	pthread_mutex_lock(&mydb_query_lock);
	r = (DBRES)malloc(sizeof(DBRESX));
	memset(r, 0, sizeof(DBRESX));
	
	r->tblsz = r->rows = r->cols = r->crow = 0;
	r->lastresult = SQLITE_OK;             /* Return Code */
	
	while( sql[0] && (SQLITE_OK == r->lastresult) ){
		r->lastresult = sqlite3_prepare_v2(DBCONN, sql, -1, &pStmt, &zLeftover);
		if( SQLITE_OK == r->lastresult ){
			if( !pStmt ){
				/* this happens for a comment or white-space */
				sql = zLeftover;
				while( isspace(sql[0]) ) sql++;
				continue;
			}
			
			/* perform the first step.  this will tell us if we
			** have a result set or not and how wide it is.
			*/
			r->lastresult = sqlite3_step(pStmt);
			/* if we have a result set... */
			if( SQLITE_ROW == r->lastresult ){
				/* allocate space for col name ptr, value ptr, and type */
				r->cols = sqlite3_column_count(pStmt);
				void *pData = sqlite3_malloc(2*r->cols*sizeof(const char*) + 1);
				if( !pData ){
					r->lastresult = SQLITE_NOMEM;
				}else{
					char **azCols = (char **)pData;      /* Names of result columns */
					char **azVals = &azCols[r->cols];       /* Results */
					int i;
					
					for(i=0; i<r->cols; i++){
						azCols[i] = (char *)sqlite3_column_name(pStmt, i);
					}
					
					db_save_row(r, azCols);
					
					do{
						/* extract the data and data types */
						for(i=0; i < r->cols; i++){
							azVals[i] = (char *)sqlite3_column_text(pStmt, i);
							if( !azVals[i] ){
								r->lastresult = SQLITE_NOMEM;
								break; /* from for */
							}
						} /* end for */
								
						/* if data and types extracted successfully... */
						if( SQLITE_ROW == r->lastresult ){ 
							/* call the supplied callback with the result row data */
							if( !db_save_row(r, azVals) ){
								r->lastresult = SQLITE_ABORT;
							}else{
								r->lastresult = sqlite3_step(pStmt);
							}
						}
					}while(SQLITE_ROW == r->lastresult);
					sqlite3_free(pData);
				}
			}
			
			/* Finalize the statement just executed. If this fails, save a 
			** copy of the error message. Otherwise, set zSql to point to the
			** next statement to execute. */
			r->lastresult = sqlite3_finalize(pStmt);
			if( r->lastresult == SQLITE_OK ){
				sql = zLeftover;
				while( isspace(sql[0]) ) sql++;
			}else{
				rep(db_error());
			}
		}
	}
	
	pthread_mutex_unlock(&mydb_query_lock);
	return r;
}

void db_free_result(DBRES res){
	if(res==NULL) return;
	try{
		if(res->tbl != NULL){
			for(char** i = res->tbl; i<res->tbllast; i++){
				//fprintf(stderr,"\t%s try to free %p:%p\n",__func__,i,*i);
				if(*i!=NULL) free(*i);
			}
			free(res->tbl);
			res->tbl = NULL;
		}
		free(res);
	}catch(...){}
}

