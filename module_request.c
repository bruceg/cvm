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

str cvm_account_name;
str cvm_account_domain;

char inbuffer[BUFSIZE+1];
unsigned inbuflen;

#define ADVANCE do{ \
  char* tmp; \
  if ((tmp = memchr(buf, 0, len)) == 0) return CVME_BAD_CLIDATA; \
  ++tmp; \
  if (len < (unsigned)(tmp - buf)) return CVME_BAD_CLIDATA; \
  len -= tmp - buf; \
  buf = tmp; \
} while (0)

#define COPY_ADVANCE(STR) do{ \
  if (!str_copys(STR, buf)) return CVME_IO; \
  if ((STR)->len >= len) return CVME_BAD_CLIDATA; \
  len -= (STR)->len+1; \
  buf += (STR)->len+1; \
} while (0)

static int parse_input(void)
{
  unsigned i;
  char* buf;
  unsigned len;

  if (inbuffer[0] != CVM_PROTOCOL) return CVME_BAD_CLIDATA;
  
  /* Prevent buffer run-off by ensuring there is at least one NUL byte */
  inbuffer[BUFSIZE] = 0;
  buf = inbuffer + 1;
  len = inbuflen - 1;

  COPY_ADVANCE(&cvm_account_name);
  COPY_ADVANCE(&cvm_account_domain);

  for (i = 0; i < cvm_credential_count; ++i)
    COPY_ADVANCE(&cvm_credentials[i]);
  
  if (*buf != 0) return CVME_BAD_CLIDATA;
  return 0;
}

int handle_request(void)
{
  int code;
  if ((code = parse_input()) != 0) return code;
  cvm_fact_start();
  if ((code = cvm_authenticate()) != 0) return code;
  cvm_fact_str(CVM_FACT_USERNAME, cvm_fact_username);
  cvm_fact_uint(CVM_FACT_USERID, cvm_fact_userid);
  cvm_fact_uint(CVM_FACT_GROUPID, cvm_fact_groupid);
  if (cvm_fact_realname)
    cvm_fact_str(CVM_FACT_REALNAME, cvm_fact_realname);
  cvm_fact_str(CVM_FACT_DIRECTORY, cvm_fact_directory);
  cvm_fact_str(CVM_FACT_SHELL, cvm_fact_shell);
  if (cvm_fact_groupname)
    cvm_fact_str(CVM_FACT_GROUPNAME, cvm_fact_groupname);
  if (cvm_fact_sys_username)
    cvm_fact_str(CVM_FACT_SYS_USERNAME, cvm_fact_sys_username);
  if (cvm_fact_sys_directory)
    cvm_fact_str(CVM_FACT_SYS_DIRECTORY, cvm_fact_sys_directory);
  if (cvm_fact_domain)
    cvm_fact_str(CVM_FACT_DOMAIN, cvm_fact_domain);
  return 0;
}
