#ifndef CVM__MODULE__H__
#define CVM__MODULE__H__

#include "facts.h"
#include "errors.h"

#define BUFSIZE 512

#define LOG_OK 'y'
#define LOG_FAIL 'n'
#define LOG_ERROR '?'

extern const char* cvm_account_name;
extern char outbuffer[BUFSIZE];
extern unsigned outbuflen;
extern char inbuffer[BUFSIZE];
extern unsigned inbuflen;

extern void cvm_fact_start(void);
extern int cvm_fact_str(int number, const char* data);
extern int cvm_fact_uint(int number, unsigned long data);
extern void cvm_fact_end(int code);

extern int handle_request(void);

extern void log_startup(void);
extern void log_request(void);
extern void log_shutdown(void);

/* The following need to be provided by the module.
 * The "credentials" global is filled by the input handling code. */
extern const unsigned cvm_credential_count;
extern const char* cvm_credentials[];
extern int cvm_auth_init(void);
extern int cvm_authenticate(void);

#endif
