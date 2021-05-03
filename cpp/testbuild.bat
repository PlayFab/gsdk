@echo off
rmdir /s /q out
mkdir out
pushd out
cmake ..
cmake --build .
