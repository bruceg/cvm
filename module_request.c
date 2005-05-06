/* cvm/module_request.c - Request parsing code
 * Copyright (C) 2005  Bruce Guenter <bruceg@em.ca>
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
#include <stdlib.h>
#include <string.h>
#include "module.h"
#include "protocol.h"

const char* cvm_account_name;
const char* cvm_account_domain;

unsigned char inbuffer[BUFSIZE+1];
unsigned inbuflen;

const char* cvm_lookup_secret = 0;
static const char* lookup_creds[1];
static unsigned credential_count;
static const char** credentials;

void init_request(void)
{
  /* Determine if the module is to operate in lookup mode, and if not
     set the local credential values appropriately. */
  if ((cvm_lookup_secret = getenv("CVM_LOOKUP_SECRET")) == 0) {
    credentials = cvm_credentials;
    credential_count = cvm_credential_count;
  }
  else {
    credentials = lookup_creds;
    credential_count = (*cvm_lookup_secret != 0);
  }
}

static int copy_advance(const char** ptr, char**buf, unsigned* len)
{
  char* tmp;
  if ((tmp = memchr(*buf, 0, *len)) == 0) return 0;
  ++tmp;
  if (*len < (unsigned)(tmp - *buf)) return 0;
  *ptr = *buf;
  *len -= tmp - *buf;
  *buf = tmp;
  return 1;
}

static int parse_input(void)
{
  unsigned i;
  char* buf;
  unsigned len;

  if (inbuffer[0] != CVM1_PROTOCOL) return CVME_BAD_CLIDATA;
  
  /* Prevent buffer run-off by ensuring there is at least one NUL byte */
  inbuffer[BUFSIZE] = 0;
  buf = inbuffer + 1;
  len = inbuflen - 1;

  if (!copy_advance(&cvm_account_name, &buf, &len)) return CVME_BAD_CLIDATA;
  if (!copy_advance(&cvm_account_domain, &buf, &len)) return CVME_BAD_CLIDATA;

  for (i = 0; i < credential_count; ++i)
    if (!copy_advance(&credentials[i], &buf, &len))
      return CVME_BAD_CLIDATA;

  if (len != 1) return CVME_BAD_CLIDATA;
  return 0;
}

int handle_request(void)
{
  int code;
  if ((code = parse_input()) != 0) return code;
  if (cvm_lookup_secret != 0 && *cvm_lookup_secret != 0) {
    if (credentials[0] == 0 ||
	strcmp(credentials[0], cvm_lookup_secret) != 0)
      return CVME_NOCRED;
  }
  if ((code = cvm_lookup()) != 0) return code;
  if (cvm_lookup_secret == 0)
    if ((code = cvm_authenticate()) != 0) return code;
  cvm_fact_start();
  if ((code = cvm_results()) != 0) return code;
  cvm_fact_str(CVM_FACT_USERNAME, cvm_fact_username);
  cvm_fact_uint(CVM_FACT_USERID, cvm_fact_userid);
  cvm_fact_uint(CVM_FACT_GROUPID, cvm_fact_groupid);
  if (cvm_fact_realname)
    cvm_fact_str(CVM_FACT_REALNAME, cvm_fact_realname);
  cvm_fact_str(CVM_FACT_DIRECTORY, cvm_fact_directory);
  if (cvm_fact_shell)
    cvm_fact_str(CVM_FACT_SHELL, cvm_fact_shell);
  if (cvm_fact_groupname)
    cvm_fact_str(CVM_FACT_GROUPNAME, cvm_fact_groupname);
  if (cvm_fact_sys_username)
    cvm_fact_str(CVM_FACT_SYS_USERNAME, cvm_fact_sys_username);
  if (cvm_fact_sys_directory)
    cvm_fact_str(CVM_FACT_SYS_DIRECTORY, cvm_fact_sys_directory);
  if (cvm_fact_domain)
    cvm_fact_str(CVM_FACT_DOMAIN, cvm_fact_domain);
  if (cvm_fact_mailbox)
    cvm_fact_str(CVM_FACT_MAILBOX, cvm_fact_mailbox);
  return 0;
}
