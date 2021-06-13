#!/bin/bash

for N in {1..50}
do
    ./client localhost 3000 home.html&
done
wait