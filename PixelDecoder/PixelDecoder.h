// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#ifndef PIXELDECODER_H
#define PIXELDECODER_H

#include <iostream>
#include <fstream>
#include <time.h>
#include <map>
#include <vector>
#include <string>
#include <bitset>

using ROCs = std::map<int, int>;
using Channels = std::map<int, ROCs>;

class Decoder {
private:
    Channels storage;
public:
    Decoder() { }
    virtual ~Decoder() { }
    int open(std::string file, int chanMulti);
    void decodeRoc32(uint32_t line, int chanID, int count);
    void decodeRoc64(uint64_t line, int chanID, int count);
    void graph(std::string file);
};

#endif
