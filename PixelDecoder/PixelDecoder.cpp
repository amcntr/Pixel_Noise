// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#include "PixelDecoder.h"

void Decoder::process(std::string filename) {
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    uint32_t headerBuffer;
    file.read((char*)&headerBuffer, 1);
    std::cout << std::bitset<8>(headerBuffer) << '\n';
}

void Decoder::graph(std::string filename) {

}

int main(int argc, char* argv[]) {
    Decoder decode;
    decode.process("SRAMhit0.bin");
    return 1;
}
