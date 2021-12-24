#!/bin/bash

# touch /hosts/$(hostname)
ssh-keygen -t rsa -b 4096 -N "" -f ~/.ssh/id_rsa
service logmein-hamachi --full-restart
/usr/sbin/sshd -D
