#ifndef AUTHENTICATOR__CLIENT__H__
#define AUTHENTICATOR__CLIENT__H__

#include "facts.h"
#include "errors.h"

int authenticate(const char* module, const char** credentials);
int fact_str(int number, const char** data);
int fact_uint(int number, unsigned* data);

#endif
