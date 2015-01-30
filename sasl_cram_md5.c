#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <bglibs/systime.h>
#include "v2client.h"
#include "sasl.h"
#include "sasl_internal.h"

static const unsigned char hex2bin[256] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 0-15 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 16-31 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 32-47 */
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1, /* 48-63 */
  -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 64-79 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 80-95 */
  -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 96-111 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 112-127 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 128-143 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 144-159 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 160-175 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 176-191 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 192-207 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 208-223 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 224-239 */
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 240-255 */
};

static int response1(struct sasl_state* ss,
		     const str* response, str* challenge)
{
  char binresp[16];
  const str binrespstr = { binresp, 16, 0 };      
  unsigned i;
  unsigned j;
  if (response->len == 0) return SASL_RESP_BAD;
  if ((i = str_findfirst(response, ' ')) == (unsigned)-1
      || response->len - i != 33)
    return SASL_RESP_BAD;
  response->s[i] = 0;
  for (j = 0; j < 32; j += 2)
    binresp[j/2] = hex2bin[(unsigned char)response->s[i+j]] << 4
      | hex2bin[(unsigned char)response->s[i+j+1]];
  return sasl_authenticate_cram(ss, response->s, "CRAM-MD5",
				&ss->init, &binrespstr);
  (void)challenge;
}

int sasl_cram_md5_start(struct sasl_state* ss,
			const str* response, str* challenge)
{
  struct timeval tv;
  const char* hostname;
  
  if (response)
    return SASL_RESP_NOTALLOWED;
  ss->response = response1;
  if ((hostname = cvm_client_ucspi_domain()) == 0)
    hostname = "unknown";
  if (gettimeofday(&tv, 0) == -1 ||
      !str_copys(&ss->init, "<") ||
      !str_cati(&ss->init, getpid()) ||
      !str_catc(&ss->init, '.') ||
      !str_catu(&ss->init, tv.tv_sec) ||
      !str_catc(&ss->init, '.') ||
      !str_catuw(&ss->init, tv.tv_usec, 6, '0') ||
      !str_catc(&ss->init, '@') ||
      !str_cats(&ss->init, hostname) ||
      !str_catc(&ss->init, '>') ||
      !str_copy(challenge, &ss->init))
    return SASL_TEMP_FAIL;
  return SASL_CHALLENGE;
}
