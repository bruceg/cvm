printf='env printf'

home=$tmp/home
mkdir $home

CVM_PWFILE_PATH=$tmp/pwfile
export CVM_PWFILE_PATH
make_pwfile()
{
  (
    echo 'pwfuser:testpass:123:456:Gecos,xyz:/home/ftp:/bin/false'
    echo 'cryptuser:tpzv1IkcX9.fE:234:567:Crypt:/home/crypt:/bin/true'
  ) >$CVM_PWFILE_PATH
}

CVM_SQLITE_DB=$tmp/db.sqlite
CVM_SQLITE_QUERY='SELECT password, username, userid, groupid, directory, realname, shell, groupname, domain, sys_username, sys_directory, mailbox_path FROM accounts WHERE username=$account AND domain=$domain'
CVM_SQLITE_PWCMP=plain
export CVM_SQLITE_DB CVM_SQLITE_QUERY CVM_SQLITE_PWCMP
make_sqlite()
{
  sqlite3 $CVM_SQLITE_DB <<EOF
CREATE TABLE accounts (
  username      TEXT NOT NULL,
  domain        TEXT NOT NULL,
  password      TEXT NOT NULL,
  userid        TEXT NOT NULL,
  groupid       TEXT NOT NULL,
  realname      TEXT,
  directory     TEXT NOT NULL,
  shell         TEXT,
  groupname     TEXT,
  sys_username  TEXT,
  sys_directory TEXT,
  mailbox_path  TEXT,
  UNIQUE (username, domain)
);
INSERT INTO accounts VALUES ('sluser','','testpass',123,456,'realname','/home/sluser','/bin/false',NULL,NULL,NULL,NULL);
INSERT INTO accounts VALUES ('sluser','domain','pass2',234,567,'sluser2','/home/sluser2','/bin/true',NULL,NULL,NULL,NULL);
EOF
}

for base in pwfile qmail vmailmgr; do

cat <<EOF >$tmp/cvm-$base-lookup
#!/bin/sh
CVM_LOOKUP_SECRET=secret export CVM_LOOKUP_SECRET
exec $src/cvm-$base
EOF

cat <<EOF >$tmp/cvm-$base-nosecret
#!/bin/bash
unset CVM_LOOKUP_SECRET
exec $src/cvm-$base
EOF

done

chmod +x $tmp/cvm-*

QMAIL_ROOT=$tmp
export QMAIL_ROOT

mkdir $tmp/users
(
  uid=`id -u`
  gid=`id -g`
  hlen=`echo $home $uid $gid | wc -c`
  tlen=`echo $tmp $uid $gid | wc -c`
  $printf "+6,%d:!user\0->user\000$uid\000$gid\000$home\0\0\n" $(($hlen+6))
  $printf "+6,%d:!user-->user\000$uid\000$gid\000$home\0-\0\n" $(($hlen+7))
  $printf "+7,%d:!alias\0->alias\000$uid\000$gid\000$tmp/alias\0\0\n" $(($tlen+13))
  $printf "+7,%d:!alias-->alias\000$uid\000$gid\000$tmp/alias\0-\0\n" $(($tlen+14))
  $printf "+1,%d:!->alias\000$uid\000$gid\000$tmp/alias\0-\0\n" $(($tlen+14))
  echo
) | cdbmake $tmp/users/cdb $tmp/users/tmp

mkdir $tmp/control
echo local.dom >$tmp/control/locals
echo test.tld:user >$tmp/control/virtualdomains
echo noathost:user >>$tmp/control/virtualdomains
echo noathost >$tmp/control/envnoathost
mkdir $tmp/alias
echo \# >$home/.qmail-wild-default
echo \# >$home/.qmail-addr
echo \# >$home/.qmail-dot:addr
echo \# >$tmp/alias/.qmail-alias
echo \# >$tmp/alias/.qmail-awild-default
echo \# >$tmp/alias/.qmail-adot:addr

(
  $printf '+4,69:virt->\x2\x8\x1\0$1$aSoIrl/J$TmAwoxKzrPJ0IaW5UvX4A0\0./virt\0\0\0-\0-\0-\0-\0001016731358\0-\0\n'
  echo
) | cdbmake $home/passwd.cdb $home/passwd.tmp

sasltest() {
  make_pwfile
  env \
  CVM_SASL_PLAIN=$src/cvm-pwfile \
  PROTO=TCP \
  TCPLOCALHOST=localhost \
  $src/sasl-auth-test "$@"
  rm -f $CVM_PWFILE_PATH
}

