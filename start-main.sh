#!/usr/bin/bash

NET_NAME=$1
NET_PASS=$2

service logmein-hamachi --full-restart
hamachi login
hamachi create $NET_NAME $NET_PASS

ssh-keygen -t rsa -b 4096 -N "" -f ~/.ssh/id_rsa
/usr/sbin/sshd -D
