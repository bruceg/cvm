/* vmlookup.c - vmailmgr CVM lookup routines
 * Copyright (C) 2004  Bruce Guenter <bruceg@em.ca>
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
#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
  return 0;
}

int lookup_init(void)
{
  if (!str_truncate(&account, 0) ||
      !str_truncate(&domain, 0) ||
      !str_truncate(&baseuser, 0) ||
      !str_truncate(&virtuser, 0))
    return CVME_GENERAL;

  if (qmail_lookup_init() != 0)
    return CVME_IO;

  return 0;
}

int lookup_virtuser(void)
{
  int err;
  int fd;
  struct cdb cdb;

  DEBUG("cvm domain = '", cvm_module_credentials[CVM_CRED_DOMAIN].s, "'");
  switch (qmail_lookup_cvm(&vmuser, &domain, &baseuser, &virtuser)) {
  case -1: return CVME_IO;
  case 0:  return CVME_PERMFAIL;
  }
  if (virtuser.len == 0)
    return CVME_PERMFAIL;

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
    /* Only handle the default user when in lookup mode, as
       authenticating the default user shouldn't happen. */
    if (cvm_module_lookup_secret != 0) {
      switch (cdb_get(&cdb, &default_user, &vpwdata)) {
      case -1:
	DEBUG("cdb_get returned error", 0, 0);
	err = CVME_IO;
	break;
      case 0:
	DEBUG("cdb_get failed", 0, 0);
	err = CVME_PERMFAIL;
	break;
      default:
	err = 0;
      }
    }
    else
      err = CVME_PERMFAIL;
    break;
  default:
    err = 0;
  }
  cdb_free(&cdb);
  close(fd);
  return err;
}
