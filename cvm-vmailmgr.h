#ifndef CVM_VMAILMGR__H__
#define CVM_VMAILMGR__H__

extern str default_user;

/* Results from looking up the user */
extern struct qmail_user vmuser;

extern str domain;
extern str virtuser;
extern str vpwdata;

extern const char* pwfile;

#define DEBUG(A,B,C) debug(__FUNCTION__, __LINE__, A, B, C)
extern void debug(const char* func, int line,
		  const char* a, const char* b, const char* c);

extern int lookup_init(void);
extern int lookup_virtuser(void);
extern int vmailmgr_autoconvert(void);

#endif
