#ifndef CVM__SQL__H__
#define CVM__SQL__H__

#include <str/str.h>
extern const char sql_query_default[];
extern int sql_query_validate(const char* template);
extern int sql_query_build(const char* template, str* q);

/* These routines must be defined by the SQL module */
extern const char sql_query_var[];
extern const char sql_pwcmp_var[];
extern const char sql_postq_var[];
extern int sql_auth_init(void);
extern int sql_auth_query(const str* query);
extern int sql_post_query(const str* query);
extern const char* sql_get_field(int field);
extern void sql_auth_stop(void);

#endif
