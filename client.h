#ifndef AUTHENTICATOR__CLIENT__H__
#define AUTHENTICATOR__CLIENT__H__

#include "facts.h"
#include "errors.h"

#define CVM_PROTOCOL 1

extern const char* cvm_account_split_chars;
extern int cvm_authenticate(const char* module, const char* account,
			    const char* domain, const char** credentials,
			    int parse_account);
extern int cvm_fact_str(int number, const char** data);
extern int cvm_fact_uint(int number, unsigned long* data);

#endif
