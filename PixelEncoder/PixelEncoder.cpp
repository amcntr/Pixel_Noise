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
//  for hits per roc      for pixel addresses
//  SRAMhit#.bin        SRAMpix#.bin
//   32 bit blocks          32 bit blocks
//      0x[roc1][roc2]...         0x[25:16][13:8][7:0]
//    layer 1:            0x[row][col][adc]
//      2 rocs per 32bits
//    layer 2:
//      4 rocs per 32bits
//    layer 3-4 and FPix
//      8 rocs per 32bits

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>
#include <TH2D.h>
#include <TPDF.h>
#include <TROOT.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <bitset>

#include "PixelEncoder.h"

// checks if pixel is already stored and adds to container
// for events with zero hits, add layer 0
int Pixel_Store::add(int event,
                     int fed,
                     int layer,
                     int ch,
                     int roc,
                     int row,
                     int col,
                     int adc) {
  // layer 0 is for events with 0 hits
  if (layer > 0) {
    // merge row and col into unique number by bit shifting them
    uint32_t rowcol = ((uint32_t)row << 16 | (uint32_t)col << 8);

    if (!check(event, fed, layer, ch, roc, rowcol)) {
      storage[event][fed][layer][ch][roc][rowcol] = (uint32_t)adc;
      hitspFED_[fed] += 1;
      // return 0 for no duplicate counting
      return 0;
    } else
      // duplicate found, return 1
      return 1;
  } else {
    storage[event][fed][layer][0][0][(uint32_t)0] = (uint32_t)(0);
    return 0;
  }
}

// checks if pixel is stored in container
bool Pixel_Store::check(int event,
                        int fed,
                        int layer,
                        int chan,
                        int roc,
                        uint32_t rowcol) {
  Pixels::iterator pix = storage[event][fed][layer][chan][roc].find(rowcol);

  if (pix == storage[event][fed][layer][chan][roc].end())
    return false;

  return true;
}

void Pixel_Store::process() {
  haFEDhit = 0;
  totalHits = 0;
  hhChanhit = 0;
  hhROChit = 0;
  int Chhit = 0;
  totalEvents = storage.size();
  totalFEDs = hitspFED_.size();
  // for FEDID in "hits per fed" map
  for (auto const& fid : hitspFED_) {
    int avg = fid.second / totalEvents;
    totalHits += fid.second;
    if (avg > haFEDhit) {
      haFEDhit = avg;
      haFEDID = fid.first;
    }
  }
  for (auto const& event : storage) {
    for (auto const& fed : event.second) {
      if (fed.first == haFEDID) {
        for (auto const& lay : fed.second) {
          if (lay.first != 0) {
            std::cout<<lay.first<<' ';
            for (auto const& ch : lay.second) {
              for (auto const& roc : ch.second) {
                int index = (int)(ceil((float)ch.first / 16.0) - 1);
                  std::cout<<index<<'\n';
                if ((roc.second.size() > 15) && (lay.first > 2))
                  rocHigHitpFile_[index] = true;
                if (roc.second.size() > hhROChit)
                  hhROChit = roc.second.size();
                Chhit += roc.second.size();
              }
              if (Chhit > hhChanhit)
                hhChanhit = Chhit;
              Chhit = 0;
              chpLay_[lay.first][ch.first] += 1;
            }
          }
        }
      }
    }
  }
}

// outputs 12 binary files of "hits per roc" and pixel addresses
// for the fed during all events in data file. Half of the files
// are looped to the max file size. The other half are not looped.
void Pixel_Store::encode(int targetFED, std::string file_name) {
  std::string filename;
  std::ofstream glibhit[3];
  std::ofstream glibpix[3];

  // These are buffers for writing the data to files
  // 1 block for each channel
  // 16 channels per file
  std::vector<uint32_t> RocHits32[48];
  std::vector<uint64_t> RocHits64[48];
  // For the header file of the SRAMhit files
  // indicates the binary format used
  // 2 bits per block
  // 0: 2 rocs, 32bit
  // 1: 4 rocs, 32bit
  // 2: 8 rocs, 32bit
  // 3: 8 rocs, 64bit
  uint32_t BlockType[48];
  // counts the amount of events
  // with zero hits
  zeroEvents = 0;
  // buffer for pixel address binary
  std::vector<uint32_t> PixAdd[3];
  // buffer for hits per roc
  std::unordered_map<int, uint32_t> hits;
  // convert data in map structure to binary format
  // and place in a buffer for file writing.
  for (auto const& evt : storage) {
    for (auto const& fed : evt.second) {
      if (fed.first == targetFED) {
        for (auto const& lay : fed.second) {
          if (lay.first > 0) {
            for (auto const& ch : lay.second) {
              for (auto const& roc : ch.second) {
                for (auto const& pix : roc.second) {
                  // convert pixel addresses into binary
                  uint32_t addressBuffer = 0;
                  addressBuffer = (pix.first | pix.second);
                  if (ch.first < 17)
                    PixAdd[0].push_back(addressBuffer);
                  if ((ch.first < 33) && (ch.first > 16))
                    PixAdd[1].push_back(addressBuffer);
                  if (ch.first > 32)
                    PixAdd[2].push_back(addressBuffer);
                }
                // get rocid and hits on roc
                hits[roc.first] = (uint32_t)(roc.second.size());
              }
              // use rocid and hits on roc for conversion
              uint32_t hitBuffer32 = 0;
              uint64_t hitBuffer64 = 0;
              // index for pushing hit binary into file buffer
              int index = (int)(ch.first - 1);
              // diiferent layers have differenct # of rocs
              // create hit buffer from hit data and push into file buffer
              switch (lay.first) {
                case 1:  // layer 1
                  for (int rc = 1; rc < 3; rc++) {
                    if (hits.count(rc) > 0)
                      hitBuffer32 = (hitBuffer32 << 16 | hits[rc]);
                    else
                      hitBuffer32 <<= 16;
                  }
                  RocHits32[index].push_back(hitBuffer32);
                  break;
                case 2:  // layer 2
                  for (int rc = 1; rc < 5; rc++) {
                    if (hits.count(rc) > 0)
                      hitBuffer32 = (hitBuffer32 << 8 | hits[rc]);
                    else
                      hitBuffer32 <<= 8;
                  }
                  if (BlockType[index] < 1)
                    BlockType[index] = 1;
                  RocHits32[index].push_back(hitBuffer32);
                  break;
                default:  // layer 3-4 and fpix
                  if (rocHigHitpFile_[index / 4]) {
                    for (int rc = 1; rc < 9; rc++) {
                      if (hits.count(rc) > 0)
                        hitBuffer64 = (hitBuffer64 << 8 | hits[rc]);
                      else
                        hitBuffer64 <<= 8;
                    }
                    RocHits64[index].push_back(hitBuffer64);
                    BlockType[index] = 3;
                  } else {
                    for (int rc = 1; rc < 9; rc++) {
                      if (hits.count(rc) > 0)
                        hitBuffer32 = (hitBuffer32 << 4 | hits[rc]);
                      else
                        hitBuffer32 <<= 4;
                    }
                    if (BlockType[index] < 2)
                      BlockType[index] = 2;
                    RocHits32[index].push_back(hitBuffer32);
                  }
                  break;
              }
              hits.clear();
            }
          } else {
            // push rocs for events with zero hits
            zeroEvents += 1;
            for (int layer = 1; layer < 6; layer++) {
              for (int chan = 1; chan < 49; chan++) {
                if (chpLay_[layer].count(chan) > 0) {
                  int index = (int)ceil((float)chan / 16.0) - 1;
                  if ((rocHigHitpFile_[index]) && (layer > 2))
                    RocHits64[chan - 1].push_back((uint64_t)0);
                  else
                    RocHits32[chan - 1].push_back((uint32_t)0);
                }
              }
            }
          }
        }
      }
    }
  }
  // begin writing the files

  // These files have to be an exact file size.
  // So it loops over the data until the file size is met.
  // The size in this case is 2^21 32bit blocks or around 8.39 MB
  const int FILESIZE = 8388608; // filesize in bytes
  for (int i = 0; i < 3; i++) {
    filename = file_name + "hit" + std::to_string(i) + ".bin";
    glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    filename = file_name + "pix" + std::to_string(i) + ".bin";
    glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    int count = 0;

    // write the SRAMhit files
    // The SRAMhit files are divided into 4 blocks
    uint32_t header = 0;
    for (int j = 0; j < 16; j++) {
      int index = j + (i * 16);
      header = (header << 2 | BlockType[index]);
    }
    std::cout<<std::bitset<32>(header)<<"\n";
    glibhit[i].write((char*)&header, 4);
    int last = 0;
    int left = 0;
    // write the SRAMhit binary data
    if (rocHigHitpFile_[i]) {
      for (int j = 0; j < 16; j++) {
        count = 0;
        int index = j + (i * 16);
        for (int k = 0; k < (FILESIZE / 128); k++) {
          if ((unsigned int)count >= RocHits64[index].size()) {
            left = RocHits64[index].size();
            count = 0;
          }
          glibhit[i].write((char*)&RocHits64[index][count], 8);
          count++;
        }
        std::cout << "SRAMhit" << i << " 64-bit Ch" << (index + 1)
                  << "\tSize left: " << left
                  << "\tPosition: " << count
                  << "\tDifference: " << (left - count) << '\n';
      }
    } else {
      for (int j = 0; j < 16; j++) {
        count = 0;
        int index = j + (i * 16);
        for (int k = 0; k < (FILESIZE / 64); k++) {
          if ((unsigned int)count >= RocHits32[index].size()) {
            left = RocHits32[index].size();
            count = 0;
          }
          glibhit[i].write((char*)&RocHits32[index][count], 4);
          count++;
        }
        std::cout << "SRAMhit" << i << " 32-bit Ch" << (index + 1)
                  << "\tSize left: " << left
                  << "\tPosition: " << count
                  << "\tDifference: " << (left - count) << '\n';
      }
    }
    // write the SRAMpix files
    count = 0;
    for (int j = 0; j < (FILESIZE / 4); j++) {
      if ((unsigned int)count >= PixAdd[i].size())
        count = 0;
      glibpix[i].write((char*)&PixAdd[i][count], 4);
      count++;
    }

    glibhit[i].close();
    glibpix[i].close();
  }
}

void Pixel_Store::graph() {
  TCanvas* canvas = new TCanvas("canvas");
  TH2D *hChanROC[48], *hFEDChan;
  std::string title = "Hits in FED #" + std::to_string(haFEDID) +
                      " in Each Channel;Channel;Number of Hits";
  std::string name = "hChanFED" + std::to_string(haFEDID);
  hFEDChan = new TH2D(name.c_str(), title.c_str(), 48, 1., 49.,
                      ((float)hhChanhit + ((float)hhChanhit * 0.5)), -0.5, ((float)hhChanhit + ((float)hhChanhit * 0.5) - 0.5));
  hFEDChan->SetOption("COLZ");
  for (int i = 0; i < 48; i++) {
    title = "Hits per ROC in Channel #" + std::to_string(i + 1) +
            ";ROC;Number of Hits";
    name = "hROCChan" + std::to_string(i + 1);
    hChanROC[i] = new TH2D(name.c_str(), title.c_str(), 8, 1., 9.,
                           ((float)hhROChit + ((float)hhROChit * 0.5)), -0.5, ((float)hhROChit + ((float)hhROChit * 0.5) - 0.5));
    hChanROC[i]->SetOption("COLZ");
  }
  int chanHits = 0;
  for (auto const& event : storage) {
    for (auto const& fed : event.second) {
      if (fed.first == haFEDID) {
        for (auto const& layer : fed.second) {
          if (layer.first > 0) {
            for (auto const& chan : layer.second) {
              for (auto const& roc : chan.second) {
                if (roc.first > 0) {
                  hChanROC[chan.first - 1]->Fill(roc.first, roc.second.size());
                }
                chanHits += roc.second.size();
              }
              hFEDChan->Fill(chan.first, chanHits);
              chanHits = 0;
            }
          } else {
            for (int lay = 1; lay < 6; lay++) {
              for (int ch = 1; ch < 49; ch ++) {
                if (chpLay_[lay].count(ch) > 0) {
                  hFEDChan->Fill(ch, 0); 
                }
              }
            }
          }
        }
      }
    }
  }
  canvas->Print("histograms.pdf[");
  hFEDChan->Draw();
  title = "Title:Hits per channel in FED #" + std::to_string(haFEDID);
  canvas->Print("histograms.pdf", title.c_str());
  /*for (int i = 0; i < 48; i++) {
    title = "Title:Hits per ROC in channel #" + std::to_string(i + 1);
    hChanROC[i]->Draw();
    canvas->Print("histograms.pdf", title.c_str());
  }*/
  canvas->Print("histograms.pdf]");
}

int main(int argc, char* argv[]) {
  // clock to record process time
  clock_t t1, t2, st1, st2, et1, et2;

  if (argc != 2) {  // if no arguments
    std::cout << "usage: " << argv[0] << " <filename>\n";
    return 1;
  }

  TFile* file = new TFile(argv[1]);
  // check if file loaded correctly
  if (!(file->IsOpen())) {
    std::cout << "Couln't open file.\n";
    return 1;
  }

  t1 = clock();
  std::cout << "Program start.\n";

  // decode data from TTree
  TTreeReader readerH("HighFedData", file);
  TTreeReaderValue<int> event(readerH, "Data._eventID");
  TTreeReaderValue<int> fed(readerH, "Data._fedID");
  TTreeReaderValue<int> layer(readerH, "Data._layer");
  TTreeReaderValue<int> chan(readerH, "Data._channel");
  TTreeReaderValue<int> roc(readerH, "Data._ROC");
  TTreeReaderValue<int> row(readerH, "Data._row");
  TTreeReaderValue<int> col(readerH, "Data._col");
  TTreeReaderValue<int> adc(readerH, "Data._adc");

  TTreeReader readerZ("ZeroData", file);
  TTreeReaderValue<int> event0(readerZ, "Data._eventID");
  TTreeReaderValue<int> fed0(readerZ, "Data._fedID");
  TTreeReaderValue<int> layer0(readerZ, "Data._layer");

  // declare output file to store output information
  std::ofstream outputFile;
  outputFile.open("output.txt");

  Pixel_Store pStore;

  st1 = clock();
  std::cout << "\nStoring pixels...\n";
  // stores duplicate pixel amount
  int duplicates = 0;
  // loop through TTree and store data in Pixel_Store
  while (readerH.Next()) {
    duplicates +=
        pStore.add(*event, *fed, *layer, *chan, *roc, *row, *col, *adc);
  }
  while (readerZ.Next()) {
    duplicates += pStore.add(*event0, *fed0, *layer0, 0, 0, 0, 0, 0);
  }

  st2 = clock();
  std::cout << "Done storing pixels. Store time of "
            << (((float)st2 - (float)st1) / CLOCKS_PER_SEC)
            << " seconds.\n\nProcessing Pixels...\n";

  // process stored data
  pStore.process();

  // output is stored in a string to print both to a file and terminal
  std::string output;
  output = "Total duplicate pixels: " + std::to_string(duplicates) +
           "\nTotal events: " + std::to_string(pStore.totalEvents) +
           "\nTotal events with zero hits: " + std::to_string(pStore.zeroEvents) +
           "\nTotal hits: " + std::to_string(pStore.totalHits) +
           "\nTotal FEDs: " + std::to_string(pStore.totalFEDs) +
           "\n\nHighest Avg Hit FED Id: " + std::to_string(pStore.haFEDID) +
           "\nWith an avg hit count of: " + std::to_string(pStore.haFEDhit);

  outputFile << output;  // print to file

  et1 = clock();
  std::cout << "\n\nEncoding binary files...\n";
  pStore.encode(pStore.haFEDID);
  et2 = clock();
  std::cout << "Done encoding with an encoding time of "
            << (((float)et2 - (float)et1) / CLOCKS_PER_SEC)
            << " seconds.\n\nGenerating histograms.\n";
  pStore.graph();
  std::cout << "\nDone generating histograms.\n\n";
  file->Close();

  std::cout << output;   // print to terminal

  // output process time in seconds
  t2 = clock();
  float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
  std::cout << "\n\nProgram finish with a runtime of " << seconds
            << " seconds.\n\n";

  return 1;
}
