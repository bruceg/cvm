/* cvm/cvm-testclient.c - Diagnostic CVM client
 * Copyright (C)2006  Bruce Guenter <bruce@untroubled.org>
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
#include <string.h>
#include <fmt/number.h>
#include <iobuf/obuf.h>
#include <msg/msg.h>
#include "v2client.h"

const char program[] = "cvm-testclient";
const int msg_show_pid = 0;
const char usage[] = "\n"
"usage: cvm-testclient cvmodule account domain\n"
"   or: cvm-testclient cvmodule account domain password\n";

static void s(const char* name, const char* value)
{
  obuf_puts(&outbuf, name);
  obuf_puts(&outbuf, (value == 0) ? "(null)" : value);
  obuf_putc(&outbuf, '\n');
}

static void u(const char* name, unsigned long value)
{
  obuf_puts(&outbuf, name);
  obuf_putu(&outbuf, value);
  obuf_putc(&outbuf, '\n');
}

int main(int argc, char** argv)
{
  int i;
  unsigned long v;
  char num[FMT_ULONG_LEN];

  switch (argc) {
  case 4:
    i = cvm_client_authenticate_password(argv[1], argv[2], argv[3], 0, 1);
    break;
  case 5:
    i = cvm_client_authenticate_password(argv[1], argv[2], argv[3], argv[4], 1);
    break;
  default:
    die2(1, "Incorrect usage.", usage);
    return 1;
  }
  
  if (i) {
    num[fmt_udec(num, i)] = 0;
    msg5("Authentication failed, error #", num, " (",
	 (i < cvm_nerr) ? cvm_errlist[i] : "Unknown error code", ")");
    return i;
  }

  s("user name:        ", cvm_fact_username);
  u("user ID:          ", cvm_fact_userid);
  u("group ID:         ", cvm_fact_groupid);
  s("real name:        ", cvm_fact_realname);
  s("directory:        ", cvm_fact_directory);
  s("shell:            ", cvm_fact_shell);
  s("group name:       ", cvm_fact_groupname);
  s("system user name: ", cvm_fact_sys_username);
  s("system directory: ", cvm_fact_sys_directory);
  s("domain:           ", cvm_fact_domain);
  s("mailbox path:     ", cvm_fact_mailbox);
  while (cvm_client_fact_uint(CVM_FACT_SUPP_GROUPID, &v) == 0)
    u("supp. group ID:   ", v);
  obuf_flush(&outbuf);
  return 0;
}
