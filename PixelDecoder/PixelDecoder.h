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
#include <TH2.h>
#include <TCanvas.h>
#include <TPDF.h>

using ROCs = std::map<int, int>;
using Channels = std::map<int, ROCs>;

class Decoder {
private:
    Channels storage;
public:
    Decoder() { }
    virtual ~Decoder() { }
    void process(std::string file);
    void graph(std::string file);
};

#endif
