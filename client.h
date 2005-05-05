#ifndef CVM__CLIENT__H__
#define CVM__CLIENT__H__

#include "facts.h"
#include "errors.h"

extern const char* cvm_account_split_chars;
extern const char* cvm_ucspi_domain(void);
extern int cvm_authenticate(const char* module, const char* account,
			    const char* domain, const char** credentials,
			    int parse_account);
extern int cvm_fact_str(int number, const char** data);
extern int cvm_fact_uint(int number, unsigned long* data);

extern int cvm_setugid(void);
extern int cvm_setenv(void);

#endif
