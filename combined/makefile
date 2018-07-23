CC=g++
CXXFLAGS = -g $(shell root-config --cflags)
LDFLAGS = -g $(shell root-config --ldflags)
LDLIBS = $(shell root-config --libs)

LINK_TARGET = PixelEncoder

OBJS = Encoder.o Decoder.o Main.o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

clean :
	rm -f $(REBUILDABLES)
	echo Clean done

all : $(LINK_TARGET)
	echo All done

$(LINK_TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) $(CXXFLAGS) $(LDLIBS) -o $@ -c $<

Main.o : Main.cpp
	$(CC) $(LDFLAGS) $(CXXFLAGS) $(LDLIBS) -o $@ -c $<

Decoder.o : Decoder.cpp
	$(CC) $(LDFLAGS) $(CXXFLAGS) $(LDLIBS) -o $@ -c $<

Encoder.o : Encoder.cpp
	$(CC) $(LDFLAGS) $(CXXFLAGS) $(LDLIBS) -o $@ -c $<

Main.o : Decoder.h Encoder.h Includes.h
Decoder.o : Decoder.h Includes.h
Encoder.o : Encoder.h Includes.h
