// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#ifndef PIXELDECODER_H
#define PIXELDECODER_H

#include "Includes.h"

class Decoder {
private:
    std::map<int, std::vector<int> > hitmap;
    int maxhits;
public:
    Decoder() { maxhits = 0; hitmap.clear(); }
    virtual ~Decoder() { }
    int open(std::string file, int chanBase);
    int decodeRoc32(uint32_t line, int count);
    int decodeRoc64(uint64_t line, int count);
    void process(std::string path);
};

#endif
