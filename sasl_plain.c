#include <string.h>
#include "sasl.h"
#include "sasl_internal.h"

static int response1(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  unsigned i;
  unsigned j;
  if (response->len == 0)
    return SASL_RESP_BAD;
  if ((i = str_findfirst(response, 0)) == (unsigned)-1)
    return SASL_RESP_BAD;
  ++i;
  if ((j = str_findnext(response, 0, i)) == (unsigned)-1)
    return SASL_RESP_BAD;
  ++j;
  return sasl_authenticate_plain(ss, response->s+i, response->s+j);
  (void)challenge;
}

int sasl_plain_start(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  if (response)
    return response1(ss, response, challenge);
  if (!str_truncate(challenge, 0))
    return SASL_TEMP_FAIL;
  ss->response = response1;
  return SASL_CHALLENGE;
}
