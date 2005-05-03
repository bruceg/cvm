/*

 cvm-vchkpw.c

 for authentication against vpopmail from http://www.inter7.com

 add to conf-cc:

    -I/usr/local/vpopmail/include

 add to conf-ld:

     -L/usr/local/vpopmail/lib -L/usr/local/lib/mysql -R/usr/local/lib/mysql
     (mysql if vpopmail is compiled with mysql)
     
 add to Makefile:
 
   cvm-vchkpw: load cvm-vchkpw.o module.a crypt.lib shadow.lib s.lib socket.lib
   	./load cvm-vchkpw getpwnam.o module.a `cat crypt.lib `cat shadow.lib` `cat s.lib` `cat socket.lib` -lbg-net -lvpopmail -lmysqlclient

   cvm-vchkpw.o: compile cvm-vchkpw.c module.h facts.h errors.h
	./compile cvm-vchkpw.c

 and add cvm-vchkpw to the programs rule:

   programs: cvm-unix cvm-vchkpw cvm-pwfile cvm-benchclient cvm-testclient cvm-checkpassword

*/

#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include "vpopmail.h"
#include "vauth.h"
#include "pwcmp/client.h"
#include "module.h"

const char program[] = "cvm-vchkpw";

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
  mailbox = malloc(dirlen + 10);
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

