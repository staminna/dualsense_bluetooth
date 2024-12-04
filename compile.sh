#!/bin/bash

g++ -o ps5_kontroller main.cpp \
    -I/Library/Frameworks/SDL2.framework/Headers \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -F/Library/Frameworks \
    -framework SDL2 \
    -llo \
    -lhidapi \
    -Wl,-rpath,/Library/Frameworks

install_name_tool -add_rpath /opt/homebrew/lib ./ps5_kontroller

./ps5_kontroller