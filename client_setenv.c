/* cvm/client_setenv.c - CVM client standard setenv calls
 * Copyright (C) 2001  Bruce Guenter <bruceg@em.ca>
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
#include <string.h>
#include "client.h"
#include "setenv.h"

static char* utoa(unsigned i)
{
  static char buf[32];
  char* ptr;
  
  ptr = buf + sizeof buf - 1;

  *ptr-- = 0;
  if (!i)
    *ptr-- = '0';
  else {
    while (i) {
      *ptr-- = (i % 10) + '0';
      i /= 10;
    }
  }
  return strdup(ptr + 1);
}

int cvm_setenv(void)
{
  if (setenv("USER", cvm_fact_username, 1) != 0) return 0;
  if (setenv("UID", utoa(cvm_fact_userid), 1) != 0) return 0;
  if (setenv("GID", utoa(cvm_fact_groupid), 1) != 0) return 0;
  if (cvm_fact_realname &&
      setenv("NAME", cvm_fact_realname, 1) != 0) return 0;
  if (setenv("HOME", cvm_fact_directory, 1) != 0) return 0;
  if (cvm_fact_shell &&
      setenv("SHELL", cvm_fact_shell, 1) != 0) return 0;
  if (cvm_fact_groupname &&
      setenv("GROUP", cvm_fact_groupname, 1) != 0) return 0;
  if (cvm_fact_domain &&
      setenv("DOMAIN", cvm_fact_domain, 1) != 0) return 0;
  if (cvm_fact_mailbox &&
      setenv("MAILBOX", cvm_fact_mailbox, 1) != 0) return 0;
  return 1;
}
