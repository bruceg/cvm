#ifndef AUTHENTICATOR__FACTS__H__
#define AUTHENTICATOR__FACTS__H__

#define CVM_FACT_USERNAME 1
#define CVM_FACT_USERID 2
#define CVM_FACT_GROUPID 3
#define CVM_FACT_REALNAME 4
#define CVM_FACT_DIRECTORY 5
#define CVM_FACT_SHELL 6
#define CVM_FACT_GROUPNAME 7
#define CVM_FACT_SUPP_GROUPID 8
#define CVM_FACT_SYS_USERNAME 9
#define CVM_FACT_SYS_DIRECTORY 10
#define CVM_FACT_OFFICE_LOCATION 11
#define CVM_FACT_WORK_PHONE 12
#define CVM_FACT_HOME_PHONE 13
#define CVM_FACT_DOMAIN 14

extern const char* cvm_fact_username;
extern unsigned long cvm_fact_userid;
extern unsigned long cvm_fact_groupid;
extern const char* cvm_fact_realname;
extern const char* cvm_fact_directory;
extern const char* cvm_fact_shell;
extern const char* cvm_fact_groupname;
extern const char* cvm_fact_sys_username;
extern const char* cvm_fact_sys_directory;
extern const char* cvm_fact_domain;

#endif
