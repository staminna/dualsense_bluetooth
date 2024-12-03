gcc -o ps5_kontroller main.cpp \
    -I/Library/Frameworks/SDL2.framework/Headers \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -F/Library/Frameworks \
    -framework SDL2 \
    -llo

