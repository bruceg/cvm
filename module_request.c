#include <string.h>
#include "module.h"

const char* account_name;
char inbuffer[BUFSIZE];
unsigned inbuflen;

#define ADVANCE do{ \
  char* tmp; \
  if ((tmp = memchr(buf, 0, len)) == 0) return CVME_BAD_CLIDATA; \
  ++tmp; \
  if (len < (unsigned)(tmp - buf)) return CVME_BAD_CLIDATA; \
  len -= tmp - buf; \
  buf = tmp; \
} while (0)

static int parse_input(void)
{
  unsigned i;
  char* buf;
  unsigned len;
  
  buf = inbuffer;
  len = inbuflen;
  account_name = buf;
  ADVANCE;
  for (i = 0; i < credential_count; ++i) {
    credentials[i] = buf;
    ADVANCE;
  }
  return 0;
}

int handle_request(void)
{
  int code;
  if ((code = parse_input()) != 0) return code;
  fact_start();
  if ((code = authenticate()) != 0) return code;
  fact_str(FACT_USERNAME, fact_username);
  fact_uint(FACT_USERID, fact_userid);
  fact_uint(FACT_GROUPID, fact_groupid);
  if (fact_realname) fact_str(FACT_REALNAME, fact_realname);
  fact_str(FACT_DIRECTORY, fact_directory);
  fact_str(FACT_SHELL, fact_shell);
  if (fact_groupname) fact_str(FACT_GROUPNAME, fact_groupname);
  if (fact_sys_username) fact_str(FACT_SYS_USERNAME, fact_sys_username);
  if (fact_sys_directory) fact_str(FACT_SYS_DIRECTORY, fact_sys_directory);
  return 0;
}
