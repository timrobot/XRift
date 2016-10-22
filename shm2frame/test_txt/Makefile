CXX = g++
CXXFLAGS = -std=c++11 -O1 -pthread
LIBS =

all: source sink

source: source.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

sink: sink.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rvf source sink
