/* cvm/cvm-pwfile.c - Alternate passwd file CVM module
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
#include <sysdeps.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

#include <iobuf/ibuf.h>
#include <msg/msg.h>
#include <pwcmp/client.h>

#include "module.h"

const char program[] = "cvm-pwfile";

static const char* pwfilename;

int cvm_module_init(void)
{
  if ((pwfilename = getenv("CVM_PWFILE_PATH")) == 0) return CVME_CONFIG;
  if (!pwcmp_start(getenv("CVM_PWFILE_PWCMP"))) return CVME_GENERAL;
  return 0;
}

static int parse_rest(char* rest)
{
  /* Format: "UID:GID:NAME,other:DIR:SHELL" */
  char* tmp;
  cvm_fact_userid = strtoul(rest, &tmp, 10);
  if (*tmp != ':') return 0;
  rest = tmp + 1;

  cvm_fact_groupid = strtoul(rest, &tmp, 10);
  if (*tmp != ':') return 0;
  rest = tmp + 1;

  cvm_fact_realname = rest;
  if ((tmp = strchr(rest, ',')) != 0) {
    *tmp++ = 0;
    rest = tmp;
  }

  if ((tmp = strchr(rest, ':')) == 0) return 0;
  *tmp++ = 0;
  cvm_fact_directory = rest = tmp;

  if ((tmp = strchr(rest, ':')) == 0) return 0;
  *tmp++ = 0;
  cvm_fact_shell = rest = tmp;

  cvm_fact_groupname = 0;
  return 1;
}

static char* passwd;
static char* rest;
static str line;

int cvm_module_lookup(void)
{
  ibuf pwfile;
  long namelen;

  if (cvm_module_credentials[CVM_CRED_ACCOUNT].s == 0)
    return CVME_NOCRED;
  passwd = 0;
  namelen = cvm_module_credentials[CVM_CRED_ACCOUNT].len;

  if (!ibuf_open(&pwfile, pwfilename, 0)) return CVME_IO;
  while (ibuf_getstr(&pwfile, &line, LF)) {
    line.s[--line.len] = 0;
    if (strncasecmp(cvm_module_credentials[CVM_CRED_ACCOUNT].s, line.s, namelen) == 0
	&& line.s[namelen] == ':') {
      passwd = line.s + namelen;
      *passwd++ = 0;
      break;
    }
  }
  ibuf_close(&pwfile);
  if (passwd == 0) return CVME_PERMFAIL;

  if ((rest = strchr(passwd, ':')) == 0 || rest == passwd)
    return CVME_PERMFAIL;
  *rest++ = 0;
  return 0;
}

int cvm_module_authenticate(void)
{
  CVM_CRED_REQUIRED(PASSWORD);
  switch (pwcmp_check(cvm_module_credentials[CVM_CRED_PASSWORD].s, passwd)) {
  case 0: return 0;
  case -1: return CVME_IO | CVME_FATAL;
  default: return CVME_PERMFAIL;
  }
}

int cvm_module_results(void)
{
  cvm_fact_username = line.s;
  if (!parse_rest(rest)) return CVME_CONFIG;
  return 0;
}

void cvm_module_stop(void)
{
  pwcmp_stop();
}
