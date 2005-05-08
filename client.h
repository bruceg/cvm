#ifndef CVM__CLIENT__H__
#define CVM__CLIENT__H__

#include "facts.h"
#include "errors.h"

#define CVM_BUFSIZE 512

extern const char* cvm_account_split_chars;
extern const char* cvm_ucspi_domain(void);
extern int cvm_authenticate(const char* module, const char* account,
			    const char* domain, const char** credentials,
			    int parse_account);
extern int cvm_fact_str(unsigned number, const char** data);
extern int cvm_fact_uint(unsigned number, unsigned long* data);

extern int cvm_setugid(void);
extern int cvm_setenv(void);

extern unsigned cvm_xfer_command(const char* module,
				 unsigned char buffer[CVM_BUFSIZE],
				 unsigned* buflen);
extern unsigned cvm_xfer_local(const char* path,
			       unsigned char buffer[CVM_BUFSIZE],
			       unsigned* buflen);
extern unsigned cvm_xfer_udp(const char* hostport,
			     unsigned char buffer[CVM_BUFSIZE],
			     unsigned* buflen);

#endif
