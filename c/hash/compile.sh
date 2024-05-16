#!/bin/bash

echo compiling...
gcc -std=c99 hash.c main.c mtwister.c -o hash
echo done!