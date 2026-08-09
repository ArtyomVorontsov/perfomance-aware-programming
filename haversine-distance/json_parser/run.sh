#!/bin/bash

g++ -g ./tokenizer/tokenizer.h  \
./parser/parser.h  \
./tokenizer/tokenizer.cpp  \
./parser/parser.cpp \
-o parser.out

