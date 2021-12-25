#!/usr/bin/bash

NET_NAME=$1
NET_PASS=$2

service logmein-hamachi --full-restart
sleep 5

hamachi login
sleep 5

hamachi create $NET_NAME $NET_PASS
sleep 5

ssh-keygen -t rsa -b 4096 -N "" -f ~/.ssh/id_rsa
/usr/sbin/sshd -D
