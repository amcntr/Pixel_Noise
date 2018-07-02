// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#include "PixelDecoder.h"

std::vector<uint32_t> Decoder::decodeRoc(uint32_t line, int count) {
    std::vector<uint32_t> RocHits;
    uint32_t buf = 0;
    uint32_t bits = 32 / count;

    for(uint32_t i = 0; i < count; i++) {
        buf = line << (i * bits);
        buf >>= (count * bits) - bits;
        RocHits.push_back(buf);
    }
    return RocHits;
}

void Decoder::open(std::string filename) {
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    uint32_t headerBuffer;
    uint32_t header[4];
    file.read((char*)&headerBuffer, 1);
    for (int i = 0; i < 4; i++) {
        header[i] = headerBuffer << (i * 2);
        header[i] >>= (4 * 2) - 2;
        std::cout << header[i] << '\n';
    }
}

void Decoder::graph(std::string filename) {

}

int main(int argc, char* argv[]) {
    Decoder decode;
    decode.open("SRAMhit0.bin");
    return 1;
}
