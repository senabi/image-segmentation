#!/usr/bin/bash

service logmein-hamachi --full-restart
ssh-keygen -t rsa -b 4096 -N "" -f ~/.ssh/id_rsa
/usr/sbin/sshd -D