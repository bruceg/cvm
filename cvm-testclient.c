/* cvm/cvm-testclient.c - Diagnostic CVM client
 * Copyright (C) 2005  Bruce Guenter <bruceg@em.ca>
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
#include <stdio.h>
#include "v2client.h"

const char usage[] =
"usage: cvm-testclient cvmodule account domain\n"
"   or: cvm-testclient cvmodule account domain password\n";

int main(int argc, char** argv)
{
  int i;
  unsigned long u;

  switch (argc) {
  case 4:
    i = cvm_authenticate_lookup(argv[1], argv[2], argv[3], 0);
    break;
  case 5:
    i = cvm_authenticate_password(argv[1], argv[2], argv[3], argv[4], 0);
    break;
  default:
    fputs(usage, stderr);
    return 1;
  }
  
  if (i) {
    printf("Authentication failed, error #%d (%s)\n", i,
	   (i < cvm_nerr) ? cvm_errlist[i] : "Unknown error code");
    return i;
  }

  printf("user name:        %s\n"
	 "user ID:          %ld\n"
	 "group ID:         %ld\n"
	 "real name:        %s\n"
	 "directory:        %s\n"
	 "shell:            %s\n"
	 "group name:       %s\n"
	 "system user name: %s\n"
	 "system directory: %s\n"
	 "domain:           %s\n"
	 "mailbox path:     %s\n",
	 cvm_fact_username, cvm_fact_userid, cvm_fact_groupid,
	 cvm_fact_realname, cvm_fact_directory,
	 cvm_fact_shell, cvm_fact_groupname,
	 cvm_fact_sys_username, cvm_fact_sys_directory,
	 cvm_fact_domain, cvm_fact_mailbox);
  while (cvm_fact_uint(CVM_FACT_SUPP_GROUPID, &u) == 0)
    printf("supp. group ID:   %ld\n", u);
  return 0;
}
