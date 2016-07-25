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
#include <stdlib.h>

void spXmlStartup()
{
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;
}

spXmlParams::~spXmlParams()
{
	if(_data != NULL) {
		for(u_long x=0; x<_size; x++){
			if(_data[x*2]!=NULL) free(_data[x*2]);
			if(_data[x*2+1]!=NULL) free(_data[x*2+1]);
		}
		free(_data);
		_data = NULL;
	}
}
spXmlParams::spXmlParams(const spXmlParams& a)
{
	_size = a._size;
	len = a.len;
	_data = (char**)malloc(sizeof(char **)*(_size*2+1));

	memset(_data,0,sizeof(char **)*(_size*2+1));
	for(u_long x=0; x < len*2; x++){
		if(a._data[x]!=NULL) {
			u_long l = strlen(a._data[x]);
			_data[x] = (char*)malloc(l);
			memcpy(_data[x],a._data[x],l);
		}
	}
}
spXmlParams::spXmlParams()
{
	_size=0;
	len = 0; 
	_data = (char **)malloc(sizeof(char **)*(_size*2+1));
	memset(_data,0,sizeof(char **)*(_size*2+1));
}
bool spXmlParams::resize(u_long n){
	char ** nd;
	if(n>0){
		_size+=n;
		nd = (char **) realloc(NULL,sizeof(char **)*(_size*2+1)*10);
		if(nd==NULL) return false;
		_data = nd;
		memset(_data+_size-n*2,0,sizeof(char **)*(n*2+1));
		return true;
	}
	return false;
}

void spXmlParams::add(char* k, char * v)
{
	if(_size==len) 
		if(!resize()) return;
	_data[len*2] = (char*)malloc(strlen(k));
	_data[len*2+1] = (char*)malloc(strlen(v));
	memcpy(_data[len*2],k,strlen(k));
	memcpy(_data[len*2+1],k,strlen(v));
	_data[len*2+2]=NULL;
	len++;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


const char * spXmlParser::result()
{
	return (const char *)retbuf; 
}
spXmlParser::spXmlParser()
{ 
	nullmost();
}
void spXmlParser::nullmost(){ 
	stpl=""; 
	tpl=NULL; 
	doc=NULL; 
	_result = NULL; 
	xmlretbuf = NULL; 
	retbuf = NULL; 
	retbuflen = 0;
}
spXmlParser::spXmlParser(char* t){ 
	nullmost();
	stpl = t; 
	tpl = xsltParseStylesheetFile( (const xmlChar*) t ); 
}

spXmlParser::spXmlParser(const spXmlParser& a){
	nullmost();
	if(a.stpl != ""){
		stpl=a.stpl;
		tpl = xsltParseStylesheetFile( (const xmlChar*) stpl.c_str() ); 
	}
}
spXmlParser::spXmlParser(std::string t){
	nullmost();
	stpl = t; 
	tpl = xsltParseStylesheetFile( (const xmlChar*) t.c_str() ); 
}
char* spXmlParser::cleanup(bool full)
{
	//fprintf(stderr,"%s line %d ... %p\n",__func__,__LINE__,tpl);
	if(tpl != NULL) 
	{	xsltFreeStylesheet(tpl); tpl = NULL;}
	
	//fprintf(stderr,"%s line %d ... %p\n",__func__,__LINE__,_result);
	if(_result != NULL) 
	{	xmlFreeDoc(_result); _result = NULL; }
	
	//fprintf(stderr,"%s line %d ... %p\n",__func__,__LINE__,doc);
	if(doc != NULL) 
	{	xmlFreeDoc(doc); doc = NULL; }
	
	//fprintf(stderr,"%s line %d ... %p\n",__func__,__LINE__,retbuf);
	if(full && retbuf != NULL) 
	{	free(retbuf); retbuf = NULL; }
	
	//fprintf(stderr,"%s line %d ... %p\n",__func__,__LINE__,xmlretbuf);
	if(xmlretbuf != NULL) 
	{	free(xmlretbuf); xmlretbuf = NULL; }
	
	//fprintf(stderr,"%s line %d ...\n",__func__,__LINE__);
	return NULL;
}
spXmlParser::~spXmlParser()
{ 
	cleanup();
}
void spXmlParser::setParams(spXmlParams p){ _params = p; }

char* spXmlParser::doparse(const char* xmls, spXmlParams p){
	if(tpl == NULL)
		return cleanup();
	doc = xmlParseMemory(xmls,strlen(xmls));
	if(doc == NULL) 
		return cleanup();
	_result = xsltApplyStylesheet(tpl, doc, p.data());
	if(_result == NULL) 
		return cleanup();
	if(xsltSaveResultToString(&xmlretbuf, &retbuflen, _result, tpl))
		return cleanup();
	else
	{
		if(retbuf != NULL) { 
			free(retbuf); 
			retbuf = NULL; 
		}
		retbuf = (char*)malloc(retbuflen+1);
		memcpy(retbuf,xmlretbuf,retbuflen);
		retbuf[retbuflen]=0;
		//fprintf(stderr,"%s line %d ... %p,%p,%p,%p,%p\n",__func__,__LINE__,tpl,doc,_result,xmlretbuf,retbuf);
		cleanup(false);
		//fprintf(stderr,"%s line %d ... %p,%p,%p,%p,%p\n",__func__,__LINE__,tpl,doc,_result,xmlretbuf,retbuf);
		return retbuf;
	}
}
char* spXmlParser::parse(const char* xmls, spXmlParams p){ return doparse(xmls,p); }
char* spXmlParser::parse(char* xmls){ return doparse(xmls,_params); }
char* spXmlParser::parse(char* xmls,char * t){ 
	if(tpl==NULL){
		tpl = xsltParseStylesheetFile( (const xmlChar*) t ); 
		if(tpl == NULL) return NULL;
	}
	return doparse(xmls,_params); 
}
char* spXmlParser::parse(std::string xmls,char * t){ 
	if(tpl==NULL){
		tpl = xsltParseStylesheetFile( (const xmlChar*) t ); 
		if(tpl == NULL) return NULL;
	}
	return doparse(xmls.c_str(),_params); 
}
char* spXmlParser::parse(std::string xmls){ return doparse(xmls.c_str(),_params); }
char* spXmlParser::parse(std::string xmls, spXmlParams p){ return doparse(xmls.c_str(),p); }

