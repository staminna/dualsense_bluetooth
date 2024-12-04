g++ -o ps5_kontroller main.cpp -I/Library/Frameworks/SDL2.framework/Headers -I/opt/homebrew/include -L/opt/homebrew/lib -F/Library/Frameworks -framework SDL2 -llo -lhidapi
otool -L ps5_kontroller
install_name_tool -add_rpath /Library/Frameworks ps5_kontroller
#nohup ./ps5_kontroller > log.txt 2>&1 &
./ps5_kontroller &
