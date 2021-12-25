#!/usr/bin/bash

while [[ "$#" -gt 0 ]]; do
        case $1 in
                -H|--host) host="$2"; shift ;;
                -HN|--host-name) host_name="$2"; shift ;;
                -P|--port) port="$2"; shift ;;
                -U|--user) user="$2"; shift ;;
                *) echo "Unknown parameter passed: $1"; exit 1 ;;
        esac
        shift
done

msg="Host $host\nHostName $host_name\nPort $port\nUser $user\n"

echo -e $msg >> $HOME/.ssh/config