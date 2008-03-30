/* cvm-qmail.c - qmail lookup-only CVM
 * Copyright (C)2008  Bruce Guenter <bruce@untroubled.org>
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "module.h"
#include "qmail.h"

const char program[] = "cvm-qmail";

int cvm_module_init(void)
{
  if (qmail_lookup_init() == -1)
    return CVME_IO;
  return 0;
}

static struct qmail_user user;
static str domain;
static str username;
static str ext;

/* Account name is either "baseuser-virtuser" or "virtuser@domain" */
int cvm_module_lookup(void)
{
  switch (qmail_lookup_cvm(&user, &domain, &username, &ext)) {
  case -1:
    return CVME_IO;
  case 0:
    break;
  case 1:
    cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 1);
    /* Fall through */
  default:
    return CVME_PERMFAIL;
  }

  switch (qmail_dotfile_exists(&user, ext.s)) {
  case -1:
    return CVME_IO;
  case 0:
    return CVME_PERMFAIL;
  }

  return 0;
}

int cvm_module_authenticate(void)
{
  return CVME_CONFIG;
}

int cvm_module_results(void)
{
  cvm_fact_username = user.user.s;
  cvm_fact_userid = user.uid;
  cvm_fact_groupid = user.gid;
  cvm_fact_realname = 0;
  cvm_fact_directory = user.homedir.s;
  cvm_fact_shell = 0;
  cvm_fact_sys_username = user.user.s;
  cvm_fact_sys_directory = user.homedir.s;
  cvm_fact_domain = domain.s;
  cvm_fact_mailbox = user.homedir.s;
  return 0;
}

void cvm_module_stop(void)
{
}
