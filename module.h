#ifndef AUTHENTICATOR__MODULE__H__
#define AUTHENTICATOR__MODULE__H__

#include "facts.h"

extern const char* service_name;
extern const char* account_name;
extern const char* credentials;

void read_input(void);
const char* cred_string(void);

void fact_str(int number, const char* data);
void fact_uint(int number, unsigned data);
void fact_end(void);

extern void authenticate(void);

#endif
