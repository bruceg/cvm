#ifndef AUTHENTICATOR__FACTS__H__
#define AUTHENTICATOR__FACTS__H__

#define FACT_USERNAME 1
#define FACT_USERID 2
#define FACT_GROUPID 3
#define FACT_REALNAME 4
#define FACT_DIRECTORY 5
#define FACT_SHELL 6
#define FACT_GROUPNAME 7
#define FACT_SUPP_GROUPID 8
#define FACT_SYS_USERNAME 9
#define FACT_SYS_DIRECTORY 10
#define FACT_OFFICE_LOCATION 11
#define FACT_WORK_PHONE 12
#define FACT_HOME_PHONE 13

extern const char* fact_username;
extern unsigned long fact_userid;
extern unsigned long fact_groupid;
extern const char* fact_realname;
extern const char* fact_directory;
extern const char* fact_shell;
extern const char* fact_groupname;
extern const char* fact_sys_username;
extern const char* fact_sys_directory;

#endif
