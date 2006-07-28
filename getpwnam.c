/* cvm/getpwnam.c - Handles getpwnam+getspnam+getuserpw combinations
 * Copyright (C)2006  Bruce Guenter <bruce@untroubled.org>
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
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

#include <sysdeps.h>

#include "module.h"

#ifdef HASSPNAM
#include <shadow.h>
static struct spwd* spw;
#endif

#ifdef HASUSERPW
#include <userpw.h>
static struct userpw* uwp;
#endif

static char* actbuf = 0;
static unsigned actlen = 0;

static const char* copyact(const char* account)
{
  unsigned len;
  char *ptr;
  if ((len = strlen(account)) > actlen) {
    if ((actbuf = realloc(actbuf, len+1)) == 0) return 0;
    actlen = len;
  }
  for (ptr = actbuf; *account != 0; ++ptr, ++account)
    *ptr = isupper(*account) ? tolower(*account) : *account;
  *ptr = 0;
  return actbuf;
}
    
int cvm_getpwnam(const char* account, struct passwd** pwp)
{
  struct passwd* pw;

  account = copyact(account);
  if ((pw = getpwnam(account)) == 0)
    return (errno == ETXTBSY) ? CVME_IO : CVME_PERMFAIL;

#ifdef HASUSERPW
  if ((upw = getuserpw(account)) == 0) {
    if (errno == ETXTBSY) return CVME_IO;
  }
  else if (upw->upw_passwd)
    pw->pw_passwd = upw->upw_passwd;
#endif

#ifdef HASSPNAM
  if ((spw = getspnam(account)) == 0) {
    if (errno == ETXTBSY) return CVME_IO;
  }
  else if (spw->sp_pwdp)
    pw->pw_passwd = spw->sp_pwdp;
#endif

  *pwp = pw;
  return 0;
}
