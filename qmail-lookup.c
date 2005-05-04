/* qmail-lookup.c - qmail CVM lookup routines
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
#include <stdlib.h>
#include <str/str.h>

#include "module.h"
#include "qmail.h"

static const char* missingdomain = 0;
static const char* missinguser = "alias";

int qmail_lookup_init(void)
{
  if (qmail_init() != 0
      || qmail_users_init() != 0
      || qmail_domains_init() != 0)
    return -1;

  if ((missingdomain = getenv("CVM_QMAIL_MISSINGDOMAIN")) != 0)
    if (*missingdomain == 0)
      missingdomain = "localhost";
  if ((missinguser = getenv("CVM_QMAIL_MISSINGUSER")) == 0
      || *missinguser == 0)
    missinguser = "alias";

  return 0;
}

int qmail_lookup_cvm(struct qmail_user* user,
		     str* domain,
		     str* username,
		     str* ext)
{
  static str prefix;
  static str fullname;

  if (cvm_account_domain[0] == 0)
    cvm_account_domain = qmail_envnoathost;

  if (qmail_users_reinit() != 0
      || qmail_domains_reinit() != 0)
    return -1;

  switch (qmail_domains_lookup(cvm_account_domain, domain, &prefix)) {
  case -1:
    return -1;
  case 0:
    if (missingdomain == 0)
      return 0;
    if (!str_copys(domain, missingdomain)
	|| !str_copys(&fullname, missinguser))
      return -1;
    break;
  default:
    fullname.len = 0;
    if (prefix.len > 0)
      if (!str_copy(&fullname, &prefix)
	  || !str_catc(&fullname, '-'))
	return -1;
    if (!str_cats(&fullname, cvm_account_name))
      return -1;
  }

  switch (qmail_users_lookup_split(user, fullname.s, username, ext)) {
  case -1:
    return -1;
  case 0:
    return 0;
  }
  return 1;
}
