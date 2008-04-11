/* cvm/client.c - CVM client library
 * Copyright (C)2008  Bruce Guenter <bruce@untroubled.org>
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
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sysdeps.h>
#include <net/socket.h>
#include <str/iter.h>
#include <str/str.h>

#include "v2client.h"
#include "credentials.h"
#include "protocol.h"
#include "random.h"

const char* cvm_client_account_split_chars = "@";

static struct cvm_packet request;
static struct cvm_packet response;

static struct 
{
  unsigned type;
  unsigned start;
} offsets[CVM_BUFSIZE/2];
static str randombytes;

/* Packet management code ****************************************************/
static int parse_packet(struct cvm_packet* p)
{
  unsigned i;
  unsigned o;
  if (p->length < 3)
    return CVME_BAD_MODDATA;
  if (p->data[1] != randombytes.len)
    return CVME_BAD_MODDATA;
  if (memcmp(p->data+2, randombytes.s, randombytes.len) != 0)
    return CVME_BAD_MODDATA;
  if (p->data[p->length-1] != 0)
    return CVME_BAD_MODDATA;
  /* This funny loop gives all the strings in the p->data NUL termination. */
  for (i = 0, o = p->data[1] + 2;
       o < sizeof p->data && p->data[o] != 0;
       ++i, o += p->data[o+1] + 2) {
    offsets[i].type = p->data[o];
    offsets[i].start = o+2;
    p->data[o] = 0;
  }
  offsets[i].type = offsets[i].start = 0;
  if (p->data[0] != 0)
    return p->data[0];
  /* Extract required and common facts. */
  if (cvm_client_fact_str(CVM_FACT_USERNAME, &cvm_fact_username, &i) ||
      cvm_client_fact_uint(CVM_FACT_USERID, &cvm_fact_userid) ||
      cvm_client_fact_uint(CVM_FACT_GROUPID, &cvm_fact_groupid) ||
      cvm_client_fact_str(CVM_FACT_DIRECTORY, &cvm_fact_directory, &i))
    return CVME_BAD_MODDATA;
  cvm_client_fact_str(CVM_FACT_SHELL, &cvm_fact_shell, &i);
  cvm_client_fact_str(CVM_FACT_REALNAME, &cvm_fact_realname, &i);
  cvm_client_fact_str(CVM_FACT_GROUPNAME, &cvm_fact_groupname, &i);
  cvm_client_fact_str(CVM_FACT_SYS_USERNAME, &cvm_fact_sys_username, &i);
  cvm_client_fact_str(CVM_FACT_SYS_DIRECTORY, &cvm_fact_sys_directory, &i);
  cvm_client_fact_str(CVM_FACT_DOMAIN, &cvm_fact_domain, &i);
  cvm_client_fact_str(CVM_FACT_MAILBOX, &cvm_fact_mailbox, &i);
  return 0;
}

static unsigned packet_add(struct cvm_packet* p, unsigned type,
			   unsigned len, const char* data)
{
  unsigned char* ptr;
  if (p->length + len + 2 >= CVM_BUFSIZE-1)
    return 0;
  ptr = p->data + p->length;
  *ptr++ = type;
  *ptr++ = len;
  memcpy(ptr, data, len);
  p->length += len + 2;
  return 1;
}

static void make_randombytes(void)
{
  static int initialized = 0;
  unsigned i;
  const char *e;

  if (!initialized) {
    cvm_random_init();

    if (randombytes.len == 0) {
      if ((e = getenv("CVM_RANDOM_BYTES")) != 0)
	i = atoi(e);
      else
	i = 8;
      str_ready(&randombytes, i);
      randombytes.len = i;
    }
    initialized = 1;
  }

  cvm_random_fill((unsigned char*)randombytes.s, randombytes.len);
}

static unsigned build_packet(struct cvm_packet* p,
			     unsigned count,
			     const struct cvm_credential* credentials,
			     int addrandom)
{
  const char* env;
  unsigned i;
  int has_secret;

  if (addrandom)
    make_randombytes();
  else
    randombytes.len = 0;
  p->length = 0;
  if (!packet_add(p, CVM2_PROTOCOL, randombytes.len, randombytes.s))
    return 0;

  for (i = 0, has_secret = 0; i < count; ++i, ++credentials) {
    if (credentials->type == CVM_CRED_SECRET)
      has_secret = 1;
    if (!packet_add(p, credentials->type,
		    credentials->value.len, credentials->value.s))
      return 0;
  }

  if (!has_secret
      && (env = getenv("CVM_LOOKUP_SECRET")) != 0)
    if (!packet_add(p, CVM_CRED_SECRET, strlen(env), env))
      return 0;

  p->data[p->length++] = 0;
  return 1;
}

int cvm_client_fact_str(unsigned number, const char** data, unsigned* length)
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
      *length = (*data = response.data + offsets[o-1].start)[-1];
      err = 0;
      break;
    }
  }
  last_offset = o;
  return err;
}

int cvm_client_fact_uint(unsigned number, unsigned long* data)
{
  const char* ptr;
  unsigned len;
  unsigned long i;
  int err;
  
  if ((err = cvm_client_fact_str(number, &ptr, &len)) != 0) return err;

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

int cvm_client_split_account(str* account, str* domain)
{
  unsigned actlen;
  char* actptr;
  unsigned i;
  const char* sc;
  actlen = account->len;
  actptr = account->s;
  if ((sc = getenv("CVM_ACCOUNT_SPLIT_CHARS")) == 0)
    sc = cvm_client_account_split_chars;
  i = actlen;
  while (i-- > 0) {
    if (strchr(sc, actptr[i]) != 0) {
      if (!str_copyb(domain, actptr + i + 1, actlen - i - 1))
	return 0;
      account->s[account->len = i] = 0;
      break;
    }
  }
  return 1;
}

/* Top-level wrapper *********************************************************/
int cvm_client_authenticate(const char* modules, unsigned count,
			    const struct cvm_credential* credentials)
{
  int result;
  void (*oldsig)(int);
  int addrandom;
  static str module_list;
  striter i;
  unsigned long u;

  /* Make a copy of the module list so we can make the strings NUL
   * terminated internally. */
  if (!str_copys(&module_list, modules))
    return CVME_IO | CVME_FATAL;
  str_subst(&module_list, ',', '\0');

  /* Set addrandom to true if any module uses UDP. */
  addrandom = 0;
  striter_loop(&i, &module_list, '\0') {
    if (memcmp(i.startptr, "cvm-udp:", 8) == 0) {
      addrandom = 1;
      break;
    }
  }

  if (!build_packet(&request, count, credentials, addrandom))
    return CVME_GENERAL;
  
  oldsig = signal(SIGPIPE, SIG_IGN);

  /* Invoke each module in the list, exiting when any module produces a
   * non-PERMFAIL result, or when it produces a PERMFAIL result with
   * OUTOFSCOPE set to 0. */
  striter_loop(&i, &module_list, '\0') {
    if (!memcmp(i.startptr, "cvm-udp:", 8))
      result = cvm_xfer_udp_packets(i.startptr+8, &request, &response);
    else if (!memcmp(i.startptr, "cvm-local:", 10))
      result = cvm_xfer_local_packets(i.startptr+10, &request, &response);
    else {
      if (!memcmp(i.startptr, "cvm-command:", 12))
	i.startptr += 12;
      result = cvm_xfer_command_packets(i.startptr, &request, &response);
    }
    /* Note: the result returned by cvm_xfer_* indicates if transmission
     * succeeded, not the actual result of validation.  The validation
     * result is returned by parse_packet. */
    if (result == 0)
      result = parse_packet(&response);
    /* Return success and temporary failures. */
    if (result != CVME_PERMFAIL)
      break;
    /* Also return permanent failure if the result is in scope. */
    if (cvm_client_fact_uint(CVM_FACT_OUTOFSCOPE, &u) == 0
	&& u == 0)
      break;
  }
  signal(SIGPIPE, oldsig);
  return result;
}
