CXX = g++
CXXFLAGS = -std=c++11 -O1 -pthread `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all: source sink

source: source.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

sink: sink.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rvf source sink
