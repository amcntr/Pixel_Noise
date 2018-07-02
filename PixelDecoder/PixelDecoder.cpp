// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#include "PixelDecoder.h"

int Decoder::decodeRoc32(uint32_t line, int chanID, int count) {
    int hits = 0;
    uint32_t buf = 0;
    uint32_t bits = 32 / count;
    for(uint32_t i = 0; i < count; i++) {
        buf = line << (i * bits);
        buf >>= (count * bits) - bits;
        storage[chanID][i + 1] += (int)buf;
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
        storage[chanID][i + 1] += (int)buf;
        hits += (int)buf;
    }
    return hits;
}

int Decoder::open(std::string filename, int chanBase) {
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
    if ((int)file.tellg() != 8388609)
        return 0;
    int block = ((int)file.tellg() - 1) / 4;
    file.seekg(0);
    uint8_t headerBuffer;
    uint8_t hbuf;
    int header[4];
    uint32_t line32;
    uint64_t line64;
    ROCs rocBuffer;
    file.read((char*)&headerBuffer, 1);
    for (int i = 0; i < 4; i++) {
        hbuf = headerBuffer << (i * 2);
        hbuf >>= (6);
        header[i] = (int)hbuf;
        int chanID = chanBase + (i * 4);
        std::cout << "Processing block " << i << ' ';
        switch (header[i]) {
            case 0:
                std::cout << "Layer 1.\n";
                for (int j = 0; j < block / 4; j) {
                    file.read( (char*) &line32, 4);
                    hFEDChan.Fill(chanID, decodeRoc32(line32, chanID, 2));
                    chanID++;
                    line32 = 0;
                    if ( chanID > ((chanBase + 3) + (i * 4)) )
                        chanID = chanBase + (i * 4);
                }
                break;
            case 1:
                std::cout << "Layer 2.\n";
                for (int j = 0; j < block / 4; j++) {
                    file.read( (char*) &line32, 4);
                    hFEDChan.Fill(chanID, decodeRoc32(line32, chanID, 4));
                    chanID++;
                    line32 = 0;
                    if ( chanID > ((chanBase + 3) + (i * 4)) )
                        chanID = chanBase + (i * 4);
                }
                break;
            case 2:
                std::cout << "Layer 3-4 and fpix, 32 bit.\n";
                for (int j = 0; j < block / 4; j++) {
                    file.read( (char*) &line32, 4);
                    hFEDChan.Fill(chanID, decodeRoc32(line32, chanID, 8));
                    chanID++;
                    line32 = 0;
                    if ( chanID > ((chanBase + 3) + (i * 4)) )
                        chanID = chanBase + (i * 4);
                }
                break;
            case 3:
                std::cout << "Layer 3-4 and fpix, 64 bit.\n";
                for (int j = 0; j < block / 8; j++) {
                    file.read( (char*) &line64, 8);
                    hFEDChan.Fill(chanID, decodeRoc64(line64, chanID, 8));
                    chanID++;
                    line64 = 0;
                    if ( chanID > ((chanBase + 3) + (i * 4)) )
                        chanID = chanBase + (i * 4);
                }
                break;
            default:
                std::cout<<"Error: Incorrect header format.\n";
        }
    }
    file.close();
    return 1;
}

void Decoder::process() {
    int size = storage.size();
    int hits;
    std::cout << "Number of Channels: " << size << '\n';
    for (auto const& chan : storage) {
        std::cout<<"Number of ROCs in channel " << chan.first
            << ": " << chan.second.size() << '\n';
        hits = 0;
        for (auto const& roc : chan.second) {
            std::cout<<"\tHits in ROC " << roc.first
                << ": " << roc.second << '\n';
        }
    }
    TCanvas* canvas = new TCanvas("canvas");
    hFEDChan.Draw();
    canvas->Print("HitsPerChannel.pdf");
}

int main(int argc, char* argv[]) {
    clock_t t1, t2;
    t1 = clock();
    if (argc != 2) {  // if no arguments
        std::cout << "usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    std::string path = argv[1];
    Decoder decode;
    int error;
    std::cout<<"Opening file SRAMhit0.bin\n";
    if (decode.open((path + "SRAMhit0.bin"), 1) != 1)
        std::cout<<"Error: Missing SRAMhit0.bin in directory.\n";
    std::cout<<"Opening file SRAMhit1.bin\n";
    if (decode.open((path + "SRAMhit1.bin"), 17) != 1)
        std::cout<<"Error: Missing SRAMhit1.bin in directory.\n";
    std::cout<<"Opening file SRAMhit2.bin\n";
    if (decode.open((path + "SRAMhit2.bin"), 33) != 1)
        std::cout<<"Error: Missing SRAMhit2.bin in directory.\n";

    decode.process();

    t2 = clock();
    float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
    std::cout << "\n\nProgram finish with a runtime of " << seconds << " seconds.\n\n";
    return 1;
}
