CC = g++
CFLAGS = -I/opt/homebrew/include/SDL2 -std=c++11
LDFLAGS = -L/opt/homebrew/lib -lSDL2
SRC = test.cpp
OUT = build/ps5-kontroller

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OUT)

