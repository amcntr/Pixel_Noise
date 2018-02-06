CC=g++
CXXFLAGS = -g $(shell root-config --cflags)
LDFLAGS = -g $(shell root-config --ldflags)
LDLIBS = $(shell root-config --libs) $(shell root-config --libs)

PixelEncoder: PixelEncoder.cpp
	$(CC) $(LDFLAGS) -o PixelEncoder PixelEncoder.cpp $(CXXFLAGS) $(LDLIBS)
