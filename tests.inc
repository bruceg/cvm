CVM_PWFILE_PATH=$tmp/pwfile
export CVM_PWFILE_PATH
make_pwfile()
{
  (
    echo 'pwfuser:testpass:123:456:Gecos,xyz:/home/ftp:/bin/false'
    echo 'cryptuser:tpzv1IkcX9.fE:234:567:Crypt:/home/crypt:/bin/true'
  ) >$CVM_PWFILE_PATH
}
