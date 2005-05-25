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

extern const char* cvm_module_lookup_secret;

extern unsigned char cvm_module_outbuffer[BUFSIZE];
extern unsigned cvm_module_outbuflen;
extern unsigned char cvm_module_inbuffer[BUFSIZE+1];
extern unsigned cvm_module_inbuflen;

extern void cvm_module_fact_start(void);
extern int cvm_module_fact_str(unsigned number, const char* data);
extern int cvm_module_fact_uint(unsigned number, unsigned long data);
extern void cvm_module_fact_end(unsigned code);

extern void cvm_module_init_request(void);
extern int cvm_module_handle_request(void);

extern void cvm_module_log_startup(void);
extern void cvm_module_log_request(void);
extern void cvm_module_log_shutdown(void);

extern int cvm_module_parse_domain(const char* seperators);

extern str cvm_module_credentials[CVM_CRED_MAX+1];

extern int cvm_module_init(void);
extern int cvm_module_lookup(void);
extern int cvm_module_authenticate(void);
extern int cvm_module_results(void);
extern void cvm_module_stop(void);

#ifndef CVM_NOCOMPAT

/* Legacy definitions. */
#define cvm_lookup_secret      cvm_module_lookup_secret
#define outbuffer              cvm_module_outbuffer
#define outbuflen              cvm_module_outbuflen
#define inbuffer               cvm_module_inbuffer
#define inbuflen               cvm_module_inbuflen
#define cvm_fact_start         cvm_module_fact_start
#define cvm_fact_end           cvm_module_fact_end
#define init_request           cvm_module_init_request
#define handle_request         cvm_module_handle_request
#define log_startup            cvm_module_log_startup
#define log_request            cvm_module_log_request
#define log_shutdown           cvm_module_log_shutdown
#define cvm_parse_domain       cvm_module_parse_domain
#define cvm_credential_count   cvm_module_credential_count
#define cvm_credentials        cvm_module_credentials
#define cvm_lookup             cvm_module_lookup
#define cvm_results            cvm_module_results

#ifndef CVM__CLIENT__H__
# define cvm_authenticate      cvm_module_authenticate
# define cvm_fact_str          cvm_module_fact_str
# define cvm_fact_uint         cvm_module_fact_uint
#else
# undef cvm_authenticate
# undef cvm_fact_str
# undef cvm_fact_uint
#endif

#endif

#endif
