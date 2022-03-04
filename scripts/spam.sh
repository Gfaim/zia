#!/bin/bash
for ((i = 0 ; i <= $(($2)) ; i++)); do
    curl localhost/$1 &
done
