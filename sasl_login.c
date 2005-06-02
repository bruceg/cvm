#include "sasl.h"
#include "sasl_internal.h"

static const char cusername[] = "Username:";
static const char cpassword[] = "Password:";

static int response2(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  if (response->len == 0)
    return SASL_RESP_BAD;
  return sasl_authenticate_plain(ss, ss->username.s, response->s);
  (void)challenge;
}

static int response1(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  if (response->len == 0)
    return SASL_RESP_BAD;
  if (!str_copy(&ss->username, response) ||
      !str_copys(challenge, cpassword))
    return SASL_TEMP_FAIL;
  ss->response = response2;
  return SASL_CHALLENGE;
}

int sasl_login_start(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  ss->response = response1;
  if (response)
    return response1(ss, response, challenge);
  if (!str_copys(challenge, cusername))
    return SASL_TEMP_FAIL;
  return SASL_CHALLENGE;
}
