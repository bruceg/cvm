#ifndef CVM__SQL__H__
#define CVM__SQL__H__

#include "str/str.h"
extern const char sql_query_default[];
extern int sql_query_validate(const char* template);
extern int sql_query_build(const char* template, str* q);

#endif
