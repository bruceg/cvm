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
#include <str/str.h>

#include "module.h"
#include "qmail.h"

int qmail_lookup_init(void)
{
  if (qmail_init() != 0
      || qmail_users_init() != 0
      || qmail_vdomains_init() != 0)
    return -1;

  return 0;
}

int qmail_lookup_cvm(struct qmail_user* user,
		     str* domain,
		     str* username,
		     str* ext)
{
  static str prefix;
  static str fullname;

  if (qmail_users_reinit() != 0
      || qmail_vdomains_reinit() != 0)
    return -1;

  switch (qmail_vdomains_lookup(cvm_account_domain, domain, &prefix)) {
  case -1:
    return -1;
  case 0:
    if (!str_copys(&fullname, cvm_account_name))
      return -1;
    break;
  default:
    if (!str_copy(&fullname, &prefix) ||
	!str_catc(&fullname, '-') ||
	!str_cats(&fullname, cvm_account_name))
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
