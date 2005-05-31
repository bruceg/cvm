#ifndef CVM__SASL_INTERNAL__H__
#define CVM__SASL_INTERNAL__H__

extern const char* sasl_mech_cvm;

extern int sasl_login_start(const str*, str*);
extern int sasl_login_response(const str*, str*);

extern int sasl_plain_start(const str*, str*);
extern int sasl_plain_response(const str*, str*);

extern int sasl_cram_md5_start(const str*, str*);
extern int sasl_cram_md5_response(const str*, str*);

extern int sasl_authenticate_plain(const char* account, const char* password);
extern int sasl_authenticate_cram(const char* account, const char* cram_type,
				  const str* challenge, const str* response);

#endif
