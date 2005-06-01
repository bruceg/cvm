#ifndef CVM__SASL__H__
#define CVM__SASL__H__

#include <str/str.h>

struct sasl_state;

typedef int (*saslfn)(struct sasl_state*, const str* response, str* challenge);

struct sasl_mechanism 
{
  const char* name;
  const char* var;
  const char* cvm;
  saslfn start;
  struct sasl_mechanism* next;
};

struct sasl_state
{
  saslfn response;
  str init;
  str username;
  const char* domain;
  const struct sasl_mechanism* mech;
};

extern const struct sasl_mechanism* sasl_mechanisms;
extern int sasl_init(struct sasl_state*);
extern int sasl_start(struct sasl_state*,
		      const char* mechanism, const str* initresponse,
		      str* challenge);

#define SASL_AUTH_OK 0		/* Authentication is complete */
#define SASL_AUTH_FAILED 1	/* Authentication failed (permanently) */
#define SASL_NO_MECH 2		/* Invalid or unknown mechanism name */
#define SASL_TEMP_FAIL 3	/* Temporary or internal failure */
#define SASL_CHALLENGE 4	/* Send challenge to client, wait for response */
#define SASL_RESP_REQUIRED 5	/* A response was required but not given */
#define SASL_RESP_NOTALLOWED 6	/* A (initial) response was given but not allowed */
#define SASL_RESP_BAD 7		/* The response was invalid */
#define SASL_RESP_EOF 8		/* EOF while waiting for response */

struct ibuf;
struct obuf;

struct sasl_auth
{
  struct sasl_state state;
  const char* prefix;
  const char* suffix;
  struct ibuf* in;
  struct obuf* out;
};

int sasl_auth_caps(str* caps);
int sasl_auth_init(struct sasl_auth* sa);
int sasl_auth2(struct sasl_auth* sa,
	       const char* mechanism, const char* iresponse);
int sasl_auth1(struct sasl_auth* sa, const str* arg);
const char* sasl_auth_msg(int* code);

#endif
