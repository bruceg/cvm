/* cvm/module_local_main.c - Local CVM server module main routine
 * Copyright (C) 2003  Bruce Guenter <bruceg@em.ca>
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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "module.h"

void usage(void)
{
  fprintf(stderr, "usage: %s /PATH/TO/SOCKET\n", program);
  exit(1);
}

extern int local_main(const char*);

int main(int argc, char** argv)
{
  if (argc != 2) usage();
  return local_main(argv[1]);
}
