/* main.c - vmailmgr CVM main routines
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

const char* secret;
str default_user = {0,0,0};
int lock_disabled;

str domain = {0,0,0};
str virtuser = {0,0,0};
str vpwdata = {0,0,0};

const char* qmail_root = "/var/qmail";

const char* pwfile = 0;

#define DEBUG(A,B,C) debug(__FUNCTION__, __LINE__, A, B, C)
static int show_debug = 0;
void debug(const char* func, int line,
	   const char* a, const char* b, const char* c)
{
  if (!show_debug) return;
  obuf_puts(&errbuf, func);
  obuf_putc(&errbuf, '(');
  obuf_puti(&errbuf, line);
  obuf_puts(&errbuf, "): ");
  if (a) obuf_puts(&errbuf, a);
  if (b) obuf_puts(&errbuf, b);
  if (c) obuf_puts(&errbuf, c);
  obuf_putsflush(&errbuf, "\n");
}

int cvm_auth_init(void)
{
  const char* tmp;
  if ((tmp = getenv("QMAIL_ROOT")) != 0) qmail_root = tmp;
  if ((pwfile = getenv("VMAILMGR_PWFILE")) == 0) pwfile = "passwd.cdb";
  if ((tmp = getenv("VMAILMGR_DEFAULT")) == 0) tmp = "+";
  secret = getenv("VMLOOKUP_SECRET");
  lock_disabled = getenv("VMAILMGR_LOCK_DISABLED") != 0;
  if (!str_copys(&default_user, tmp)) return CVME_GENERAL;
  if (getenv("DEBUG") != 0) show_debug = 1;
  return lookup_init();
}

static vpwentry vpw;
static str directory;

/* Account name is either "baseuser-virtuser" or "virtuser@domain" */
int cvm_lookup(void)
{
  int err;

  if ((err = lookup_reinit()) != 0) return err;
  if ((err = lookup_domain()) != 0) return err;
  if ((err = lookup_baseuser()) != 0) return err;
  if ((err = lookup_virtuser()) != 0) return err;

  if (!vpwentry_import(&vpw, &virtuser, &vpwdata)) {
    DEBUG("Could not import virtual password data", 0, 0);
    return CVME_IO;
  }

  return 0;
}

int cvm_authenticate(void)
{
  return vpwentry_auth(&vpw);
}

int cvm_results(void)
{
  if (!str_copys(&directory, pw_dir)) return CVME_IO;
  if (!path_merge(&directory, vpw.directory.s)) return CVME_IO;
  cvm_fact_username = vpw.name.s;
  cvm_fact_userid = pw_uid;
  cvm_fact_groupid = pw_gid;
  cvm_fact_realname = 0;
  cvm_fact_directory = directory.s;
  cvm_fact_shell = 0;
  cvm_fact_sys_username = pw_name;
  cvm_fact_sys_directory = pw_dir;
  cvm_fact_domain = domain.s;
  cvm_fact_mailbox = directory.s;
  return 0;
}

void cvm_auth_stop(void)
{
}
