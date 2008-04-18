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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iobuf/ibuf.h>
#include <str/iter.h>

#include "module.h"
#include "qmail.h"

const char program[] = "cvm-qmail";

static struct qmail_user user;
static str domain;
static str username;
static str ext;
static str programs;

int cvm_module_init(void)
{
  const char* tmp;
  if (qmail_lookup_init() == -1)
    return CVME_IO;
  if ((tmp = getenv("CVM_QMAIL_LOOKASIDE")) != 0) {
    if (!str_copys(&programs, tmp))
      return CVME_IO | CVME_FATAL;
    str_subst(&programs, ' ', 0);
  }

  return 0;
}

static int lookup_programs(const str* path)
{
  static str dotqmail;
  striter line;
  striter progname;
  const char* start;
  const char* end;
  unsigned long left;

  if (!ibuf_openreadclose(path->s, &dotqmail))
    return -1;
  striter_loop(&line, &dotqmail, '\n') {
    /* skip over spaces preceding '|' */
    for (start = line.startptr, left = line.len;
	 left > 0 && isspace(*start);
	 --left, ++start)
      ;
    if (left > 0 && *start == '|') {
      /* skip spaces preceding the program name */
      for (++start, --left;
	   left > 0 && isspace(*start);
	   --left, ++start)
	;
      /* the program name ends at the first space */
      for (end = start;
	   left > 0 && !isspace(*end);
	   --left, ++end)
	;
      if (end > start) {
	striter_loop(&progname, &programs, 0) {
	  if ((unsigned long)(end - start) == progname.len
	      && memcmp(progname.startptr, start, progname.len) == 0)
	    return 1;
	}
      }
    }
  }
  return 0;
}

/* Account name is either "baseuser-virtuser" or "virtuser@domain" */
int cvm_module_lookup(void)
{
  static str path;

  switch (qmail_lookup_cvm(&user, &domain, &username, &ext)) {
  case -1:
    return CVME_IO;
  case 0:
    break;
  case 1:
    cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 1);
    return CVME_PERMFAIL;
  default:
    cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 0);
    return CVME_PERMFAIL;
  }

  switch (qmail_dotfile_exists(&user, ext.s, &path)) {
  case -1:
    return CVME_IO;
  case 0:
    cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 0);
    return CVME_PERMFAIL;
  }

  if (programs.len > 0) {
    switch (lookup_programs(&path)) {
    case -1:
      return CVME_IO;
    case 0:
      break;
    default:
      cvm_module_fact_uint(CVM_FACT_OUTOFSCOPE, 1);
      return CVME_PERMFAIL;
    }
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
