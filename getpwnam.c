/* cvm/getpwnam.c - Handles getpwnam+getspnam+getuserpw combinations
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
#include "module.h"
#include "hasspnam.h"
#include "hasuserpw.h"

#ifdef HASGETSPNAM
#include <shadow.h>
static struct spwd* spw;
#endif

#ifdef HASUSERPW
#include <userpw.h>
static struct userpw* uwp;
#endif

int cvm_getpwnam(const char* account, struct passwd** pwp)
{
  struct passwd* pw;
  
  pw = getpwnam(account);
  if (!pw)
    return (errno == ETXTBSY) ? CVME_IO : CVME_PERMFAIL;

#ifdef HASUSERPW
  upw = getuserpw(account);
  if (upw) {
    if (upw->upw_passwd)
      pw->pw_passwd = upw->upw_passwd;
  }
  else
    if (errno == ETXTBSY) return CVME_IO;
#endif

#ifdef HASGETSPNAM
  spw = getspnam(account);
  if (spw) {
    if (spw->sp_pwdp)
      pw->pw_passwd = spw->sp_pwdp;
  }
  else
    if (errno == ETXTBSY) return CVME_IO;
#endif

  *pwp = pw;
  return 0;
}