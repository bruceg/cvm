#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include "vpopmail.h"
#include "vauth.h"
#include "pwcmp/client.h"
#include "module.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

int cvm_auth_init(void)
{
  return 0;
}

void cvm_auth_stop(void)
{
  return;
}

static struct passwd* mypw;

int cvm_lookup(void)
{
  if ((mypw = vauth_user(cvm_account_name, cvm_account_domain,
			 cvm_credentials[0], NULL)) == 0)
    return CVME_PERMFAIL;
  return 0;
}

int cvm_authenticate(void)
{
  return 0;
}

int cvm_results(void)
{
  char* mailbox;
  const long dirlen = strlen(mypw->pw_dir);
  mailbox = malloc(strlen(dirlen + 10));
  memcpy(mailbox, mypw->pw_dir, dirlen);
  memcpy(mailbox+dirlen, "/Maildir/", 10);
  cvm_fact_username = mypw->pw_name;
  cvm_fact_userid = mypw->pw_uid;
  cvm_fact_groupid = mypw->pw_gid;
  cvm_fact_realname = mypw->pw_gecos;
  cvm_fact_directory = mypw->pw_dir;
  cvm_fact_mailbox = mailbox;
  cvm_fact_shell = mypw->pw_shell;
  cvm_fact_domain = cvm_account_domain;
  cvm_fact_groupname = 0;
  return 0;		
}

