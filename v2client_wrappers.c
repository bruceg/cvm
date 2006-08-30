/* cvm/v2client_wrappers.c - CVM version 2 client library wrapper functions
 * Copyright (C)2006  Bruce Guenter <bruce@untroubled.org>
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
#include "v2client.h"
#include "credentials.h"

static unsigned add(struct cvm_credential* creds,
		    unsigned i, unsigned type, const char* value)
{
  if (value == 0)
    return i;
  if (value[0] == 0)
    return i;
  creds[i].type = type;
  if (!str_copys(&creds[i].value, value))
    return 0;
  return i + 1;
}

static int doit(struct cvm_credential creds[],
		const char* module,
		const char* account,
		const char* domain,
		const char* password,
		int split_account)
{
  unsigned i;
  creds[0].type = CVM_CRED_ACCOUNT;
  if (!str_copys(&creds[0].value, account))
    return CVME_IO;
  if ((i = add(creds, 1, CVM_CRED_DOMAIN, domain)) == 0)
    return CVME_IO;
  if (split_account) {
    cvm_client_split_account(&creds[0].value, &creds[1].value);
    if (i == 1)
      if ((i = add(creds, i, CVM_CRED_DOMAIN, creds[i].value.s)) == 0)
	return CVME_IO;
  }
  if ((i = add(creds, i, CVM_CRED_PASSWORD, password)) == 0)
    return CVME_IO;
  return cvm_client_authenticate(module, i, creds);
}

int cvm_client_authenticate_password(const char* module,
			      const char* account,
			      const char* domain,
			      const char* password,
			      int split_account)
{
  struct cvm_credential creds[3];
  unsigned i;
  int result;
  memset(creds, 0, sizeof creds);
  result = doit(creds, module, account, domain, password, split_account);
  for (i = 0; i < 3; ++i)
    str_free(&creds[i].value);
  return result;
}
