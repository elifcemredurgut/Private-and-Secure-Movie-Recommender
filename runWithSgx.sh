#!/bin/bash
if [ $1 -eq 10 ]
then
        sed -i 's/numberOfClientsNeedToBeDone =.*/numberOfClientsNeedToBeDone = 10;/gi' server-with-sgx/Enclave/Enclave.cpp
        cd server-with-sgx
        make
        (time ./app) 2> time_10_withSgx.txt
fi

if [ $1 -eq 100 ]
then
        sed -i 's/numberOfClientsNeedToBeDone =.*/numberOfClientsNeedToBeDone = 100;/gi' server-with-sgx/Enclave/Enclave.cpp
        cd server-with-sgx
        make
        (time ./app) 2> time_100_withSgx.txt
fi

if [ $1 -eq 1000 ]
then
        sed -i 's/numberOfClientsNeedToBeDone =.*/numberOfClientsNeedToBeDone = 1000;/gi' server-with-sgx/Enclave/Enclave.cpp
        cd server-with-sgx
        make
        (time ./app) 2> time_1000_withSgx.txt
fi

