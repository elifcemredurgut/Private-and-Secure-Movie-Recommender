#!/bin/bash
if [ $1 -eq 10 ]
then
        sed -i 's/numberOfClientsNeedToBeDone =.*/numberOfClientsNeedToBeDone = 10;/gi' server-without-sgx/server.cpp
	cd server-without-sgx
	chmod +x run.sh
	(time ./run.sh) 2> time_10_withoutSgx.txt
fi

if [ $1 -eq 100 ]
then
        sed -i 's/numberOfClientsNeedToBeDone =.*/numberOfClientsNeedToBeDone = 100;/gi' server-without-sgx/server.cpp
	cd server-without-sgx
	chmod +x run.sh
	(time ./run.sh) 2> time_100_withoutSgx.txt
fi    

if [ $1 -eq 1000 ]
then
        sed -i 's/numberOfClientsNeedToBeDone =.*/numberOfClientsNeedToBeDone = 1000;/gi' server-without-sgx/server.cpp
	cd server-without-sgx
	chmod +x run.sh
	(time ./run.sh) 2> time_1000_withoutSgx.txt
fi
