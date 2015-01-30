/* cvm/module_local_main.c - Local CVM server module main routine
 * Copyright (C) 2010  Bruce Guenter <bruce@untroubled.org>
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
#include <stdlib.h>
#include <unistd.h>

#include <bglibs/msg.h>

#include "module.h"

const int msg_show_pid = 0;

void usage(void)
{
  die3(1, "usage: ", program, "-local /PATH/TO/SOCKET");
}

extern int local_main(const char*);

int main(int argc, char** argv)
{
  if (argc != 2) usage();
  cvm_module_init_request();
  return local_main(argv[1]);
}
