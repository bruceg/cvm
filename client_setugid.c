/* cvm/client_setugid.c - CVM client standard setuid/setgid call
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
#include <sys/types.h>
#include <unistd.h>
#include "client.h"

int cvm_setugid(void)
{
  if (chdir(cvm_fact_directory) == -1) return 0;
  if (setgid(cvm_fact_groupid) == -1) return 0;
  if (setuid(cvm_fact_userid) == -1) return 0;
  return 1;
}
