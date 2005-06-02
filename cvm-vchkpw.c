/*

 cvm-vchkpw.c

 for authentication against vpopmail from http://www.inter7.com

 add to conf-cc:

    -I/usr/local/vpopmail/include

 add to Makefile (mysql if vpopmail is compiled with mysql):
 
   cvm-vchkpw: load cvm-vchkpw.o module.a crypt.lib shadow.lib s.lib socket.lib
   	./load cvm-vchkpw getpwnam.o module.a `cat crypt.lib` `cat shadow.lib` `cat s.lib` `cat socket.lib` -L/usr/local/vpopmail/lib -L/usr/local/lib/mysql -lbg -lvpopmail -lmysqlclient

   cvm-vchkpw.o: compile cvm-vchkpw.c module.h facts.h errors.h
	./compile cvm-vchkpw.c

 and add cvm-vchkpw to the programs rule

*/

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <vpopmail.h>
#include <vauth.h>
#include "module.h"

const char program[] = "cvm-vchkpw";

int cvm_module_init(void)
{
  return 0;
}

void cvm_module_stop(void)
{
  return;
}

static struct vqpasswd* mypw;

int cvm_lookup(void)
{
  CVM_CRED_REQUIRED(DOMAIN);
  if ((mypw = vauth_getpw(cvm_credentials[CVM_CRED_ACCOUNT].s,
			  cvm_credentials[CVM_CRED_DOMAIN].s)) == 0)
    return CVME_PERMFAIL;
  return 0;
}

int cvm_authenticate(void)
{
  CVM_CRED_REQUIRED(PASSWORD);
  if (vauth_crypt(cvm_credentials[CVM_CRED_ACCOUNT].s,
		  cvm_credentials[CVM_CRED_DOMAIN].s,
		  cvm_credentials[CVM_CRED_PASSWORD].s,
		  mypw) != 0)
    return CVME_PERMFAIL;
  return 0;
}

int cvm_results(void)
{
  char* mailbox;
  char* tmpstr;
  uid_t uid;
  gid_t gid;
  const long dirlen = strlen(mypw->pw_dir);
  tmpstr = vget_assign(cvm_credentials[CVM_CRED_DOMAIN].s, 0, 0, &uid, &gid);
  mailbox = malloc(dirlen + 10);
  memcpy(mailbox, mypw->pw_dir, dirlen);
  memcpy(mailbox+dirlen, "/Maildir/", 10);
  cvm_fact_username = mypw->pw_name;
  cvm_fact_userid = uid;
  cvm_fact_groupid = gid;
  cvm_fact_realname = mypw->pw_gecos;
  cvm_fact_directory = mypw->pw_dir;
  cvm_fact_mailbox = mailbox;
  cvm_fact_shell = mypw->pw_shell;
  cvm_fact_domain = cvm_credentials[CVM_CRED_DOMAIN].s;
  cvm_fact_groupname = 0;
  return 0;		
}

