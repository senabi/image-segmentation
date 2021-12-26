#!/usr/bin/bash

login_tries=0

until [ "$login_tries" -ge 2 ]
do
    hamachi login && break
    login_tries=$((login_tries+1))
    sleep 2
done

if [[ $NET_MODE == "main" ]]; 
    then echo "create net"; 
    create_tries=0
    go_online_tries=0

    sleep 2
    until [ "$create_tries" -ge 2 ]
    do
        hamachi create $NET_NAME $NET_PASS && break
        create_tries=$((create_tries+1))
        sleep 2
    done
    sleep 2
    until [ "$go_online_tries" -ge 2 ]
    do
        hamachi go-online $NET_NAME && break
        go_online_tries=$((go_online_tries+1))
        sleep 2
    done

elif [[ $NET_MODE == "worker" ]];
    then echo "join net"; 
    join_tries=0

    sleep 2
    until [ "$join_tries" -ge 2 ]
    do
        hamachi join $NET_NAME $NET_PASS && break
        join_tries=$((join_tries+1))
        sleep 2
    done
fi

echo "localhost" > $HOME/hostfile