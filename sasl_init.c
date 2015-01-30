#include <stdlib.h>
#include <string.h>
#include <bglibs/ucspi.h>
#include "sasl.h"
#include "sasl_internal.h"

static struct sasl_mechanism mechanisms[4] = {
  { "LOGIN",    "CVM_SASL_PLAIN",    0, sasl_login_start,    0 },
  { "PLAIN",    "CVM_SASL_PLAIN",    0, sasl_plain_start,    0 },
  { "CRAM-MD5", "CVM_SASL_CRAM_MD5", 0, sasl_cram_md5_start, 0 },
  { 0,           0,                  0, 0,                   0 },
};

const struct sasl_mechanism* sasl_mechanisms;

static void set_mech(struct sasl_mechanism* mech, const char* cvm,
		     struct sasl_mechanism** prev,
		     struct sasl_mechanism** first)
{
  mech->cvm = cvm;
  if (*prev != 0)
    (*prev)->next = mech;
  else if (*first == 0)
    *prev = *first = mech;
}

int sasl_init(struct sasl_state* ss)
{
  struct sasl_mechanism* prev;
  struct sasl_mechanism* first;
  struct sasl_mechanism* mech;
  const char* tmp;
  for (mech = mechanisms, first = 0, prev = 0; mech->name != 0; ++mech) {
    if ((tmp = getenv(mech->var)) != 0)
      set_mech(mech, tmp, &prev, &first);
  }
  /* backwards compatibility for $CVM_SASL_LOGIN */
  if (!mechanisms[0].cvm && (tmp = getenv("CVM_SASL_LOGIN")) != 0) {
    prev = 0;
    first = 0;
    set_mech(&mechanisms[0], tmp, &prev, &first);
  }
  sasl_mechanisms = first;
  memset(ss, 0, sizeof *ss);
  ss->domain = ucspi_localhost();
  return 1;
}
