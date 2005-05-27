#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#include <sysdeps.h>

#include "module.h"
#include "v2client.h"

const char program[] = "cvm-chain";

const char* chains[10];
int chain_count;

static void cvm_chain_init()
{
  cvm_fact_username = 0;
  cvm_fact_userid = -1;
  cvm_fact_groupid = -1;
  cvm_fact_directory = 0;
  cvm_fact_shell = 0;
  cvm_fact_realname = 0;
  cvm_fact_groupname = 0;
  cvm_fact_sys_username = 0;
  cvm_fact_sys_directory = 0;
  cvm_fact_domain = 0;
  cvm_fact_mailbox = 0;
}

int cvm_module_init(void)
{
  int i;
  char varname[] = "CVM_CHAIN#";

  chain_count = 0;
  for (i = 0; i <= 9; ++i) {
    varname[9] = i + '0';
    if ((chains[chain_count] = getenv(varname)) != 0)
      ++chain_count;
  }
  if (chain_count == 0)
    return CVME_CONFIG | CVME_FATAL;

  cvm_chain_init();
  return 0;
}

int cvm_module_lookup(void)
{
  int i;
  int credcount;
  int code;
  struct cvm_credential creds[CVM_CRED_MAX+1];

  for (i = credcount = 0; i <= CVM_CRED_MAX; ++i) {
    if (cvm_module_credentials[i].len > 0) {
      creds[credcount].type = i;
      creds[credcount].value = cvm_module_credentials[i];
      ++credcount;
    }
  }

  for (code = i = 0; i < chain_count && ((code & CVME_FATAL) == 0); i++) {
    cvm_chain_init();
    code = cvm_client_authenticate(chains[i], credcount, creds);
    if (code == 0)
      return 0;
  }
  return code;
}

int cvm_module_authenticate(void)
{
  return 0;
}

int cvm_module_results(void)
{
  cvm_client_setenv();
  return 0;
}


void cvm_module_stop(void)
{
}
