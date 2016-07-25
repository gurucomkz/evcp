#ifndef _SPANEL_H_BASE64_
#define _SPANEL_H_BASE64_
#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif /*_SPANEL_H_BASE64_*/
