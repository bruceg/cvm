#ifndef CVM__MODULE__H__
#define CVM__MODULE__H__

#include "credentials.h"
#include "errors.h"
#include "facts.h"

#include <str/str.h>

#define BUFSIZE 512

#define LOG_OK 'y'
#define LOG_FAIL 'n'
#define LOG_ERROR '?'

extern const char program[];

#if 0
extern const char* cvm_account_name;
extern const char* cvm_account_domain;
#endif
extern const char* cvm_lookup_secret;

extern unsigned char outbuffer[BUFSIZE];
extern unsigned outbuflen;
extern unsigned char inbuffer[BUFSIZE+1];
extern unsigned inbuflen;

extern void cvm_fact_start(void);
extern int cvm_fact_str(unsigned number, const char* data);
extern int cvm_fact_uint(unsigned number, unsigned long data);
extern void cvm_fact_end(unsigned code);

extern void init_request(void);
extern int handle_request(void);

extern void log_startup(void);
extern void log_request(void);
extern void log_shutdown(void);

extern int cvm_parse_domain(const char* seperators);

extern str cvm_credentials[CVM_CRED_MAX+1];

extern int cvm_module_init(void);
extern int cvm_lookup(void);
extern int cvm_authenticate(void);
extern int cvm_results(void);
extern void cvm_module_stop(void);

#endif
