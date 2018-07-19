// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#ifndef PIXELDECODER_H
#define PIXELDECODER_H

#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string>
#include <utility>
#include <TCanvas.h>
#include <TH2.h>
#include <TH2D.h>
#include <TPDF.h>

class Decoder {
private:
    std::vector< std::pair<int, int> > hitmap;
    int maxhits = 0;
public:
    Decoder() { hFEDChan.SetOption("COLZ"); }
    virtual ~Decoder() { }
    int open(std::string file, int chanBase);
    int decodeRoc32(uint32_t line, int chanID, int count);
    int decodeRoc64(uint64_t line, int chanID, int count);
    void process(std::string path);
};

#endif
