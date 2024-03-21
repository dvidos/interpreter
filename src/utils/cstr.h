#ifndef _CSTR_H
#define _CSTR_H

#include "../containers/_containers.h"
#include "../containers/_containers.h"


extern contained_item_info *cstr_item_info;


typedef const char cstr;

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(cstr);
#define failed_cstr(inner, fmt, ...)  __failed_cstr(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


#endif
