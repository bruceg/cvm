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

#include "v2client.h"
#include "credentials.h"
#include "protocol.h"

const char* cvm_account_split_chars = "@";

static unsigned char buffer[CVM_BUFSIZE];
static unsigned buflen;
static struct 
{
  unsigned type;
  unsigned start;
} offsets[CVM_BUFSIZE/2];
static str randombytes;

/* Buffer management code ****************************************************/
static int parse_buffer(void)
{
  unsigned i;
  unsigned o;
  if (buflen < 3)
    return CVME_BAD_MODDATA;
  if (buffer[1] != randombytes.len)
    return CVME_BAD_MODDATA;
  if (memcmp(buffer+2, randombytes.s, randombytes.len) != 0)
    return CVME_BAD_MODDATA;
  if (buffer[0] != 0)
    return buffer[0];
  if (buffer[buflen-1] != 0)
    return CVME_BAD_MODDATA;
  /* This funny loop gives all the strings in the buffer NUL termination. */
  for (i = 0, o = buffer[1] + 2;
       o < sizeof buffer && buffer[o] != 0;
       ++i, o += buffer[o+1] + 2) {
    offsets[i].type = buffer[o];
    offsets[i].start = o+2;
    buffer[o] = 0;
  }
  offsets[i].type = offsets[i].start = 0;
  /* Extract required and common facts. */
  if (cvm_fact_str(CVM_FACT_USERNAME, &cvm_fact_username, &i) ||
      cvm_fact_uint(CVM_FACT_USERID, &cvm_fact_userid) ||
      cvm_fact_uint(CVM_FACT_GROUPID, &cvm_fact_groupid) ||
      cvm_fact_str(CVM_FACT_DIRECTORY, &cvm_fact_directory, &i))
    return CVME_BAD_MODDATA;
  cvm_fact_str(CVM_FACT_SHELL, &cvm_fact_shell, &i);
  cvm_fact_str(CVM_FACT_REALNAME, &cvm_fact_realname, &i);
  cvm_fact_str(CVM_FACT_GROUPNAME, &cvm_fact_groupname, &i);
  cvm_fact_str(CVM_FACT_SYS_USERNAME, &cvm_fact_sys_username, &i);
  cvm_fact_str(CVM_FACT_SYS_DIRECTORY, &cvm_fact_sys_directory, &i);
  cvm_fact_str(CVM_FACT_DOMAIN, &cvm_fact_domain, &i);
  cvm_fact_str(CVM_FACT_MAILBOX, &cvm_fact_mailbox, &i);
  return 0;
}

static unsigned char* buffer_add(unsigned char* ptr, unsigned type,
				 unsigned len, const char* data)
{
  if (ptr - buffer + len + 2 >= CVM_BUFSIZE-1) return 0;
  *ptr++ = type;
  *ptr++ = len;
  memcpy(ptr, data, len);
  return ptr + len;
}

static int parse_domain(struct cvm_credential* credentials,
			unsigned account, unsigned domain)
{
  unsigned actlen;
  char* actptr;
  unsigned i;
  if (account > 0 || domain > 0) {
    const char* sc;
    actlen = credentials[account].value.len;
    actptr = credentials[account].value.s;
    if ((sc = getenv("CVM_ACCOUNT_SPLIT_CHARS")) == 0)
      sc = cvm_account_split_chars;
    i = actlen;
    while (i-- > 0) {
      if (strchr(sc, actptr[i]) != 0) {
	if (!str_copyb(&credentials[domain].value,
		       actptr + i + 1,
		       actlen - i - 1))
	  return 0;
	credentials[domain].value.len = i;
	break;
      }
    }
  }
  return 1;
}

static unsigned build_buffer(unsigned count,
			     struct cvm_credential* credentials)
{
  const char* env;
  unsigned char* ptr;
  unsigned i;
  int has_secret;

  /* FIXME: generate real random data here */
  ptr = buffer_add(buffer, CVM2_PROTOCOL, randombytes.len, randombytes.s);

  for (i = 0, has_secret = 0; i < count; ++i, ++credentials) {
    if (credentials->type == CVM_CRED_SECRET)
      has_secret = 1;
    if ((ptr = buffer_add(ptr, credentials->type,
			  credentials->value.len,
			  credentials->value.s)) == 0)
      return 0;
  }

  if (!has_secret
      && (env = getenv("CVM_LOOKUP_SECRET")) != 0)
    if ((ptr = buffer_add(ptr, CVM_CRED_SECRET, strlen(env), env)) == 0)
      return 0;

  *ptr++ = 0;
  buflen = ptr - buffer;
  return 1;
}

int cvm_fact_str(unsigned number, const char** data, unsigned* length)
{
  static unsigned last_offset = 0;
  static unsigned last_number = -1;
  unsigned o;
  int err = CVME_NOFACT;
  
  o = (number != last_number || offsets[last_offset].type == 0)
    ? 0
    : last_offset;
  last_number = number;
  
  while (offsets[o].type != 0) {
    if (offsets[o++].type == number) {
      *length = (*data = buffer + offsets[o-1].start)[-1];
      err = 0;
      break;
    }
  }
  last_offset = o;
  return err;
}

int cvm_fact_uint(unsigned number, unsigned long* data)
{
  const char* ptr;
  unsigned len;
  unsigned long i;
  int err;
  
  if ((err = cvm_fact_str(number, &ptr, &len)) != 0) return err;

  for (i = 0; len > 0 && *ptr >= '0' && *ptr <= '9'; ++ptr, --len) {
    unsigned long tmp = i;
    i = (i * 10) + (*ptr - '0');
    if (i < tmp)
      return CVME_BAD_MODDATA;
  }
  if (len > 0)
    return CVME_BAD_MODDATA;
  *data = i;
  return 0;
}

/* Top-level wrapper *********************************************************/
int cvm_authenticate(const char* module, unsigned count,
		     struct cvm_credential* credentials,
		     unsigned account, unsigned domain)
{
  int result;
  void (*oldsig)(int);
  parse_domain(credentials, account, domain);
  if (!build_buffer(count, credentials))
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
