/* lookup.c - vmailmgr CVM lookup routines
 * Copyright (C) 2003  Bruce Guenter <bruceg@em.ca>
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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cdb/cdb.h>
#include <cdb/str.h>
#include <dict/dict.h>
#include <dict/load.h>
#include <iobuf/iobuf.h>
#include <path/path.h>
#include <str/str.h>
#include <vmailmgr/vpwentry.h>

#include "module.h"
#include "qmail.h"
#include "cvm-vmailmgr.h"

static str account;
static str baseuser;

/* Results from looking up the user */
struct qmail_user vmuser;

int lookup_reinit(void)
{
  if (qmail_users_reinit() != 0)
    return CVME_IO;
  if (qmail_vdomains_reinit() != 0)
    return CVME_IO;
  return 0;
}

int lookup_init(void)
{
  if (!str_truncate(&account, 0) ||
      !str_truncate(&domain, 0) ||
      !str_truncate(&baseuser, 0) ||
      !str_truncate(&virtuser, 0))
    return CVME_GENERAL;

  if (qmail_users_init() != 0)
    return CVME_IO;
  if (qmail_vdomains_init() != 0)
    return CVME_IO;

  return 0;
}

int lookup_domain(void)
{
  static str prefix;

  DEBUG("cvm_account_domain='", cvm_account_domain, "'");
  switch (qmail_vdomains_lookup(cvm_account_domain, &domain, &prefix)) {
  case -1: return CVME_GENERAL;
  case 0:
    if (!str_copys(&account, cvm_account_name))
      return CVME_GENERAL;
    break;
  default:
    if (!str_copy(&account, &prefix) ||
	!str_catc(&account, '-') ||
	!str_cats(&account, cvm_account_name))
      return CVME_GENERAL;
  }
  return 0;
}

int lookup_baseuser(void)
{
  switch (qmail_users_lookup_split(&vmuser, account.s, &baseuser, &virtuser)) {
  case -1: return CVME_GENERAL;
  case 0: return vpwentry_auth(0);
  }
  if (virtuser.len == 0)
    return vpwentry_auth(0);
  return 0;
}

int lookup_virtuser(void)
{
  int err;
  int fd;
  struct cdb cdb;

  memset(&cdb, 0, sizeof cdb);
  str_lower(&virtuser);
  /* Found a virtual user, authenticate it. */
  if (chdir(vmuser.homedir.s) == -1) return CVME_IO;
  if ((fd = open(pwfile, O_RDONLY)) == -1) return CVME_IO;
  cdb_init(&cdb, fd);
  switch (cdb_get(&cdb, &virtuser, &vpwdata)) {
  case -1:
    DEBUG("cdb_get returned error", 0, 0);
    err = CVME_IO;
    break;
  case 0:
    DEBUG("cdb_get failed", 0, 0);
    if (try_default) {
      switch (cdb_get(&cdb, &default_user, &vpwdata)) {
      case -1:
	DEBUG("cdb_get returned error", 0, 0);
	err = CVME_IO;
	break;
      case 0:
	DEBUG("cdb_get failed", 0, 0);
	err = vpwentry_auth(0);
	break;
      default:
	err = 0;
      }
    }
    else
      err = vpwentry_auth(0);
    break;
  default:
    err = 0;
  }
  cdb_free(&cdb);
  close(fd);
  return err;
}
