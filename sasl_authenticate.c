#include "v1client.h"
#include "sasl.h"
#include "sasl_internal.h"

int sasl_authenticate(const char* account, const char** creds)
{
  switch (cvm_authenticate(sasl_mech_cvm, account, sasl_domain, creds, 1))
  {
  case 0: return SASL_AUTH_OK;
  case CVME_PERMFAIL: return SASL_AUTH_FAILED;
  default: return SASL_TEMP_FAIL;
  }
}
