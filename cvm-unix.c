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
#include <pwd.h>
#include <string.h>
#include "hasspnam.h"
#include "hasuserpw.h"
#include "module.h"

const unsigned credential_count = 1;
const char* credentials[1];

#ifdef HASGETSPNAM
#include <shadow.h>
static struct spwd* spw;
#endif

#ifdef HASUSERPW
#include <userpw.h>
static struct userpw* upw;
#endif

static struct passwd* pw;

extern char* crypt(const char* key, const char* salt);

int authenticate(void)
{
  const char* cpw;
  cpw = 0;
  
  pw = getpwnam(account_name);
  if (pw && pw->pw_passwd)
    cpw = pw->pw_passwd;
  else
    return (errno == ETXTBSY) ? 111 : 100;

#ifdef HASUSERPW
  upw = getuserpw(account_name);
  if (upw && upw->upw_passwd)
    cpw = upw->upw_passwd;
  else
    if (errno == ETXTBSY) return 111;
#endif

#ifdef HASGETSPNAM
  spw = getspnam(account_name);
  if (spw && spw->sp_pwdp)
    cpw = spw->sp_pwdp;
  else
    if (errno == ETXTBSY) return 111;
#endif

  if (!cpw) return 100;
  if (strcmp(crypt(credentials[0], cpw), cpw)) return 100;

  fact_username = pw->pw_name;
  fact_userid = pw->pw_uid;
  fact_groupid = pw->pw_gid;
  fact_realname = pw->pw_gecos;
  fact_directory = pw->pw_dir;
  fact_shell = pw->pw_shell;
  return 0;
}
