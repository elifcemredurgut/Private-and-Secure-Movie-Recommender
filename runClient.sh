#!/bin/bash
if [ $1 -eq 10 ]
then
        sed -i 's/numberOfRequestsNeedToBeSent =.*/numberOfRequestsNeedToBeSent = 1/gi' client/client.py
fi

if [ $1 -eq 100 ]
then
        sed -i 's/numberOfRequestsNeedToBeSent =.*/numberOfRequestsNeedToBeSent = 10/gi' client/client.py
fi

if [ $1 -eq 1000 ]
then
        sed -i 's/numberOfRequestsNeedToBeSent =.*/numberOfRequestsNeedToBeSent = 100/gi' client/client.py
fi

cd client
chmod +x script.sh
chmod +x run.sh
./run.sh 
