#!/bin/bash
for ((i = 0 ; i <= $(($1)) ; i++)); do
    ( (echo -e "Hello world" && cat) | nc localhost 80 )
done
