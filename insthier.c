#include <installer.h>
#include "conf_home.c"

void insthier(void) {
  int home = opendir(conf_home);
  int dir;

  dir = d(home, "bin",          -1, -1, 0755);
  c(dir, "cvm-benchclient",     -1, -1, 0755);
  c(dir, "cvm-checkpassword",   -1, -1, 0755);
  c(dir, "cvm-pwfile",          -1, -1, 0755);
  c(dir, "cvm-testclient",      -1, -1, 0755);
  c(dir, "cvm-unix",            -1, -1, 0755);

  dir = d(home, "include",      -1, -1, 0755);
  dir = d(dir,  "cvm",          -1, -1, 0755);
  c(dir, "client.h",            -1, -1, 0644);
  /* c(dir, "credentials.h",    -1, -1, 0644); Not yet included. */
  c(dir, "errors.h",            -1, -1, 0644);
  c(dir, "facts.h",             -1, -1, 0644);
  c(dir, "module.h",            -1, -1, 0644);

  dir = d(home, "lib",          -1, -1, 0755);
  cf(dir, "cvm-client.a",       -1, -1, 0644, "client.a");
  cf(dir, "cvm-command.a",      -1, -1, 0644, "command.a");
  cf(dir, "cvm-local.a",        -1, -1, 0644, "local.a");
  cf(dir, "cvm-module.a",       -1, -1, 0644, "module.a");
  cf(dir, "cvm-udp.a",          -1, -1, 0644, "udp.a");
}
