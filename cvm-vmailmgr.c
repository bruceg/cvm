/* cvm-vmailmgr.c - Direct file access vmailmgr CVM
 * Copyright (C) 2004  Bruce Guenter <bruceg@em.ca>
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
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "module.h"
#include <vmailmgr/vpwentry.h>

#include "cvm-vmailmgr.h"

const char program[] = "cvm-vmailmgr";

extern char* crypt(const char* key, const char* salt);
extern char* md5_crypt(const char* key, const char* salt);

static const char* null_crypt(const char* pass)
{
  static str buffer;
  if (!str_copys(&buffer, "$0$")) return 0;
  if (!str_cats(&buffer, pass)) return 0;
  return buffer.s;
}

int cvm_module_authenticate(void)
{
  const char* stored;
  const char* enc;
  const char* pass;

  CVM_CRED_REQUIRED(PASSWORD);
  
  if (lock_disabled && !vpw.is_mailbox_enabled) {
    DEBUG("Mailbox is disabled", 0, 0);
    return CVME_PERMFAIL;
  }
  if (vpw.pass.len < 3) {
    DEBUG("Encoded password is too short", 0, 0);
    return CVME_PERMFAIL;
  }
  stored = vpw.pass.s;
  pass = cvm_module_credentials[CVM_CRED_PASSWORD].s;
  if (stored[0] == '$' && stored[2] == '$') {
    switch (stored[1]) {
    case '0':
      enc = null_crypt(pass);
      break;
    case '1':
      enc = md5_crypt(pass, stored);
      break;
    default:
      enc = crypt(pass, stored);
    }
  }
  else
    enc = crypt(pass, stored);
  if (strcmp(enc, stored) == 0)
    return 0;
  DEBUG("authentication denied", 0, 0);
  return CVME_PERMFAIL;
}
