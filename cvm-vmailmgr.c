/* cvm-vmailmgr.c - Direct file access vmailmgr CVM
 * Copyright (C) 2001-2002  Bruce Guenter <bruceg@em.ca>
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

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];
const char program[] = "cvm-vmailmgr";
const int try_default = 0;

extern char* crypt(const char* key, const char* salt);
extern char* md5_crypt(const char* key, const char* salt);

static const char* null_crypt(const char* pass)
{
  static str buffer;
  if (!str_copys(&buffer, "$0$")) return 0;
  if (!str_cats(&buffer, pass)) return 0;
  return buffer.s;
}

int vpwentry_auth(const vpwentry* vpw)
{
  const char* pass;
  const char* enc;

  if (vpw == 0)
    return CVME_PERMFAIL;

  if (lock_disabled && !vpw->is_mailbox_enabled) {
    DEBUG("Mailbox is disabled", 0, 0);
    return CVME_PERMFAIL;
  }
  if (vpw->pass.len < 3) {
    DEBUG("Encoded password is too short", 0, 0);
    return CVME_PERMFAIL;
  }
  pass = vpw->pass.s;
  if (pass[0] == '$' && pass[2] == '$') {
    switch (pass[1]) {
    case '0':
      enc = null_crypt(cvm_credentials[0]);
      break;
    case '1':
      enc = md5_crypt(cvm_credentials[0], pass);
      break;
    default:
      enc = crypt(cvm_credentials[0], pass);
    }
  }
  else
    enc = crypt(cvm_credentials[0], pass);
  if (strcmp(enc, pass) == 0)
    return 0;
  DEBUG("authentication denied", 0, 0);
  return CVME_PERMFAIL;
}
