#include "installer.h"
#include "conf_bin.c"

void insthier(void) {
  int bin = opendir(conf_bin);
  c(bin, "cvm-benchclient",     -1, -1, 0755);
  c(bin, "cvm-checkpassword",   -1, -1, 0755);
  c(bin, "cvm-pwfile",          -1, -1, 0755);
  c(bin, "cvm-pwfile-local",    -1, -1, 0755);
  c(bin, "cvm-pwfile-udp",      -1, -1, 0755);
  c(bin, "cvm-testclient",      -1, -1, 0755);
  c(bin, "cvm-unix",            -1, -1, 0755);
  c(bin, "cvm-unix-local",      -1, -1, 0755);
  c(bin, "cvm-unix-udp",        -1, -1, 0755);
}
