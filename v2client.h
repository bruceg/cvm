#ifndef CVM__V2CLIENT__H__
#define CVM__V2CLIENT__H__

#include <str/str.h>

#include "facts.h"
#include "errors.h"

#define CVM_BUFSIZE 512

struct cvm_credential
{
  unsigned type;
  str value;
};

extern const char* cvm_account_split_chars;
extern const char* cvm_ucspi_domain(void);
extern int cvm_authenticate(const char* module, unsigned count,
			    struct cvm_credential* credentials,
			    unsigned account, unsigned domain);
extern int cvm_fact_str(unsigned number, const char** data, unsigned* length);
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

/* Wrapper functions. */
extern int cvm_authenticate_lookup(const char* module,
				   const char* account,
				   const char* domain,
				   int split_account);
extern int cvm_authenticate_password(const char* module,
				     const char* account,
				     const char* domain,
				     const char* password,
				     int split_account);

#endif
