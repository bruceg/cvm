/* cvm/random.c - CVM random number generation
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

/* Derived from dns_random.c from djbdns-1.05, which was made public
 * domain as per http://cr.yp.to/distributors.html on 2007-12-28. */

#include <string.h>
#include <bglibs/systime.h>
#include <unistd.h>
#include <bglibs/uint32.h>
#include <bglibs/surfrand.h>
#include "random.h"

static struct surfrand state;

void cvm_random_init(void)
{
  struct timeval tv;
  uint32 data[32];

  gettimeofday(&tv, 0);
  data[0] += tv.tv_sec;
  data[1] += tv.tv_usec;
  data[2] = getpid();
  data[3] = getppid();

  surfrand_init(&state, data, 32);
}

void cvm_random_fill(unsigned char* buf, unsigned len)
{
  surfrand_fill(&state, buf, len);
}
