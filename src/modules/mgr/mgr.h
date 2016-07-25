#include <const_c.h>

int mgrProcess(sp_conn_s*);
int mgrPages(sp_conn_p);

int mgrAuth(sp_conn_s*);

void mgrDesctop(int, short int, void*);
void mgrOsTemp(int, short int, void*);
void mgrAddIp(int, short int, void*);
void mgrPS(int, short int, void*);
void mgrEdit(int, short int, void*);
void mgrLogout(int, short int, void*);
void mgrLogin(int, short int, void*);
void mgrKeepAlive(int, short int, void*);

void mgrCtsList(int, short int, void*);
void mgrCtsCreate(int, short int, void*);


/*
void mgrIpList(int, short int, void*);
void mgrIpAdd(int, short int, void*);
void mgrIpEdit(int, short int, void*);

void mgrOsList(int, short int, void*);
void mgrOsAdd(int, short int, void*);
void mgrOsEdit(int, short int, void*);
*/

void mgrSiteGroupList(int, short int, void*);
void mgrSiteGroupAdd(int, short int, void*);
void mgrSiteGroupEdit(int, short int, void*);


void mgrSiteList(int, short int, void*);
void mgrSiteAdd(int, short int, void*);
void mgrSiteUrls(int, short int, void*);
void mgrSiteUrlAdd(int, short int, void*);
void mgrSiteUrlDel(int, short int, void*);
void mgrSiteUrlEdit(int, short int, void*);

void mgrTplGoto(sp_conn_p c, char *, char *);

struct createInfo_s
{
	u_int id;
	char ip[INET_ADDRSTRLEN];
	char hostname[HOSTLEN];
	char ostmpl[HOSTLEN];
};
typedef struct createInfo_s createInfo;
typedef createInfo* createInfo_p;

