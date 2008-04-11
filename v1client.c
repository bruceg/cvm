/* cvm/client.c - CVM client library
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
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sysdeps.h>
#include <net/socket.h>

#include "v1client.h"
#include "protocol.h"

const char* cvm_client_account_split_chars = "@";

static struct cvm_packet packet;

/* Packet management code ****************************************************/
static int parse_packet(struct cvm_packet* p)
{
  if (p->data[0] != 0)
    return p->data[0];
  if (p->length < 3
      || p->data[p->length-1] != 0
      || p->data[p->length-2] != 0)
    return CVME_BAD_MODDATA;
  if (cvm_client_fact_str(CVM_FACT_USERNAME, &cvm_fact_username) ||
      cvm_client_fact_uint(CVM_FACT_USERID, &cvm_fact_userid) ||
      cvm_client_fact_uint(CVM_FACT_GROUPID, &cvm_fact_groupid) ||
      cvm_client_fact_str(CVM_FACT_DIRECTORY, &cvm_fact_directory))
    return CVME_BAD_MODDATA;
  cvm_client_fact_str(CVM_FACT_SHELL, &cvm_fact_shell);
  cvm_client_fact_str(CVM_FACT_REALNAME, &cvm_fact_realname);
  cvm_client_fact_str(CVM_FACT_GROUPNAME, &cvm_fact_groupname);
  cvm_client_fact_str(CVM_FACT_SYS_USERNAME, &cvm_fact_sys_username);
  cvm_client_fact_str(CVM_FACT_SYS_DIRECTORY, &cvm_fact_sys_directory);
  cvm_client_fact_str(CVM_FACT_DOMAIN, &cvm_fact_domain);
  cvm_client_fact_str(CVM_FACT_MAILBOX, &cvm_fact_mailbox);
  return 0;
}

static int packet_add(struct cvm_packet* p,
		      const char* str, unsigned len)
{
  unsigned char* ptr;
  if (p->length + len + 1 >= CVM_BUFSIZE-1)
    return 0;
  ptr = p->data + p->length;
  memcpy(ptr, str, len);
  ptr[len] = 0;
  p->length += len + 1;
  return 1;
}

static unsigned build_packet(struct cvm_packet* p,
			     const char* account, const char* domain,
			     const char** credentials, int parse_domain)
{
  unsigned i;
  unsigned actlen;
  
  p->data[0] = CVM1_PROTOCOL;
  p->length = 1;

  actlen = strlen(account);
  if (parse_domain) {
    const char* sc;
    if ((sc = getenv("CVM_ACCOUNT_SPLIT_CHARS")) == 0)
      sc = cvm_client_account_split_chars;
    i = strlen(account);
    while (i-- > 0) {
      if (strchr(sc, account[i]) != 0) {
	domain = account + i + 1;
	actlen = i;
	break;
      }
    }
  }
  
  if (!packet_add(p, account, actlen)) return 0;
  if (!packet_add(p, domain, strlen(domain))) return 0;

  for (i = 0; credentials[i] != 0; i++)
    if (!packet_add(p, credentials[i], strlen(credentials[i])))
      return 0;

  p->data[p->length++] = 0;
  return 1;
}

int cvm_client_fact_str(unsigned number, const char** data)
{
  static unsigned char* ptr = 0;
  static unsigned last_number = -1;
  
  if (!ptr || number != last_number)
    ptr = packet.data+1;
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

int cvm_client_fact_uint(unsigned number, unsigned long* data)
{
  const char* str;
  unsigned long i;
  int err;
  
  if ((err = cvm_client_fact_str(number, &str)) != 0) return err;

  for (i = 0; *str >= '0' && *str <= '9'; ++str) {
    unsigned long tmp = i;
    i = (i * 10) + (*str - '0');
    if (i < tmp)
      return CVME_BAD_MODDATA;
  }
  if (*str)
    return CVME_BAD_MODDATA;
  *data = i;
  return 0;
}

/* Top-level wrapper *********************************************************/
int cvm_client_authenticate(const char* module, const char* account,
			    const char* domain, const char** credentials,
			    int parse_domain)
{
  int result;
  void (*oldsig)(int);
  if (domain == 0) domain = "";
  if (!build_packet(&packet, account, domain, credentials, parse_domain))
    return CVME_GENERAL;
  
  oldsig = signal(SIGPIPE, SIG_IGN);
  if (!memcmp(module, "cvm-udp:", 8))
    result = cvm_xfer_udp(module+8, &packet);
  else if (!memcmp(module, "cvm-local:", 10))
    result = cvm_xfer_local(module+10, &packet);
  else {
    if (!memcmp(module, "cvm-command:", 12)) module += 12;
    result = cvm_xfer_command(module, &packet);
  }
  signal(SIGPIPE, oldsig);
  if (result != 0) return result;
  return parse_packet(&packet);
}
