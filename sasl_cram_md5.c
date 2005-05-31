#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <systime.h>
#include "v2client.h"
#include "sasl.h"
#include "sasl_internal.h"

static str init;

int sasl_cram_md5_start(const str* response, str* challenge)
{
  struct timeval tv;
  const char* hostname;
  
  if (response) return SASL_RESP_NOTALLOWED;
  if ((hostname = cvm_client_ucspi_domain()) == 0) hostname = "unknown";
  if (gettimeofday(&tv, 0) == -1 ||
      !str_copys(&init, "<") ||
      !str_cati(&init, getpid()) ||
      !str_catc(&init, '.') ||
      !str_catu(&init, tv.tv_sec) ||
      !str_catc(&init, '.') ||
      !str_catuw(&init, tv.tv_usec, 6, '0') ||
      !str_catc(&init, '@') ||
      !str_cats(&init, hostname) ||
      !str_catc(&init, '>') ||
      !str_copy(challenge, &init)) return SASL_TEMP_FAIL;
  return SASL_CHALLENGE;
}

static unsigned char hex2bin[256] = {
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

int sasl_cram_md5_response(const str* response, str* challenge)
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
  return sasl_authenticate_cram(response->s, "CRAM-MD5", &init, &binrespstr);
  (void)challenge;
}
