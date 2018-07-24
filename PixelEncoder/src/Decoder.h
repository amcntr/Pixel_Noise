// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#ifndef DECODER_H
#define DECODER_H

#include "Includes.h"

class Decoder {
private:
    TH2D hFEDChan = TH2D("hChanFED", "Binary Hits per Channel;Channel;Number of Hits", 48, 1., 49., 400, -0.5, 399.5);
public:
    Decoder() { hFEDChan.SetOption("COLZ"); }
    virtual ~Decoder() { }
    int open(std::string file, int chanBase);
    int decodeRoc32(uint32_t line, int chanID, int count);
    int decodeRoc64(uint64_t line, int chanID, int count);
    void graph(std::string path = "");
};

#endif
