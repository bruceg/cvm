/* qmail-users.c - qmail users/cdb lookup routines
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
#include <sysdeps.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cdb/cdb.h>
#include <cdb/str.h>
#include <iobuf/iobuf.h>
#include <str/str.h>

#include "qmail.h"

static str users_path;
static int users_fd = -1;
static struct cdb users_cdb;
static struct stat users_stat;

int qmail_users_reinit(void)
{
  struct stat s;
  /* If we can see the users CDB file... */
  if (stat(users_path.s, &s) != -1) {
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
      if ((users_fd = open(users_path.s, O_RDONLY)) != -1) {
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

int qmail_users_init(void)
{
  if (!str_copy2s(&users_path, qmail_root, "/users/cdb"))
    return -1;
  return qmail_users_reinit();
}

static int lookup_userscdb(struct qmail_user* u,
			   str* name, char dash)
{
  char* ptr;
  const char* end;
  const char* user;
  const char* home;
  int i;

  if (!str_spliceb(name, 0, 0, "!", 1)
      || (name->len > 1 && !str_catc(name, dash))) {
    errno = ENOMEM;
    return -1;
  }

  if ((i = cdb_get(&users_cdb, name, name)) <= 0)
    return i;

  /* tmp now contains:
   * user NUL uid NUL gid NUL home NUL dash NUL ext
   */
  errno = EDOM;
  ptr = name->s;
  end = name->s + name->len;
  user = ptr;
  if ((ptr += strlen(ptr) + 1) >= end) return -1;
  u->uid = strtoul(ptr, &ptr, 10);
  if (*ptr++ != 0 || ptr >= end) return -1;
  u->gid = strtoul(ptr, &ptr, 10);
  if (*ptr++ != 0 || ptr >= end) return -1;
  home = ptr;
  if ((ptr += strlen(ptr) + 1) >= end) return -1;
  if ((u->dash = *ptr) != 0) ++ptr;
  if (*ptr++ != 0 || ptr > end) return -1;

  if (!str_copys(&u->user, user)
      || !str_copys(&u->homedir, home)
      || !str_copyb(&u->ext, ptr, end-ptr)) {
    errno = ENOMEM;
    return -1;
  }

  return 1;
}

static int lookup_passwd(struct qmail_user* u, const str* namestr, char dash)
{
  const struct passwd* pw;
  const char* name;

  if (*(name = namestr->s) == 0)
    name = "alias";
  if ((pw = getpwnam(name)) == 0)
    return (errno == ETXTBSY) ? -1 : 0;

  if (!str_copys(&u->user, pw->pw_name)
      || !str_copys(&u->homedir, pw->pw_dir)
      || !str_copys(&u->ext, "")) {
    errno = ENOMEM;
    return -1;
  }
  u->uid = pw->pw_uid;
  u->gid = pw->pw_gid;
  u->dash = dash;
  return 1;
}

int qmail_users_lookup(struct qmail_user* u, const char* name, char dash)
{
  static str lname;
  if (!str_copys(&lname, name)){
    errno = ENOMEM;
    return -1;
  }
  str_lower(&lname);
  return (users_fd == -1)
    ? lookup_passwd(u, &lname, dash)
    : lookup_userscdb(u, &lname, dash);
}

int qmail_users_lookup_split(struct qmail_user* u, const char* name,
			     str* local, str* ext)
{
  static str account;
  int i;

  /* Check if the name is a base UNIX user. */
  if (!str_copys(local, name)) return -1;
  if (!str_copys(ext, "")) return -1;
  switch (qmail_users_lookup(u, name, 0)) {
  case -1: return -1;
  case 0:  break;
  default: return 1;
  }

  /* Now, look for increasingly shorter base-ext pairs */
  if (!str_copy(&account, local)) return -1;
  i = account.len;
  while (i > 0 && (i = str_findprev(&account, '-', i-1)) != -1) {
    if (!str_copyb(local, account.s, i)) return -1;
    if (!str_copyb(ext, account.s+i+1, account.len-i-1)) return -1;
    switch (qmail_users_lookup(u, local->s, '-')) {
    case -1: return -1;
    case 0: continue;
    default: return 1;
    }
  }

  switch (qmail_users_lookup(u, "", '-')) {
  case -1: return -1;
  case 0: return 0;
  }
  str_copyb(local, "", 0);
  if (!str_copy(ext, &account)) return -1;
  return 1;
}
