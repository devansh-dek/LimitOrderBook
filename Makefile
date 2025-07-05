LOB_BIN = lob
CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall

SRC = $(wildcard src/*.cpp)
INC = -I include

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(INC) -o $(LOB_BIN)

clean:
	rm -f $(LOB_BIN)
