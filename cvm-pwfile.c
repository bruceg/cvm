/* cvm/cvm-pwfile.c - Alternate passwd file CVM module
 * Copyright (C) 2001,2003  Bruce Guenter <bruceg@em.ca>
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
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwcmp/client.h>
#include "module.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

static const char* pwfilename;

int cvm_auth_init(void)
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

int cvm_authenticate(void)
{
  FILE* pwfile;
  char* passwd;
  char* rest;
  long namelen;
  char line[1024];

  passwd = 0;
  namelen = strlen(cvm_account_name);

  if ((pwfile = fopen(pwfilename, "r")) == 0) return CVME_IO;
  while (fgets(line, sizeof line, pwfile) != 0) {
    long len = strlen(line);
    if (line[len-1] != '\n') continue;
    line[len-1] = 0;
    if (strncasecmp(cvm_account_name, line, namelen) == 0 &&
	line[namelen] == ':') {
      passwd = line + namelen;
      *passwd++ = 0;
      break;
    }
  }
  fclose(pwfile);
  if (passwd == 0) return CVME_PERMFAIL;

  if ((rest = strchr(passwd, ':')) == 0 || rest == passwd)
    return CVME_PERMFAIL;
  *rest++ = 0;
  switch (pwcmp_check(cvm_credentials[0], passwd)) {
  case 0: break;
  case -1: return CVME_IO | CVME_FATAL;
  default: return CVME_PERMFAIL;
  }

  cvm_fact_username = line;
  if (!parse_rest(rest)) return CVME_CONFIG;
  return 0;
}

void cvm_auth_stop(void)
{
  pwcmp_stop();
}
