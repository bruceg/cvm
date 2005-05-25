/* cvm/cvm-v1benchclient.c - CVM benchmark client
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
#include <stdlib.h>
#include <fmt/number.h>
#include <msg/msg.h>
#include "v1client.h"

const char program[] = "cvm-v1benchclient";
const int msg_show_pid = 0;

int main(int argc, char** argv)
{
  int a;
  unsigned long count;
  unsigned long i;
  char* ptr;
  char num[FMT_ULONG_LEN];
  
  if (argc < 6)
    die3(1, "usage: ", program, " count cvmodule account domain credential [credential ...]\n");

  if ((count = strtoul(argv[1], &ptr, 10)) == 0 || *ptr)
    die2(1, "Invalid number for count: ", argv[1]);

  for (i = 0; i < count; i++) {
    if ((a = cvm_client_authenticate(argv[2], argv[3], argv[4],
			      (const char**)(argv+5), 0)) != 0) {
      num[fmt_udec(num, a)] = 0;
      die5(a, "Authentication failed, error #", num, " (",
	   (a < cvm_nerr) ? cvm_errlist[i] : "Unknown error code", ")");
    }
  }
  return 0;
}
