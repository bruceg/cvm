#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <installer.h>
#include "conf_home.c"

void insthier(void) {
  struct stat st;
  const int do_mysql = stat("cvm-mysql", &st) != -1;
  const int do_pgsql = stat("cvm-pgsql", &st) != -1;
  int home = opendir(conf_home);
  int dir;

  dir = d(home, "bin",          -1, -1, 0755);
  c(dir, "cvm-benchclient",     -1, -1, 0755);
  c(dir, "cvm-checkpassword",   -1, -1, 0755);
  if (do_mysql) {
    c(dir, "cvm-mysql",           -1, -1, 0755);
    c(dir, "cvm-mysql-local",     -1, -1, 0755);
    c(dir, "cvm-mysql-udp",       -1, -1, 0755);
  }
  if (do_pgsql) {
    c(dir, "cvm-pgsql",           -1, -1, 0755);
    c(dir, "cvm-pgsql-local",     -1, -1, 0755);
    c(dir, "cvm-pgsql-udp",       -1, -1, 0755);
  }
  c(dir, "cvm-pwfile",          -1, -1, 0755);
  c(dir, "cvm-testclient",      -1, -1, 0755);
  c(dir, "cvm-vmailmgr",        -1, -1, 0755);
  c(dir, "cvm-vmailmgr-local",  -1, -1, 0755);
  c(dir, "cvm-vmailmgr-udp",    -1, -1, 0755);
  c(dir, "cvm-vmlookup",        -1, -1, 0755);
  c(dir, "cvm-unix",            -1, -1, 0755);

  dir = d(home, "include",      -1, -1, 0755);
  s(dir, "cvm-sasl.h",          "cvm/sasl.h");
  dir = d(dir,  "cvm",          -1, -1, 0755);
  c(dir, "client.h",            -1, -1, 0644);
  /* c(dir, "credentials.h",    -1, -1, 0644); Not yet included. */
  c(dir, "errors.h",            -1, -1, 0644);
  c(dir, "facts.h",             -1, -1, 0644);
  c(dir, "module.h",            -1, -1, 0644);
  c(dir, "sasl.h",              -1, -1, 0644);

  dir = d(home, "lib",          -1, -1, 0755);
  cf(dir, "libcvm-client.a",    -1, -1, 0644, "client.a");
  cf(dir, "libcvm-command.a",   -1, -1, 0644, "command.a");
  cf(dir, "libcvm-local.a",     -1, -1, 0644, "local.a");
  cf(dir, "libcvm-module.a",    -1, -1, 0644, "module.a");
  cf(dir, "libcvm-sasl.a",      -1, -1, 0644, "sasl.a");
  cf(dir, "libcvm-udp.a",       -1, -1, 0644, "udp.a");
}
