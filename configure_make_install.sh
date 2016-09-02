#!/bin/bash -e

NGINX_VER=1.11.3
DISTFILES=~/distfiles
PARALLEL=3

cd `dirname $0`

tar xzf $DISTFILES/nginx-$NGINX_VER.tar.gz
OPTS=" 
 --prefix=/usr --conf-path=/tmp/no.such.file/nginx.conf
 --user=nobody --group=nobody --add-module=`pwd`
 "
for x in pcre{,-jit} threads http_{realip_module,ssl_module,v2_module} ; do
 OPTS="$OPTS --with-$x"
done
for x in stream_upstream_{hash,least_conn,hash} \
         mail_{imap,pop3,smtp} ; do
 OPTS+=" --without-${x}_module"
done

cd nginx-$NGINX_VER
./configure $OPTS
make VERBOSE=1 -j$PARALLEL
strip objs/nginx -o ../nginx.exe
cd ..
rm -rf nginx-$NGINX_VER

DST=/tmp/memory.test.nginx

mkdir -p $DST/{www,var/log}
cp nginx.exe 0.{key,crt} loop.sh $DST/
for x in nginx.{conf,sh} ; do
 sed s:@root:$DST:g < $x > $DST/$x
done
chmod +x $DST/nginx.sh

echo "Playground is at $DST. Enjoy"
