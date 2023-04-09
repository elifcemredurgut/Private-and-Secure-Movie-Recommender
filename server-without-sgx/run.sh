#! /bin/bash
g++ -o server server.cpp zmq_server.cpp -lzmq --std=c++11
./server