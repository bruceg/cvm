#ifndef AUTHENTICATOR__CLIENT__H__
#define AUTHENTICATOR__CLIENT__H__

#include "facts.h"
#include "errors.h"

int cvm_authenticate(const char* module, const char** credentials);
int cvm_fact_str(int number, const char** data);
int cvm_fact_uint(int number, unsigned long* data);

#endif
