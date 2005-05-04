#ifndef CVM__QMAIL__H__
#define CVM__QMAIL__H__

#include <str/str.h>

struct qmail_user
{
  str user;
  unsigned long uid;
  unsigned long gid;
  str homedir;
  char dash;
  str ext;
};

extern const char* qmail_root;
extern const char* qmail_me;
extern const char* qmail_envnoathost;
int qmail_init(void);

int qmail_users_init(void);
int qmail_users_reinit(void);
int qmail_users_lookup(struct qmail_user* user, const char* name, char dash);
int qmail_users_lookup_split(struct qmail_user* u, const char* name,
			     str* local, str* ext);

int qmail_domains_init(void);
int qmail_domains_reinit(void);
int qmail_domains_lookup(const char* d, str* domain, str* prefix);

int qmail_dotfile_exists(const struct qmail_user* user, const char* ext);

int qmail_lookup_init(void);
int qmail_lookup_cvm(struct qmail_user* user,
		     str* domain,
		     str* username,
		     str* ext);

#endif
