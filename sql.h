#ifndef CVM__SQL__H__
#define CVM__SQL__H__

#include "str/str.h"
extern int sql_query_setup(const char* q);
extern int sql_query_build(str* q);

#endif
