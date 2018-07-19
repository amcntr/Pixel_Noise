// Pixel Decoder
// Anthony McIntyre, July 2018
// Decodes binary file to pixels for consistency checking

#include "PixelDecoder.h"

int Decoder::decodeRoc32(uint32_t line, int count) {
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

int Decoder::decodeRoc64(uint64_t line, int count) {
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
    int hits;
    file.read((char*)&headerBuffer, 4);
    for (int i = 0; i < 16; i++) {
        header = headerBuffer << (i * 2);
        header >>= (30);
        int chanID = chanBase + i;
        std::cout << "Processing channel " << i << '\n';
        switch (header) {
            case 0:
                for (int j = 0; j < blocksize / 4; j) {
                	line32 = 0;
                	hits = 0;
                    file.read( (char*) &line32, 4);
                    hits = decodeRoc32(line32, 2);
                    hitmap[chanID].push_back(hits);
                    if (hits > maxhits) {maxhits = hits;}
                }
                break;
            case 1:
                for (int j = 0; j < blocksize / 4; j++) {
                	line32 = 0;
                	hits = 0;
                    file.read( (char*) &line32, 4);
                    hits = decodeRoc32(line32, 4);
                    hitmap[chanID].push_back(hits);
                    if (hits > maxhits) {maxhits = hits;}
                }
                break;
            case 2:
                for (int j = 0; j < blocksize / 4; j++) {
                	line32 = 0;
                	hits = 0;
                    file.read( (char*) &line32, 4);
                    hits = decodeRoc32(line32, 8);
                    hitmap[chanID].push_back(hits);
                    if (hits > maxhits) {maxhits = hits;}
                }
                break;
            case 3:
                for (int j = 0; j < blocksize / 8; j++) {
                	line64 = 0;
                	hits = 0;
                    file.read( (char*) &line64, 8);
                    hits = decodeRoc64(line64, 8);
                    hitmap[chanID].push_back(hits);
                    if (hits > maxhits) {maxhits = hits;}
                }
                break;
            default:
                std::cout<<"Error: Incorrect header format.\n";
        }
    }
    file.close();
    return 1;
}

void Decoder::process(std::string path) {
	std::cout<<maxhits<<'\n';
	TH2D hFEDChan = TH2D("Binary", "Hits per Channel in Binary Files;Channel;Number of Hits",
						 48, 1., 49., ((float)maxhits + ((float)maxhits * 0.5)),
						 -0.5, ((float)maxhits + ((float)maxhits * 0.5) - 0.5));
	hFEDChan.SetOption("COLZ");
	for (auto const& chan : hitmap) {
		for (int hits : chan.second) {
			hFEDChan.Fill(chan.first, hits);
		}
	}
    TCanvas* canvas = new TCanvas("canvas");
    hFEDChan.Draw();
    std::string filename = path + "histogram_binary.pdf";
    canvas->Print(filename.c_str());
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

    std::cout<<"Opening file SRAMhit0.bin\n";
    if (decode.open((path + "SRAMhit0.bin"), 1) != 1)
        std::cout<<"Error: Missing SRAMhit0.bin in directory.\n";
    std::cout<<"Opening file SRAMhit1.bin\n";
    if (decode.open((path + "SRAMhit1.bin"), 17) != 1)
        std::cout<<"Error: Missing SRAMhit1.bin in directory.\n";
    std::cout<<"Opening file SRAMhit2.bin\n";
    if (decode.open((path + "SRAMhit2.bin"), 33) != 1)
        std::cout<<"Error: Missing SRAMhit2.bin in directory.\n";

    decode.process(path);

    t2 = clock();
    float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
    std::cout << "\n\nProgram finish with a runtime of " << seconds << " seconds.\n\n";
    return 1;
}
