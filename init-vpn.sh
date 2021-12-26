#!/usr/bin/bash

set -x

hamachi login

if [[ $NET_MODE == "main" ]]; 
    then echo "create net"; 
    hamachi create $NET_NAME $NET_PASS
elif [[ $NET_MODE == "worker" ]];
    then echo "join net"; 
    hamachi join $NET_NAME $NET_PASS
fi