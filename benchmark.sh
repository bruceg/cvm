#!/bin/sh
reps=10000
user=cvmtest
pass=cvmpass
base=cvm-unix

echo Running cvm-command benchmark...
time ./cvm-benchclient $reps cvm-command:./$base $user $pass

echo
echo Running cvm-local benchmark...
time ./$base-local ./socket >/dev/null &
time ./cvm-benchclient $reps cvm-local:./socket $user $pass
killall $base-local && wait

echo
echo Running cvm-udp benchmark...
time ./$base-udp 127.2.3.4 2345 >/dev/null &
time ./cvm-benchclient $reps cvm-udp:127.2.3.4:2345 $user $pass
killall $base-udp && wait
