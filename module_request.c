/* cvm/module_request.c - Request parsing code
 * Copyright (C) 2001  Bruce Guenter <bruceg@em.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
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
