#include <errno.h>
#include <pwd.h>
#include <string.h>
#include "hasspnam.h"
#include "hasuserpw.h"
#include "module.h"

#ifdef HASGETSPNAM
#include <shadow.h>
#endif

#ifdef HASUSERPW
#include <userpw.h>
#endif

extern char* crypt(const char* key, const char* salt);

void authenticate(void)
{
  struct passwd* pw;
#ifdef HASGETSPNAM
  struct spwd* spw;
#endif
#ifdef HASUSERPW
  struct userpw* upw;
#endif
  const char* password;
  const char* cpw;
  
  password = cred_string();
  pw = getpwnam(account_name);
  if (pw)
    cpw = pw->pw_passwd;
  else
    exit((errno == ETXTBSY) ? 111 : 100);

#ifdef HASUSERPW
  upw = getuserpw(account_name);
  if (upw)
    cpw = upw->upw_passwd;
  else
    if (errno == ETXTBSY) exit(111);
#endif

#ifdef HASGETSPNAM
  spw = getspnam(account_name);
  if (spw)
    cpw = spw->sp_pwdp;
  else
    if (errno == ETXTBSY) exit(111);
#endif

  if (!cpw) exit(100);
  if (strcmp(crypt(password, cpw), cpw)) exit(100);

  fact_username = pw->pw_name;
  fact_userid = pw->pw_uid;
  fact_groupid = pw->pw_gid;
  fact_realname = pw->pw_gecos;
  fact_directory = pw->pw_dir;
  fact_shell = pw->pw_shell;
}
