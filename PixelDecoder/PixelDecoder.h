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
#include <TCanvas.h>
#include <TH2.h>
#include <TH2D.h>
#include <TPDF.h>

using ROCs = std::map<int, int>;
using Channels = std::map<int, ROCs>;

class Decoder {
private:
    Channels storage;
    TH2D hFEDChan = TH2D("hChanFED", "Hits per Channel;Channel;Number of Hits", 48, 1., 49., 300, -0.5, 599.5);
public:
    Decoder() { hFEDChan.SetOption("COLZ"); }
    virtual ~Decoder() { }
    int open(std::string file, int chanBase);
    int decodeRoc32(uint32_t line, int chanID, int count);
    int decodeRoc64(uint64_t line, int chanID, int count);
    void process();
};

#endif
