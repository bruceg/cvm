/* cvm-vmlookup.c - vmailmgr lookup-only CVM
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
#include <string.h>
#include <unistd.h>

#include <cvm/module.h>
#include <vmailmgr/vpwentry.h>

#include "cvm-vmailmgr.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];
const char program[] = "cvm-vmlookup";
const int try_default = 1;

int vpwentry_auth(const vpwentry* vpw)
{
  if (secret != 0)
    if (strcmp(secret, cvm_credentials[0]) != 0)
      return CVME_BAD_MODDATA;
  if (vpw == 0)
    return CVME_PERMFAIL;
  return 0;
}
