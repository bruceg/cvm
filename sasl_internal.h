#ifndef CVM__SASL_INTERNAL__H__
#define CVM__SASL_INTERNAL__H__

extern int sasl_login_start(struct sasl_state*, const str*, str*);
extern int sasl_plain_start(struct sasl_state*, const str*, str*);
extern int sasl_cram_md5_start(struct sasl_state*, const str*, str*);
extern int sasl_authenticate_plain(struct sasl_state*,
				   const char* account, const char* password);
extern int sasl_authenticate_cram(struct sasl_state*,
				  const char* account, const char* cram_type,
				  const str* challenge, const str* response);

#endif
