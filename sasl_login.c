#include "sasl.h"
#include "sasl_internal.h"

static const char cusername[] = "Username:";
static const char cpassword[] = "Password:";

int sasl_login_response(struct sasl_state* ss,
			const str* response, str* challenge)
{
  if (response->len == 0)
    return SASL_RESP_BAD;
  if (ss->username.len == 0) {
    if (!str_copy(&ss->username, response) ||
	!str_copys(challenge, cpassword))
      return SASL_TEMP_FAIL;
    return SASL_CHALLENGE;
  }
  return sasl_authenticate_plain(ss, ss->username.s, response->s);
}

int sasl_login_start(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  ss->username.len = 0;
  ss->response = sasl_login_response;
  if (response)
    return sasl_login_response(ss, response, challenge);
  if (!str_copys(challenge, cusername))
    return SASL_TEMP_FAIL;
  return SASL_CHALLENGE;
}
