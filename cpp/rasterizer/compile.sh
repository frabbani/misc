#!/bin/bash

echo compiling...
g++ -std=c++17 bitmap.cpp file.cpp main.cpp myvector.cpp rasterizer.cpp texture.cpp -o rasterizer
echo done!
