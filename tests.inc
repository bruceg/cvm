CVM_PWFILE_PATH=$tmp/pwfile
export CVM_PWFILE_PATH
make_pwfile()
{
  (
    echo 'pwfuser:testpass:123:456:Gecos,xyz:/home/ftp:/bin/false'
    echo 'cryptuser:tpzv1IkcX9.fE:234:567:Crypt:/home/crypt:/bin/true'
  ) >$CVM_PWFILE_PATH
}
testc() { ./cvm-testclient ./cvm-vmailmgr "$@"; }
testl() { CVM_LOOKUP_SECRET=secret ./cvm-testclient ./cvm-vmailmgr "$@"; }
testq() { CVM_LOOKUP_SECRET=secret ./cvm-testclient ./cvm-qmail "$@"; }

QMAIL_ROOT=$tmp
export QMAIL_ROOT

mkdir $tmp/users
(
  uid=`id -u`
  gid=`id -g`
  len=`echo $tmp $uid $gid | wc -c`
  printf "+6,%d:!user\0->user\0000$uid\0000$gid\0000$tmp\0\0\n" $(($len+6))
  printf "+6,%d:!user-->user\0000$uid\0000$gid\0000$tmp\0-\0\n" $(($len+7))
  printf "+1,%d:!->alias\0000$uid\0000$gid\0000$tmp/alias\0-\0\n" $((len+14))
  echo
) | cdbmake $tmp/users/cdb $tmp/users/tmp

mkdir $tmp/control
echo local.dom >$tmp/control/locals
echo test.tld:user >$tmp/control/virtualdomains
echo noathost:user >>$tmp/control/virtualdomains
echo noathost >$tmp/control/envnoathost
mkdir $tmp/alias
echo \# >$tmp/.qmail-wild-default
echo \# >$tmp/.qmail-addr
echo \# >$tmp/.qmail-dot:addr
echo \# >$tmp/alias/.qmail-alias
echo \# >$tmp/alias/.qmail-awild-default
echo \# >$tmp/alias/.qmail-adot:addr

(
  printf '+4,69:virt->\x2\x8\x1\0$1$aSoIrl/J$TmAwoxKzrPJ0IaW5UvX4A0\0./virt\0\0\0-\0-\0-\0-\00001016731358\0-\0\n'
  echo
) | cdbmake $tmp/passwd.cdb $tmp/passwd.tmp
