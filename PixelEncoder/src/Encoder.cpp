// PixelEncoder
// Anthony McIntyre - June 2018
// Using pixel data stored in a root TTree;
// finds the fed id with the highest avg hits,
// encodes pixel addresses into a binary format
// for FED testing.
//
// It might be possible to merge this code into
// a CMSSW framework analyzer program.
//
// Format:
//  for hits per roc         for pixel addresses
//  SRAMhit#.bin             SRAMpix#.bin
//   32 bit blocks            32 bit blocks
//   or 64 bit blocks          0x[25:16][13:8][7:0]
//    0x[roc1][roc2]...        0x[row][col][adc]


#include "Encoder.h"

// checks if pixel is already stored and adds to container
// for events with zero hits, add layer 0
int Encoder::add(int event,
                     int fed,
                     int layer,
                     int ch,
                     int roc,
                     int row,
                     int col,
                     int adc) {
    eventStore_[event] = 1;
    // layer 0 is for events with 0 hits
    if ((layer > 0) || (roc > 0)) {
        // merge row and col into unique number by bit shifting them
        uint32_t rowcol = ((uint32_t)row << 16 | (uint32_t)col << 8);
        ChannelLayer_[ch] = layer;
        auto pix = storage[fed][event][ch][roc].find(rowcol);
        if (pix == storage[fed][event][ch][roc].end()) {
            storage[fed][event][ch][roc][rowcol] = (uint32_t)adc;
            hitspFED_[fed] += 1;
            // return 0 for no duplicate counting
            return 0;
        } else {
            // duplicate found, return 1
            return 1;
        }
    } else {
        zeroEvents_[fed][event] += 1;
        for (int c = 1; c < 49; c++){
            storage[fed][event][c][-1][(uint32_t)(0)] = (uint32_t)(0);
        }
        return 0;
    }
}

void Encoder::process() {
    haFEDhit = 0;
    totalHits = 0;
    totalZeroEvents = 0;
    hhRoc = 0;
    hhChan = 0;
    totalEvents = eventStore_.size();
    totalFEDs = hitspFED_.size();
    for (auto const& fid : hitspFED_) {
        int avg = fid.second / totalEvents;
        totalHits += fid.second;
        if (avg > haFEDhit) {
            haFEDhit = avg;
            haFEDID = fid.first;
        }
    }
    totalZeroEvents = zeroEvents_[haFEDID].size();
    for (auto const& event : storage[haFEDID]) {
        for (auto const& ch : event.second) {
            int hits = 0;
            for (auto const& roc : ch.second) {
                if ((roc.second.size() > 15) && (ChannelLayer_[ch.first] > 2))
                    rocHigHitpFile_ = true;
                if (roc.first > 0) {
                    if (roc.second.size() > hhRoc) {
                        hhRoc = roc.second.size();
                    }
                    hits += roc.second.size();
                }
            }
            if (hits > hhChan) {
                hhChan = hits;
            }
        }
    }
}

// convert data in map structure to binary format
// and place in a buffer for file writing.
void Encoder::encode(int targetFED) {
    // These are buffers for writing the data to files
    // 1 block for each channel
    // 16 channels per file
    std::vector<uint64_t> RocFileBuffer[48];
    // For the header file of the SRAMhit files
    // indicates the binary format used
    // 2 bits per block
    // 0: 2 rocs, 32bit
    // 1: 4 rocs, 32bit
    // 2: 8 rocs, 32bit
    // 3: 8 rocs, 64bit
    uint32_t BlockType[48];
    int emptyCh = 0;
    int hitCh = 0;
    // buffer for pixel address binary
    std::vector<uint32_t> PixAdd[3];
    // buffer for hits per roc
    std::unordered_map<int, uint64_t> hits;
    // loop over events in target fed id
    for (auto const& event : storage[haFEDID]) {
        // loop over all channels
        for (int ch = 1; ch < 49; ch++) {
            // if event is registered as a zero event
            // or channel has no registered hits
            // push zero hits for channel
            if ((event.second.count(ch) == 0) || (zeroEvents_[haFEDID][event.first] > 0)) {
                RocFileBuffer[ch - 1].push_back((uint64_t)0);
                emptyCh++;
            } else {
                hitCh++;
                for (auto const& roc : storage[haFEDID][event.first][ch]) {
                    for (auto const& pix : roc.second) {
                        // convert pixel addresses into binary
                        // and push into pix buffer
                        uint32_t addressBuffer = 0;
                        addressBuffer = (pix.first | pix.second);
                        int index = (int)ceil((float)ch / 16.0) - 1;
                        PixAdd[index].push_back(addressBuffer);
                    }
                    // store rocs that have registered hits
                    hits[roc.first] = (uint64_t)roc.second.size();
                }
                uint64_t hitBuffer = 0;
                switch (ChannelLayer_[ch]) {
                    case 1:
                    BlockType[ch - 1] = 0;
                    for (int r = 1; r < 3; r++) {
                        if (hits.count(r) > 0)
                            hitBuffer = (hitBuffer << 16 | hits[r]);
                        else
                            hitBuffer <<= 16;
                    }
                    break;
                    case 2:
                    BlockType[ch - 1] = 1;
                    for (int r = 1; r < 5; r++) {
                        if (hits.count(r) > 0)
                            hitBuffer = (hitBuffer << 8 | hits[r]);
                        else
                            hitBuffer <<= 8;
                    }
                    break;
                    default:
                    if (rocHigHitpFile_) {
                        BlockType[ch - 1] = 3;
                        for (int r = 1; r < 9; r++) {
                            if (hits.count(r) > 0)
                                hitBuffer = (hitBuffer << 8 | hits[r]);
                            else
                                hitBuffer <<= 8;
                        }
                    } else {
                        BlockType[ch - 1] = 2;
                        for (int r = 1; r < 9; r++) {
                            if (hits.count(r) > 0)
                                hitBuffer = (hitBuffer << 4 | hits[r]);
                            else
                                hitBuffer <<= 4;
                        }
                    }
                }
                RocFileBuffer[ch - 1].push_back(hitBuffer);
                hits.clear();
            }
        }
    }

    // checks if buffer sizes match
    for (int i = 0; i < 48; i++) {
        std::cout << "Roc Hit Buffer " << i << " size: " 
                  << RocFileBuffer[i].size() << '\n'; 
        //if (RocFileBuffer[i].size() != RocFileBuffer[i - 1].size())
        //    throw std::length_error("Block sizes do not match.");
    }
    for (int i = 0; i < 3; i++) {
        std::cout << "Pixel Address Buffer " << i
                  << "size: " << PixAdd[i].size() << '\n';
    }
    std::cout << "\nNumber of channels with zero hits: " << emptyCh
              << "\nNumer of channels with hits: " << hitCh
              << "\n64-bit binary files: ";
    if (rocHigHitpFile_)
        std::cout<<"True\n";
    else
        std::cout<<"False\n";

    // These files have to be an exact file size.
    // So it loops over the data until the file size is met.
    // The size in this case is 2^21 32bit registers or around 8.39 MB
    std::string filename;
    std::ofstream glibhit[3];
    std::ofstream glibpix[3];
    // Filesize in registers
    const int FILESIZE = 2097152; // 2^21
    const int BLOCKSIZE = 131072; // 2^17
    // begin writing files.
    for (int filenum = 0; filenum < 3; filenum++) {
        filename = "SRAMhit" + std::to_string(filenum) + ".bin";
        glibhit[filenum].open(filename.c_str(), std::ios::binary | std::ios::out);
        filename = "SRAMpix" + std::to_string(filenum) + ".bin";
        glibpix[filenum].open(filename.c_str(), std::ios::binary | std::ios::out);
        
        int position = 0;
        
        uint32_t header = 0;
        for (int block = 0; block < 16; block++) {
            int index = block + (filenum * 16);
            header = (header << 2 | BlockType[index]);
        }
        glibhit[filenum].write((char*)&header, 4);
        
        for (int block = 0; block < 16; block++) {
            position = 0;
            int index = block + (filenum * 16);
            if (rocHigHitpFile_) {
                for (int registers = 0; registers < BLOCKSIZE; registers += 2) {
                    if (position == RocFileBuffer[index].size())
                        position = 0;
                    glibhit[filenum].write((char*)&RocFileBuffer[index][position], 8);
                    position++;
                }
            } else {
                for (int registers = 0; registers < BLOCKSIZE; registers++) {
                    if (position == RocFileBuffer[index].size())
                        position = 0;
                    glibhit[filenum].write((char*)&RocFileBuffer[index][position], 4);
                    position++;
                }
            }
        }
        
        position = 0;
        for (int registers = 0; registers < FILESIZE; registers++) {
            if ((unsigned int)position == PixAdd[filenum].size())
                position = 0;
            glibpix[filenum].write((char*)&PixAdd[filenum][position], 4);
            position++;
        }
        glibhit[filenum].close();
        glibpix[filenum].close();
    }
}


void Encoder::graph() {
    TCanvas* canvas = new TCanvas("canvas");
    TH2D *hChanROC[48], *hFEDChan;
    std::string title = "Hits Per Channel in FED #" + std::to_string(haFEDID) +
                      " in Each Channel;Channel;Number of Hits";
    std::string name = "hChanFED" + std::to_string(haFEDID);
    hFEDChan = new TH2D(name.c_str(), title.c_str(), 48, 1., 49.,
                      ((float)hhChan + ((float)hhChan * 0.5)), -0.5,
                      ((float)hhChan + ((float)hhChan * 0.5) - 0.5));
    hFEDChan->SetOption("COLZ");
    int chanHits = 0;
    for (auto const& event : storage[haFEDID]) {
        for (int ch = 1; ch < 49; ch++) {
            for (int rc = 1; rc < 9; rc++) {
                auto roc = storage[haFEDID][event.first][ch].find(rc);
                if (roc != storage[haFEDID][event.first][ch].end()) {
                    chanHits += storage[haFEDID][event.first][ch][rc].size();
                }
            }
            hFEDChan->Fill(ch, chanHits);
            chanHits = 0;
        }
    }
    hFEDChan->Draw();
    title = "Title:Source - Hits per channel in FED #" + std::to_string(haFEDID);
    canvas->Print("histogram_source.pdf", title.c_str());
}
