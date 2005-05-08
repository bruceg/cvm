/* cvm/client.c - CVM client library
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
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sysdeps.h>
#include <net/socket.h>

#include "v1client.h"
#include "protocol.h"

const char* cvm_account_split_chars = "@";

static unsigned char buffer[CVM_BUFSIZE];
static unsigned buflen;

/* Buffer management code ****************************************************/
static int parse_buffer(void)
{
  if (buffer[0] != 0) return buffer[0];
  if (buflen < 3) return CVME_BAD_MODDATA;
  if (buffer[buflen-1] != 0 || buffer[buflen-2] != 0) return CVME_BAD_MODDATA;
  if (cvm_fact_str(CVM_FACT_USERNAME, &cvm_fact_username) ||
      cvm_fact_uint(CVM_FACT_USERID, &cvm_fact_userid) ||
      cvm_fact_uint(CVM_FACT_GROUPID, &cvm_fact_groupid) ||
      cvm_fact_str(CVM_FACT_DIRECTORY, &cvm_fact_directory))
    return CVME_BAD_MODDATA;
  cvm_fact_str(CVM_FACT_SHELL, &cvm_fact_shell);
  cvm_fact_str(CVM_FACT_REALNAME, &cvm_fact_realname);
  cvm_fact_str(CVM_FACT_GROUPNAME, &cvm_fact_groupname);
  cvm_fact_str(CVM_FACT_SYS_USERNAME, &cvm_fact_sys_username);
  cvm_fact_str(CVM_FACT_SYS_DIRECTORY, &cvm_fact_sys_directory);
  cvm_fact_str(CVM_FACT_DOMAIN, &cvm_fact_domain);
  cvm_fact_str(CVM_FACT_MAILBOX, &cvm_fact_mailbox);
  return 0;
}

static unsigned char* buffer_add(unsigned char* ptr,
				 const char* str, unsigned len)
{
  if (ptr - buffer + len + 1 >= CVM_BUFSIZE-1) return 0;
  memcpy(ptr, str, len);
  ptr[len] = 0;
  return ptr + len + 1;
}

static unsigned build_buffer(const char* account, const char* domain,
			     const char** credentials, int parse_domain)
{
  unsigned char* ptr;
  unsigned i;
  unsigned actlen;
  
  buffer[0] = CVM1_PROTOCOL;
  ptr = buffer + 1;

  actlen = strlen(account);
  if (parse_domain) {
    const char* sc;
    if ((sc = getenv("CVM_ACCOUNT_SPLIT_CHARS")) == 0)
      sc = cvm_account_split_chars;
    i = strlen(account);
    while (i-- > 0) {
      if (strchr(sc, account[i]) != 0) {
	domain = account + i + 1;
	actlen = i;
	break;
      }
    }
  }
  
  if ((ptr = buffer_add(ptr, account, actlen)) == 0) return 0;
  if ((ptr = buffer_add(ptr, domain, strlen(domain))) == 0) return 0;

  for (i = 0; credentials[i] != 0; i++)
    if ((ptr = buffer_add(ptr, credentials[i], strlen(credentials[i]))) == 0)
      return 0;

  *ptr++ = 0;
  buflen = ptr - buffer;
  return 1;
}

int cvm_fact_str(unsigned number, const char** data)
{
  static unsigned char* ptr = 0;
  static unsigned last_number = -1;
  
  if (!ptr || number != last_number)
    ptr = buffer+1;
  last_number = number;
  
  while (*ptr) {
    unsigned char* tmp = ptr;
    ptr += strlen(ptr) + 1;
    if (*tmp == number) {
      *data = tmp + 1;
      return 0;
    }
  }
  return CVME_NOFACT;
}

int cvm_fact_uint(unsigned number, unsigned long* data)
{
  const char* str;
  unsigned long i;
  int err;
  
  if ((err = cvm_fact_str(number, &str)) != 0) return err;

  for (i = 0; *str >= '0' && *str <= '9'; ++str) {
    unsigned long tmp = i;
    i *= 10;
    if (i < tmp) return CVME_BAD_MODDATA;
    i += *str - '0';
  }
  if (*str) return CVME_BAD_MODDATA;
  *data = i;
  return 0;
}

/* Top-level wrapper *********************************************************/
int cvm_authenticate(const char* module, const char* account,
		     const char* domain, const char** credentials,
		     int parse_domain)
{
  int result;
  void (*oldsig)(int);
  if (domain == 0) domain = "";
  if (!build_buffer(account, domain, credentials, parse_domain))
    return CVME_GENERAL;
  
  oldsig = signal(SIGPIPE, SIG_IGN);
  if (!memcmp(module, "cvm-udp:", 8))
    result = cvm_xfer_udp(module+8, buffer, &buflen);
  else if (!memcmp(module, "cvm-local:", 10))
    result = cvm_xfer_local(module+10, buffer, &buflen);
  else {
    if (!memcmp(module, "cvm-command:", 12)) module += 12;
    result = cvm_xfer_command(module, buffer, &buflen);
  }
  signal(SIGPIPE, oldsig);
  if (result != 0) return result;
  return parse_buffer();
}
