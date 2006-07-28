/* cvm/client_setenv.c - CVM client standard setenv calls
 * Copyright (C) 2006  Bruce Guenter <bruce@untroubled.org>
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

#include "v1client.h"

static char* utoa_rec(unsigned i, char* buf)
{
  if (i < 10)
    *buf = i + '0';
  else {
    buf = utoa_rec(i / 10, buf);
    *buf = (i % 10) + '0';
  }
  *++buf = 0;
  return buf;
}

static char utoa_buf[32];

static char* utoa(unsigned i)
{
  utoa_rec(i, utoa_buf);
  return utoa_buf;
}

static int utoa_len(unsigned i)
{
  return utoa_rec(i, utoa_buf) - utoa_buf;
}

static int set_gids(void)
{
  unsigned long gid;
  long len;
  char* start;
  char* ptr;
  int result;
  
  len = 0;
  while (cvm_client_fact_uint(CVM_FACT_SUPP_GROUPID, &gid) == 0)
    len += utoa_len(gid) + 1;
  /* Don't set $GIDS if no supplementary group IDs were listed */
  if (len == 0) return 1;
  /* Reset to the start of facts list */
  cvm_client_fact_uint(-1, &gid);
  ptr = start = malloc(len);
  while (cvm_client_fact_uint(CVM_FACT_SUPP_GROUPID, &gid) == 0) {
    if (ptr > start) *ptr++ = ',';
    ptr = utoa_rec(gid, ptr);
  }
  result = setenv("GIDS", start, 1) == 0;
  free(start);
  return result;
}

int cvm_client_setenv(void)
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
      (setenv("MAIL", cvm_fact_mailbox, 1) != 0 ||
       setenv("MAILBOX", cvm_fact_mailbox, 1) != 0)) return 0;
  if (!set_gids()) return 0;
  return 1;
}
