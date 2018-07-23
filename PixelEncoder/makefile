CC=g++
CXXFLAGS = -g $(shell root-config --cflags)
LDFLAGS = -g $(shell root-config --ldflags)
LDLIBS = $(shell root-config --libs)

TARGET ?= PixelEncoder
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) $(CXXFLAGS) $(LDLIBS) $(OBJS) -o $@

.PHONY: clean
clean:
	rm $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)

