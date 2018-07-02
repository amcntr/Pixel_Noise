// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#include "PixelDecoder.h"

void Decoder::decodeRoc32(uint32_t line, int chanID, int count) {
    uint32_t buf = 0;
    uint32_t bits = 32 / count;

    for(uint32_t i = 0; i < count; i++) {
        buf = line << (i * bits);
        buf >>= (count * bits) - bits;
        storage[chanID][i] += (int)buf;
    }
}

void Decoder::decodeRoc64(uint32_t line, int chanID, int count) {
    uint64_t buf = 0;
    uint64_t bits = 64 / count;
    std::cout<<std::bitset<64>(line)<<'\n';
    for(uint64_t i = 0; i < count; i++) {
        buf = line << (i * bits);
        buf >>= (count * bits) - bits;
        storage[chanID][i] += (int)buf;
    }
}

int Decoder::open(std::string filename, int chanMulti) {
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
        int chanID = 1 + (i * 16 * chanMulti);
        switch (header[i]) {
            case 0:
                std::cout << "Processing Layer 1.\n";
                for (int j = 0; j < block; j++) {
                    file.read( (char*) &line32, 4);
                    decodeRoc32(line32, chanID, 2);
                    chanID++;
                    if ( chanID > (16 + (i * 16 * chanMulti)) )
                        chanID = 1 + (i * 16 * chanMulti);
                }
                break;
            case 1:
                std::cout << "Processing Layer 2.\n";
                for (int j = 0; j < block; j++) {
                    file.read( (char*) &line32, 4);
                    decodeRoc32(line32, chanID, 4);
                    chanID++;
                    if ( chanID > (16 + (i * 16 * chanMulti)) )
                        chanID = 1 + (i * 16 * chanMulti);
                }
                break;
            case 2:
                std::cout << "Processing Layer 3-4 and fpix, 32 bit.\n";
                for (int j = 0; j < block; j++) {
                    file.read( (char*) &line32, 4);
                    decodeRoc32(line32, chanID, 8);
                    chanID++;
                    if ( chanID > (16 + (i * 16 * chanMulti)) )
                        chanID = 1 + (i * 16 * chanMulti);
                }
                break;
            case 3:
                std::cout << "Processing Layer 3-4 and fpix, 64 bit.\n";
                for (int j = 0; j < block; j++) {
                    file.read( (char*) &line64, 8);
                    decodeRoc64(line64, chanID, 8);
                    chanID++;
                    if ( chanID > (16 + (i * 16 * chanMulti)) )
                        chanID = 1 + (i * 16 * chanMulti);
                }
                break;
            default:
                std::cout<<"Error: Incorrect header format.\n";
        }
    }
    file.close();
    return 1;
}

void Decoder::graph(std::string filename) {
    int size = storage.size();
    std::cout << "Number of Channels: " << size << '\n';
    for (auto const& chan : storage) {
        std::cout<<"Number of ROCs in channel " << chan.first
            << ": " << chan.second.size() << '\n';
        for (auto const& roc : chan.second) {
            std::cout<<"\tHits in ROC " << roc.first
                << ": " << roc.second << '\n';
        }
    }
}

int main(int argc, char* argv[]) {
    clock_t t1, t2;
    t1 = clock();
    Decoder decode;
    int error;
    if (decode.open("SRAMhit0.bin", 1) != 1)
        std::cout<<"Error: Missing SRAMhit0.bin in directory.\n";

    decode.graph("");

    t2 = clock();
    float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
    std::cout << "\n\nProgram finish with a runtime of " << seconds << " seconds.\n\n";
    return 1;
}
