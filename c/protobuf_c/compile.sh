#!/bin/bash

echo compiling...
gcc -std=c11 main.c message.pb-c.c -lprotobuf-c -o protobuf_c
echo done!