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

#ifndef _SPANEL_H_XML_
#define _SPANEL_H_XML_

#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/HTMLtree.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

void spXmlStartup();

class spXmlParams
{
private:
	u_long len;
	u_long _size;
	char ** _data;
	bool resize(u_long n = 10);
public:
	u_long length(){return len;}
	u_long size(){return _size;}
	const char ** data() {
		//fprintf(stderr,"%s line %d ; D=%p, D[0]=%p\n",__func__,__LINE__,_data,_data[0]);
		return (const char **)_data;
	}
	spXmlParams(); 
	spXmlParams(const spXmlParams&);
	~spXmlParams();
	void add(char* , char * );
};

typedef spXmlParams * spXmlParams_p;


class spXmlParser
{
	spXmlParams _params;
	xmlDocPtr doc, _result;
	char* retbuf;
	void nullmost();
	char* cleanup(bool full=true);
	xmlChar * xmlretbuf;
	int retbuflen;
	char* doparse(const char*, spXmlParams);
public:
	xsltStylesheetPtr tpl;
	std::string stpl;
	const char * result();
	spXmlParser();
	spXmlParser(const spXmlParser&);
	spXmlParser(char*);
	spXmlParser(std::string);
	~spXmlParser();
	void setParams(spXmlParams);
	
	char* parse(const char*, spXmlParams);
	char* parse(char*,char*);
	char* parse(char*);
	char* parse(std::string);
	char* parse(std::string,char*);
	char* parse(std::string, spXmlParams);
};



#endif /*_SPANEL_H_XML_*/
