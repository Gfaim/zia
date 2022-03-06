#!/bin/bash
for ((i = 0 ; i <= $(($1)) ; i++)); do
    curl https://localhost/ -kvi &
done
