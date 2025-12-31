#!/bin/sh
gcc -c add.c
gcc -c hello.c
gcc -o hello add.o hello.o

