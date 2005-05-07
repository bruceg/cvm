#ifndef CVM__CREDENTIALS__H__
#define CVM__CREDENTIALS__H__

#define CVM_CRED_ACCOUNT 1
#define CVM_CRED_DOMAIN 2
#define CVM_CRED_PASSWORD 3
#define CVM_CRED_SECRET 4
#define CVM_CRED_CHALLENGE 5
#define CVM_CRED_RESPONSE 6
#define CVM_CRED_RESPONSE_TYPE 7
#define CVM_CRED_MAX 7

#define CVM_CRED_REQUIRED(X) do{ if (cvm_credentials[CVM_CRED_##X].len==0) return CVME_NOCRED; }while(0)

#endif
