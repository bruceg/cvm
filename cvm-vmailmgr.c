/* cvm-vmailmgr.c - Direct file access vmailmgr CVM
 * Copyright (C) 2010  Bruce Guenter <bruce@untroubled.org>
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

#include <bglibs/cdb.h>
#include <bglibs/cdb.h>
#include <bglibs/dict.h>
#include <bglibs/dict.h>
#include <bglibs/iobuf.h>
#include <bglibs/path.h>
#include <bglibs/str.h>
#include <vmailmgr/vpwentry.h>

#include "module.h"
#include "qmail.h"
#include "cvm-vmailmgr.h"

const char program[] = "cvm-vmailmgr";

extern char* crypt(const char* key, const char* salt);
extern char* md5_crypt(const char* key, const char* salt);

static const char* null_crypt(const char* pass)
{
  static str buffer;
  if (!str_copys(&buffer, "$0$")) return 0;
  if (!str_cats(&buffer, pass)) return 0;
  return buffer.s;
}

str default_user = {0,0,0};
str domain = {0,0,0};
str virtuser = {0,0,0};
str vpwdata = {0,0,0};

const char* pwfile = 0;

static int lock_disabled;
static int do_autoconvert;
static vpwentry vpw;

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

int cvm_module_init(void)
{
  const char* tmp;
  memset(&vpw, 0, sizeof vpw);
  if ((pwfile = getenv("VMAILMGR_PWFILE")) == 0) pwfile = "passwd.cdb";
  if ((tmp = getenv("VMAILMGR_DEFAULT")) == 0) tmp = "+";
  lock_disabled = getenv("VMAILMGR_LOCK_DISABLED") != 0;
  do_autoconvert = getenv("VMAILMGR_AUTOCONVERT") != 0;
  if (!str_copys(&default_user, tmp)) return CVME_GENERAL;
  if (getenv("DEBUG") != 0) show_debug = 1;
  return lookup_init();
}

static str directory;

/* Account name is either "baseuser-virtuser" or "virtuser@domain" */
int cvm_module_lookup(void)
{
  int err;

  if ((err = lookup_virtuser()) != 0) return err;

  if (!vpwentry_import(&vpw, &virtuser, &vpwdata)) {
    DEBUG("Could not import virtual password data", 0, 0);
    return CVME_IO;
  }

  return 0;
}

int cvm_module_authenticate(void)
{
  const char* stored;
  const char* enc;
  const char* pass;

  CVM_CRED_REQUIRED(PASSWORD);
  
  if (lock_disabled && !vpw.is_mailbox_enabled) {
    DEBUG("Mailbox is disabled", 0, 0);
    cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 0);
    return CVME_PERMFAIL;
  }
  if (vpw.pass.len < 3) {
    DEBUG("Encoded password is too short", 0, 0);
    cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 0);
    return CVME_PERMFAIL;
  }
  stored = vpw.pass.s;
  pass = cvm_module_credentials[CVM_CRED_PASSWORD].s;
  if (stored[0] == '$' && stored[2] == '$') {
    switch (stored[1]) {
    case '0':
      enc = null_crypt(pass);
      break;
    case '1':
      enc = md5_crypt(pass, stored);
      break;
    default:
      enc = crypt(pass, stored);
    }
  }
  else
    enc = crypt(pass, stored);
  if (strcmp(enc, stored) == 0) {
    if (do_autoconvert
	&& (stored[0] != '$' || stored[1] != '0' || stored[2] != '$'))
      return vmailmgr_autoconvert();
    return 0;
  }
  DEBUG("authentication denied", 0, 0);
  cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 0);
  return CVME_PERMFAIL;
}

int cvm_module_results(void)
{
  if (!str_copy(&directory, &vmuser.homedir)) return CVME_IO;
  if (!path_merge(&directory, vpw.directory.s)) return CVME_IO;
  cvm_fact_username = vpw.name.s;
  cvm_fact_userid = vmuser.uid;
  cvm_fact_groupid = vmuser.gid;
  cvm_fact_realname = 0;
  cvm_fact_directory = directory.s;
  cvm_fact_shell = 0;
  cvm_fact_sys_username = vmuser.user.s;
  cvm_fact_sys_directory = vmuser.homedir.s;
  cvm_fact_domain = domain.s;
  cvm_fact_mailbox = directory.s;
  return 0;
}

void cvm_module_stop(void)
{
}
