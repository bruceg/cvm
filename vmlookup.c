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
#include "cvm-vmailmgr.h"

extern int cvm_getpwnam(const char*, struct passwd**);

static const char* users_path = "users/cdb";
static int users_fd = -1;
static struct cdb users_cdb;
static struct stat users_stat;

static str account;
static str baseuser;

static dict vdomains;
static struct stat vdomains_stat;
static const char* vdomains_path = "control/virtualdomains";

/* Results from looking up the user */
const char* pw_dir = 0;
uid_t pw_uid = 0;
gid_t pw_gid = 0;
const char* pw_name = 0;

static int map_lower(str* s)
{
  str_lower(s);
  return 1;
}

static int load_virtualdomains(const struct stat* s)
{
  if (chdir(qmail_root) == -1) return 0;
  if (s) {
    obuf_putsflush(&errbuf, "Reloading virtualdomains\n");
    vdomains_stat = *s;
  }
  else {
    obuf_putsflush(&errbuf, "Loading virtualdomains\n");
    if (stat(vdomains_path, &vdomains_stat) == -1) return 0;
  }
  dict_free(&vdomains, dict_str_free);
  return dict_load_map(&vdomains, vdomains_path, 0, ':', map_lower, 0);
}

int lookup_reinit(void)
{
  struct stat s;
  if (chdir(qmail_root) == -1) return CVME_CONFIG;
  /* If we can see the users CDB file... */
  if (stat(users_path, &s) != -1) {
    /* If it was either not open or has changed since the last open... */
    if (users_fd == -1 ||
	s.st_ino != users_stat.st_ino ||
	s.st_mtime != users_stat.st_mtime ||
	s.st_size != users_stat.st_size) {
      /* If it was previously open, close it */
      if (users_fd != -1) {
	close(users_fd);
	cdb_free(&users_cdb);
      }
      /* And re-open it */
      if ((users_fd = open(users_path, O_RDONLY)) != -1) {
	fstat(users_fd, &users_stat);
	cdb_init(&users_cdb, users_fd);
      }
    }
  }
  else if (users_fd != -1) {
    close(users_fd);
    cdb_free(&users_cdb);
    users_fd = -1;
  }

  return 0;
}

int lookup_init(void)
{
  if (!str_truncate(&account, 0) ||
      !str_truncate(&domain, 0) ||
      !str_truncate(&baseuser, 0) ||
      !str_truncate(&virtuser, 0))
    return CVME_GENERAL;

  if (!load_virtualdomains(0)) return CVME_IO;

  return lookup_reinit();
}

int lookup_domain(void)
{
  dict_entry* e;
  int result = 0;
  struct stat s;

  DEBUG("cvm_account_domain='", cvm_account_domain, "'");
  if (!str_copys(&domain, cvm_account_domain)) return CVME_GENERAL;
  str_lower(&domain);
  DEBUG("domain='", domain.s, "'");
  if (stat(vdomains_path, &s) == -1) return CVME_IO;
  if (vdomains_stat.st_mtime != s.st_mtime ||
      vdomains_stat.st_ino != s.st_ino ||
      vdomains_stat.st_size != s.st_size)
    if (!load_virtualdomains(&s)) return CVME_IO;
  
  if ((e = dict_get(&vdomains, &domain)) == 0) {
    unsigned i;
    while ((i = str_findnext(&domain, '.', 1)) != (unsigned)-1) {
      str_lcut(&domain, i);
      DEBUG("retry domain='", domain.s, "'");
      if ((e = dict_get(&vdomains, &domain)) != 0) break;
    }
  }

  if (e == 0) {
    DEBUG("not in virtualdomains", 0, 0);
    if (!str_copys(&account, cvm_account_name)) result = CVME_GENERAL;
    str_truncate(&domain, 0);
  }
  else {
    DEBUG("found '", ((str*)e->data)->s, "'");
    if (!str_copy(&account, (str*)e->data) ||
	!str_catc(&account, '-') ||
	!str_cats(&account, cvm_account_name)) result = CVME_GENERAL;
  }

  return result;
}

static int lookup_userscdb(void)
{
  int i;
  const char* ptr;
  static str tmp;
  
  DEBUG("Looking up '", account.s, "' in users CDB");
  /* Check if the resulting account is a base UNIX user. */
  if (!str_copys(&tmp, "!")) return CVME_IO;
  if (!str_cat(&tmp, &account)) return CVME_IO;
  if (!str_catc(&tmp, 0)) return CVME_IO;
  if (cdb_find(&users_cdb, tmp.s, tmp.len)) {
    DEBUG("'", account.s, "' is local");
    return vpwentry_auth(0);
  }

  /* Now, look for increasingly shorter baseuser-virtuser pairs */
  i = account.len;
  while (i > 0 && (i = str_findprev(&account, '-', i-1)) != -1) {
    if (!str_copyb(&baseuser, account.s, i) ||
	!str_copyb(&virtuser, account.s+i+1, account.len-i-1))
      return CVME_GENERAL;
    /* The following will not expand the str, so it can't fail */
    str_copys(&tmp, "!");
    str_cat(&tmp, &baseuser);
    str_catc(&tmp, '-');
    DEBUG("baseuser='", baseuser.s, "'");
    DEBUG("virtuser='", virtuser.s, "'");
    DEBUG("key='", tmp.s, "'");
    switch (cdb_get(&users_cdb, &tmp, &tmp)) {
    case -1:
      DEBUG("cdb_get returned error", 0, 0);
      return CVME_IO;
    case 0:
      continue;
    default:
      ptr = tmp.s;
      pw_name = ptr;
      ptr += strlen(ptr) + 1;
      pw_uid = strtol(ptr, 0, 10);
      ptr += strlen(ptr) + 1;
      pw_gid = strtol(ptr, 0, 10);
      ptr += strlen(ptr) + 1;
      pw_dir = ptr;
      return 0;
    }
  }

  return vpwentry_auth(0);
}

static int lookup_passwd(void)
{
  int i;
  struct passwd* pw;
  int err;
  
  DEBUG("Looking up '", account.s, "'");
  /* Check if the resulting account is a base UNIX user. */  
  if ((err = cvm_getpwnam(account.s, &pw)) == 0) {
    DEBUG("'", pw->pw_name, "' is local");
    return vpwentry_auth(0);
  }
  if (err != CVME_PERMFAIL) return err;

  /* Now, look for increasingly shorter baseuser-virtuser pairs */
  i = account.len;
  err = CVME_PERMFAIL;
  while (i > 0 && (i = str_findprev(&account, '-', i-1)) != -1) {
    if (!str_copyb(&baseuser, account.s, i) ||
	!str_copyb(&virtuser, account.s+i+1, account.len-i-1))
      return CVME_GENERAL;
    DEBUG("baseuser='", baseuser.s, "'");
    DEBUG("virtuser='", virtuser.s, "'");
    if ((err = cvm_getpwnam(baseuser.s, &pw)) == 0) break;
    if (err != CVME_PERMFAIL) return err;
  }
  if (err == CVME_PERMFAIL)
    return vpwentry_auth(0);
  if (err != 0) return err;

  pw_dir = pw->pw_dir;
  pw_uid = pw->pw_uid;
  pw_gid = pw->pw_gid;
  pw_name = pw->pw_name;
  
  return 0;
}

int lookup_baseuser(void)
{
  if (users_fd == -1)
    return lookup_passwd();
  else
    return lookup_userscdb();
}

int lookup_virtuser(void)
{
  int err;
  int fd;
  struct cdb cdb;

  memset(&cdb, 0, sizeof cdb);
  str_lower(&virtuser);
  /* Found a virtual user, authenticate it. */
  if (chdir(pw_dir) == -1) return CVME_IO;
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
