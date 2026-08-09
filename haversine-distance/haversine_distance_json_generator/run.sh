#!/bin/bash

g++ -g ../haversine-formula/haversine-formula.h  \
../haversine-formula/haversine-formula.cpp  \
./haversine_distance_json_generator.cpp \
-o haversine_distance_json_generator.out

