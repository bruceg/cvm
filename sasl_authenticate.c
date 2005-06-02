#include <string.h>
#include "v2client.h"
#include "credentials.h"
#include "sasl.h"
#include "sasl_internal.h"

static int setup(int count,
		 struct cvm_credential* creds,
		 const char* account,
		 const char* domain)
{
  memset(creds, 0, count * sizeof creds[0]);
  creds[0].type = CVM_CRED_ACCOUNT;
  if (!str_copys(&creds[0].value, account))
    return 0;
  if (domain == 0)
    domain = "";
  creds[1].type = CVM_CRED_DOMAIN;
  if (!str_copys(&creds[1].value, domain))
    return 0;
  return cvm_client_split_account(&creds[0].value, &creds[1].value);
}

static void free_creds(int count, struct cvm_credential* creds)
{
  while (count > 1)
    str_free(&creds[--count].value);
}

static int authenticate_free(const char* cvm,
			     int count, struct cvm_credential* creds)
{
  int result;
  result = cvm_client_authenticate(cvm, count, creds);
  free_creds(count, creds);
  return result;
}

int sasl_authenticate_plain(struct sasl_state* ss,
			    const char* account, const char* password)
{
  struct cvm_credential creds[3];
  if (!setup(3, creds, account, ss->domain))
    return SASL_TEMP_FAIL;
  creds[2].type = CVM_CRED_PASSWORD;
  if (!str_copys(&creds[2].value, password))
    return SASL_TEMP_FAIL;
  switch (authenticate_free(ss->mech->cvm, 3, creds)) {
  case 0: return SASL_AUTH_OK;
  case CVME_PERMFAIL: return SASL_AUTH_FAILED;
  default: return SASL_TEMP_FAIL;
  }
}

int sasl_authenticate_cram(struct sasl_state* ss,
			   const char* account, const char* cram_type,
			   const str* challenge, const str* response)
{
  struct cvm_credential creds[5];
  if (!setup(5, creds, account, ss->domain))
    return SASL_TEMP_FAIL;
  creds[2].type = CVM_CRED_CHALLENGE;
  if (!str_copy(&creds[2].value, challenge))
    return SASL_TEMP_FAIL;
  creds[3].type = CVM_CRED_RESPONSE;
  if (!str_copy(&creds[3].value, response))
    return SASL_TEMP_FAIL;
  creds[4].type = CVM_CRED_RESPONSE_TYPE;
  if (!str_copys(&creds[4].value, cram_type))
    return SASL_TEMP_FAIL;
  switch (authenticate_free(ss->mech->cvm, 5, creds)) {
  case 0: return SASL_AUTH_OK;
  case CVME_PERMFAIL: return SASL_AUTH_FAILED;
  default: return SASL_TEMP_FAIL;
  }
}
