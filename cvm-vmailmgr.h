#ifndef CVM_VMAILMGR__H__
#define CVM_VMAILMGR__H__

extern str default_user;
extern const char* secret;

/* Results from looking up the user */
extern const char* pw_dir;
extern uid_t pw_uid;
extern gid_t pw_gid;
extern const char* pw_name;

extern str domain;
extern str virtuser;
extern str vpwdata;

extern const char* pwfile;
extern const char* qmail_root;
extern const int try_default;
extern int lock_disabled;

#define DEBUG(A,B,C) debug(__FUNCTION__, __LINE__, A, B, C)
extern void debug(const char* func, int line,
		  const char* a, const char* b, const char* c);

extern int lookup_init(void);
extern int lookup_reinit(void);
extern int lookup_domain(void);
extern int lookup_baseuser(void);
extern int lookup_virtuser(void);

extern int vpwentry_auth(const vpwentry*);

#endif
