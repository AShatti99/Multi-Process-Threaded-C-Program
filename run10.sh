#!/bin/bash

for i in {1..10}
do  
    echo "--------------------- TEST #$i -------------------------"
    make clear
    make
    make test
    echo "--------------------------------------------------------"
done
make clear
