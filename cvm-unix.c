/* cvm/cvm-unix.c - UNIX/POSIX-standard CVM module
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
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "hasspnam.h"
#include "hasuserpw.h"
#include "module.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

#ifdef HASGETSPNAM
#include <shadow.h>
#endif

#ifdef HASUSERPW
#include <userpw.h>
#endif

extern char* crypt(const char* key, const char* salt);

int cvm_auth_init(void)
{
  return 0;
}

int cvm_authenticate(void)
{
  struct passwd* pw;
  struct group* gr;
#ifdef HASUSERPW
  struct userpw* upw;
#endif
#ifdef HASGETSPNAM
  struct spwd* spw;
#endif
  const char* cpw;
  char* tmp;
  cpw = 0;
  
  pw = getpwnam(cvm_account_name);
  if (pw && pw->pw_passwd)
    cpw = pw->pw_passwd;
  else
    return (errno == ETXTBSY) ? CVME_IO : CVME_PERMFAIL;

#ifdef HASUSERPW
  upw = getuserpw(cvm_account_name);
  if (upw) {
    if (upw->upw_passwd)
      cpw = upw->upw_passwd;
  }
  else
    if (errno == ETXTBSY) return CVME_IO;
#endif

#ifdef HASGETSPNAM
  spw = getspnam(cvm_account_name);
  if (spw) {
    if (spw->sp_pwdp)
      cpw = spw->sp_pwdp;
  }
  else
    if (errno == ETXTBSY) return CVME_IO;
#endif

  if (!cpw) return CVME_PERMFAIL;
  if (strcmp(crypt(cvm_credentials[0], cpw), cpw)) return CVME_PERMFAIL;

  if ((tmp = strchr(pw->pw_gecos, ',')) != 0)
    *tmp = 0;

  cvm_fact_username = pw->pw_name;
  cvm_fact_userid = pw->pw_uid;
  cvm_fact_groupid = pw->pw_gid;
  cvm_fact_realname = pw->pw_gecos;
  cvm_fact_directory = pw->pw_dir;
  cvm_fact_shell = pw->pw_shell;

  cvm_fact_uint(CVM_FACT_SUPP_GROUPID, pw->pw_gid);
  if (cvm_fact_groupname) free((char*)cvm_fact_groupname);
  cvm_fact_groupname = 0;
  setgrent();
  while ((gr = getgrent()) != 0) {
    if (gr->gr_gid == pw->pw_gid)
      cvm_fact_groupname = strdup(gr->gr_name);
    else {
      unsigned i;
      for (i = 0; gr->gr_mem[i]; i++)
	if (strcmp(gr->gr_mem[i], pw->pw_name) == 0) {
	  cvm_fact_uint(CVM_FACT_SUPP_GROUPID, gr->gr_gid);
	  break;
	}
    }
  }
  endgrent();
  
  return 0;
}
