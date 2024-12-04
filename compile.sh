#!/bin/bash

g++ -o ps5_kontroller main.cpp \
    -I/opt/homebrew/include/SDL2 \
    -I/opt/homebrew/include \
    -D_THREAD_SAFE \
    -L/opt/homebrew/lib \
    -lSDL2 \
    -llo

install_name_tool -add_rpath /opt/homebrew/lib ./ps5_kontroller
