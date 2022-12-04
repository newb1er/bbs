OUT = bbs
FILENAME = bbs.cpp
CXX = /usr/bin/g++
CXXFLAGS = -std=c++17 -Wall -O3

all: $(FILENAME)
	$(CXX) $(CXXFLAGS) bbs.cpp -o $(OUT)

clean:
	rm -f $(OUT)