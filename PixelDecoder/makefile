CC=g++
CXXFLAGS = -g $(shell root-config --cflags)
LDFLAGS = -g $(shell root-config --ldflags)
LDLIBS = $(shell root-config --libs) $(shell root-config --libs)

PixelDecoder: PixelDecoder.cpp
	$(CC) $(LDFLAGS) -o PixelDecoder PixelDecoder.cpp $(CXXFLAGS) $(LDLIBS)
