#include "sasl.h"
#include "sasl_internal.h"

static str username;
static const char* cusername = "Username:";
static const char* cpassword = "Password:";

int sasl_login_start(const str* response, str* challenge)
{
  str_truncate(&username, 0);
  if (response) return sasl_login_response(response, challenge);
  if (!str_copys(challenge, cusername)) return SASL_TEMP_FAIL;
  return SASL_CHALLENGE;
}

int sasl_login_response(const str* response, str* challenge)
{
  if (response->len == 0) return SASL_RESP_BAD;
  if (username.len == 0) {
    if (!str_copy(&username, response) ||
	!str_copys(challenge, cpassword)) return SASL_TEMP_FAIL;
    return SASL_CHALLENGE;
  }
  return sasl_authenticate_plain(username.s, response->s);
}
