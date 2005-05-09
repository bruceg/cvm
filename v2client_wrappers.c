/* cvm/v2client_wrappers.c - CVM version 2 client library wrapper functions
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
#include "v2client.h"
#include "credentials.h"

static struct cvm_credential creds[3];

int cvm_authenticate_lookup(const char* module,
			    const char* account,
			    const char* domain,
			    int split_account)
{
  creds[0].type = CVM_CRED_ACCOUNT;
  str_copys(&creds[0].value, account);
  creds[1].type = CVM_CRED_DOMAIN;
  str_copys(&creds[1].value, domain);
  return cvm_authenticate(module, 2, creds, 0, split_account ? 1 : 0);
}

int cvm_authenticate_password(const char* module,
			      const char* account,
			      const char* domain,
			      const char* password,
			      int split_account)
{
  creds[0].type = CVM_CRED_ACCOUNT;
  str_copys(&creds[0].value, account);
  creds[1].type = CVM_CRED_DOMAIN;
  str_copys(&creds[1].value, domain);
  creds[2].type = CVM_CRED_PASSWORD;
  str_copys(&creds[2].value, password);
  return cvm_authenticate(module, 3, creds, 0, split_account ? 1 : 0);
}
