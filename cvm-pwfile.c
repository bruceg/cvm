/* cvm/cvm-pwfile.c - Alternate passwd file CVM module
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
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "module.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

extern char* crypt(const char* key, const char* salt);

static const char* pwfilename;

int cvm_auth_init(void)
{
  if ((pwfilename = getenv("CVM_PWFILE_PATH")) == 0) return CVME_CONFIG;
  return 0;
}

int cvm_authenticate(void)
{
  FILE* pwfile;
  struct passwd* pw;
  char* tmp;

  if ((pwfile = fopen(pwfilename, "r")) == 0) return CVME_IO;
  while ((pw = fgetpwent(pwfile)) != 0) {
    if (strcasecmp(cvm_account_name, pw->pw_name) == 0) break;
  }
  fclose(pwfile);
  
  if (pw == 0 || pw->pw_passwd == 0) return CVME_PERMFAIL;
  if (strcmp(crypt(cvm_credentials[0], pw->pw_passwd), pw->pw_passwd) != 0)
    return CVME_PERMFAIL;

  if ((tmp = strchr(pw->pw_gecos, ',')) != 0)
    *tmp = 0;

  cvm_fact_username = pw->pw_name;
  cvm_fact_userid = pw->pw_uid;
  cvm_fact_groupid = pw->pw_gid;
  cvm_fact_realname = pw->pw_gecos;
  cvm_fact_directory = pw->pw_dir;
  cvm_fact_shell = pw->pw_shell;
  cvm_fact_groupname = 0;
  
  return 0;
}
