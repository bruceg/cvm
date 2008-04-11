#ifndef CVM__V1CLIENT__H__
#define CVM__V1CLIENT__H__

#include "facts.h"
#include "errors.h"

#define CVM_BUFSIZE 512

struct cvm_packet
{
  unsigned length;
  unsigned char data[CVM_BUFSIZE];
};

extern const char* cvm_client_account_split_chars;
extern const char* cvm_client_ucspi_domain(void);
extern int cvm_client_authenticate(const char* module, const char* account,
				   const char* domain, const char** credentials,
				   int parse_account);
extern int cvm_client_fact_str(unsigned number, const char** data);
extern int cvm_client_fact_uint(unsigned number, unsigned long* data);

extern int cvm_client_setugid(void);
extern int cvm_client_setenv(void);

extern unsigned cvm_xfer_command(const char* module,
				 struct cvm_packet* packet);
extern unsigned cvm_xfer_local(const char* path,
			       struct cvm_packet* packet);
extern unsigned cvm_xfer_udp(const char* hostport,
			     struct cvm_packet* packet);

#ifndef CVM_NOCOMPAT

/* Legacy definitions. */
#define cvm_account_split_chars    cvm_client_account_split_chars
#define cvm_ucspi_domain           cvm_client_ucspi_domain
#define cvm_setugid                cvm_client_setugid
#define cvm_setenv                 cvm_client_setenv
#define cvm_split_account          cvm_client_split_account

#ifndef CVM__MODULE__H__
# define cvm_authenticate          cvm_client_authenticate
# define cvm_fact_str              cvm_client_fact_str
# define cvm_fact_uint             cvm_client_fact_uint
#else
# undef cvm_authenticate
# undef cvm_fact_str
# undef cvm_fact_uint
#endif

#endif

#endif
