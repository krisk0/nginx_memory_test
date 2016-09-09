#!/bin/bash -e

[ -s nginx.pid ] || ./nginx.sh

NCAT='ncat --ssl 127.0.0.1 1111'

PID=`ps ax|grep 'nginx: worker'|grep -v grep`
PID=`echo $PID|cut -d\  -f1`
echo "worker PID=$PID"

co=0
while : ; do
  printf 'GET /hello\r\n' | $NCAT
  printf 'GET /time\r\n' | $NCAT
  ti=$(echo 0 $(awk '/Pss/ {print "+", $2}' /proc/$PID/smaps) | bc)
  printf "%04d %s  " $co $ti
  co=$((co+1))
done
