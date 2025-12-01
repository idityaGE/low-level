#!/bin/bash

n=$1
c=$2

HOST=localhost
PORT=8080

if command -v ab &> /dev/null
then
    ab -n "$n" -c "$c" http://${HOST}:${PORT}/
else
    echo "ApacheBench (ab) is not installed. Please install it to run the attack script."
    exit 1
fi