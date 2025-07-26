#!/bin/sh
g++ -fPIC -shared -std=c++11 userqasm.cpp -o userqasm.so
g++ -std=c++11 -rdynamic main.cpp qcs.cpp -o main
