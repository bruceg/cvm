#include <string.h>
#include "sasl.h"
#include "sasl_internal.h"

int sasl_start(struct sasl_state* ss,
	       const char* mechanism, const str* initresponse, str* challenge)
{
  const struct sasl_mechanism* mech;
  for (mech = sasl_mechanisms; mech != 0; mech = mech->next) {
    if (strcasecmp(mechanism, mech->name) == 0) {
      ss->mech = mech;
      return mech->start(ss, initresponse, challenge);
    }
  }
  return SASL_NO_MECH;
}
