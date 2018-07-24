// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#include "Decoder.h"

int Decoder::decodeRoc32(uint32_t line, int chanID, int count) {
    int hits = 0;
    uint32_t buf = 0;
    uint32_t bits = 32 / count;
    for(uint32_t i = 0; i < count; i++) {
        buf = line << (i * bits);
        buf >>= (count * bits) - bits;
        hits += (int)buf;
    }
    return hits;
}

int Decoder::decodeRoc64(uint64_t line, int chanID, int count) {
    int hits = 0;
    uint64_t buf = 0;
    uint64_t bits = 64 / count;
    for(uint64_t i = 0; i < count; i++) {
        buf = line << (i * bits);
        buf >>= (count * bits) - bits;
        hits += (int)buf;
    }
    return hits;
}

int Decoder::open(std::string filename, int chanBase) {
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
    const int FILESIZE = 8388612;
    int blocksize = (FILESIZE - 4) / 16;
    if ((int)file.tellg() != FILESIZE)
        return 0;
    file.seekg(0);
    uint32_t headerBuffer;
    uint32_t header;
    uint32_t line32;
    uint64_t line64;
    file.read((char*)&headerBuffer, 4);
    for (int i = 0; i < 16; i++) {
        header = headerBuffer << (i * 2);
        header >>= (30);
        int chanID = chanBase + i;
        std::cout << "Processing channel " << i << '\n';
        int hits = 0;
        switch (header) {
            case 0:
                for (int j = 0; j < blocksize / 4; j) {
                    file.read( (char*) &line32, 4);
                    hits = decodeRoc32(line32, chanID, 2);
                    hFEDChan.Fill(chanID, hits);
                    line32 = 0;
                }
                break;
            case 1:
                for (int j = 0; j < blocksize / 4; j++) {
                    file.read( (char*) &line32, 4);
                    hFEDChan.Fill(chanID, decodeRoc32(line32, chanID, 4));
                    line32 = 0;
                }
                break;
            case 2:
                for (int j = 0; j < blocksize / 4; j++) {
                    file.read( (char*) &line32, 4);
                    hFEDChan.Fill(chanID, decodeRoc32(line32, chanID, 4));
                    line32 = 0;
                }
                break;
            case 3:
                for (int j = 0; j < blocksize / 8; j++) {
                    file.read( (char*) &line64, 8);
                    hFEDChan.Fill(chanID, decodeRoc64(line64, chanID, 8));
                    line64 = 0;
                }
                break;
            default:
                std::cout<<"Error: Incorrect header format.\n";
        }
    }
    file.close();
    return 1;
}

void Decoder::graph(std::string path) {
    TCanvas* canvas = new TCanvas("canvas");
    hFEDChan.Draw();
    std::string filename = path + "histogram_binary.pdf";
    canvas->Print(filename.c_str());
}
