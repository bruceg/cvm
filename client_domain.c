/* cvm/client_domain.c - Determine domain from environment
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
#include <stdlib.h>
#include <string.h>
#include "client.h"

static long len = 0;
static char* var = 0;

const char* cvm_ucspi_domain(void)
{
  const char* proto;
  long newlen;
  if ((proto = getenv("PROTO")) == 0) proto = "TCP";
  newlen = strlen(proto) + 9;
  if (newlen > len) {
    free(var);
    var = malloc(newlen+1);
    len = newlen;
  }
  strcpy(var, proto);
  strcat(var, "LOCALHOST");
  return getenv(var);
}
