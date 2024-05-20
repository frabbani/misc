#!/bin/bash

echo compiling...
g++ -std=c++17 main.cpp json.cpp -ljq -lonig -o json
echo done!
