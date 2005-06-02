#include <string.h>
#include <base64/base64.h>
#include <iobuf/iobuf.h>
#include <msg/msg.h>
#include <str/str.h>
#include "sasl.h"
#include "v2client.h"

int sasl_auth_init(struct sasl_auth* sa)
{
  if (sa->prefix == 0)
    sa->prefix = "";
  if (sa->suffix == 0)
    sa->suffix = "\r\n";
  if (sa->in == 0)
    sa->in = &inbuf;
  if (sa->out == 0)
    sa->out = &outbuf;
  return sasl_init(&sa->state);
}

int sasl_auth_caps(str* caps)
{
  const struct sasl_mechanism* smech;
  if (!sasl_mechanisms)
    return 0;

  if (!str_truncate(caps, 0) ||
      !str_copys(caps, "AUTH"))
    return -1;
  for (smech = sasl_mechanisms; smech != 0; smech = smech->next)
    if (!str_catc(caps, ' ') ||	!str_cats(caps, smech->name))
      return -1;
  return 1;
}

int sasl_auth2(struct sasl_auth* sa,
	       const char* mechanism,
	       const char* init_response)
{
  str challenge = {0,0,0};
  str challenge64 = {0,0,0};
  str response = {0,0,0};
  str response64 = {0,0,0};
  int i;
  str* iresponsestr;
  
  if (init_response != 0) {
    if (!str_truncate(&response, 0))
      return -1;
    if (!base64_decode_line(init_response, &response)) {
      msg3("SASL AUTH ", mechanism, " failed: bad response");
      str_free(&response);
      return SASL_RESP_BAD;
    }
    iresponsestr = &response;
  }
  else
    iresponsestr = 0;
  i = sasl_start(&sa->state, mechanism, iresponsestr, &challenge);
  while (i == SASL_CHALLENGE) {
    i = -1;
    if (str_truncate(&challenge64, 0)
	&& base64_encode_line(challenge.s, challenge.len, &challenge64)
	&& obuf_puts(sa->out, sa->prefix)
	&& obuf_putstr(sa->out, &challenge64)
	&& obuf_putsflush(sa->out, sa->suffix)
	&& ibuf_getstr_crlf(sa->in, &response64)) {
      if (response64.len == 0 || response64.s[0] == '*') {
	msg3("SASL AUTH ", mechanism, " failed: aborted");
	i = SASL_AUTH_FAILED;
      }
      else if (!str_truncate(&response, 0) ||
	       !base64_decode_line(response64.s, &response)) {
	msg3("SASL AUTH ", mechanism, " failed: bad response");
	i = SASL_RESP_BAD;
      }
      else
	i = sa->state.response(&sa->state, &response, &challenge);
    }
    else if (ibuf_eof(sa->in))
      i = SASL_RESP_EOF;
  }
  if (i == SASL_AUTH_OK) {
    str_truncate(&response, 0);
    str_copys(&response, "username=");
    str_cats(&response, cvm_fact_username);
    if (cvm_fact_sys_username != 0) {
      str_cats(&response, " sys_username=");
      str_cats(&response, cvm_fact_sys_username);
    }
    if (cvm_fact_domain != 0 && cvm_fact_domain[0] != 0) {
      str_cats(&response, " domain=");
      str_cats(&response, cvm_fact_domain);
    }
    msg4("SASL AUTH ", mechanism, " ", response.s);
    cvm_client_setenv();
  }
  else
    msg3("SASL AUTH ", mechanism, " failed");
  str_free(&response);
  str_free(&response64);
  str_free(&challenge);
  str_free(&challenge64);
  return i;
}

int sasl_auth1(struct sasl_auth* sa, const str* arg)
{
  str mechanism = {0,0,0};
  int s;
  if ((s = str_findfirst(arg, ' ')) != -1) {
    if (!str_copyb(&mechanism, arg->s, s))
      return -1;
    while (arg->s[s] == ' ')
      ++s;
    s = sasl_auth2(sa, mechanism.s, arg->s+s);
    str_free(&mechanism);
  }
  else
    s = sasl_auth2(sa, arg->s, 0);
  return s;
}

const char* sasl_auth_msg(int* code)
{
  int newcode;
  const char* msg;
  #define R(C,M) newcode=C; msg=M; break
  switch (*code) {
  case SASL_AUTH_FAILED: R(501,"Authentication failed.");
  case SASL_NO_MECH: R(504,"Unrecognized authentication mechanism.");
  case SASL_RESP_REQUIRED: R(535,"Response was required but not given.");
  case SASL_RESP_NOTALLOWED: R(535,"Initial response not allowed.");
  case SASL_RESP_BAD: R(501,"Could not decode the response.");
  case SASL_RESP_EOF: R(535,"End of file reached.");
  default: R(451,"Internal error.");
  }
  *code = newcode;
  return msg;
}
