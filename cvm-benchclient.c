/* cvm/cvm-benchclient.c - CVM benchmark client
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
#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main(int argc, char** argv)
{
  int a;
  unsigned long count;
  unsigned long i;
  char* ptr;
  
  if (argc < 5) {
    printf("usage: testclient count cvmodule account credential [credential ...]\n");
    return 1;
  }
  
  if ((count = strtoul(argv[1], &ptr, 10)) == 0 || *ptr) {
    printf("Invalid number for count");
    return 1;
  }
  
  for (i = 0; i < count; i++) {
    a = authenticate(argv[2], (const char**)(argv+3));
    if (a) {
      printf("Authentication failed, error #%d\n", a);
      return a;
    }
  }
  return 0;
}
