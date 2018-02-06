CC=g++
CXXFLAGS = -g $(shell root-config --cflags)
LDFLAGS = -g $(shell root-config --ldflags)
LDLIBS = $(shell root-config --libs) $(shell root-config --libs)

PixelPacker: PixelPacker.cpp
	$(CC) $(LDFLAGS) -o PixelPacker PixelPacker.cpp $(CXXFLAGS) $(LDLIBS)
