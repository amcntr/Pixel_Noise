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
// 	for hits per roc 			for pixel addresses
// 	SRAMhit#.bin 				SRAMpix#.bin
// 	 32 bit blocks				  32 bit blocks
//   	  0x[roc1][roc2]...				  0x[25:16][13:8][7:0]
// 	  layer 1:					  0x[row][col][adc]
//	    2 rocs per 32bits
//	  layer 2:
//	    4 rocs per 32bits
//	  layer 3-4 and FPix
//      8 rocs per 32bits

#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>

// Container types
// row, col as key, adc as value
//    row, col is bit shifted
//	  row is shifted 16, col is shifted 8
using Pixels = std::unordered_map<uint32_t, uint32_t>;
// roc id as key and row map as value
using ROCs = std::unordered_map<int, Pixels>;
// channel id as key and roc map
using Chans = std::unordered_map<int, ROCs>;
// layer number as key, chans map as value
//     0 is unknown, 1-4 is BPix, 5+ is FPix
using Layers = std::unordered_map<int, Chans>;
// fed id as key and channel map
using FEDs = std::unordered_map<int, Layers>;
// event id as key and fed map for list of feds
using Events = std::unordered_map<int, FEDs>;

class Pixel_Store {
  // Multiple Pixel Storage Class
  // stores pixels in a nested map structure
 private:
 	// map of total hits per fed
 	std::unordered_map<int, int> hitspFED_;
 public:
 	// ch, roc of roc with highest hits
 	std::pair<int, int> hhROCID;
 	// number of hits in above roc 
 	int hhROChit;
 	// highest avg hit per event fed id
 	int haFEDID;
  // avg number of hits in above fed
 	int haFEDhit;
  // total count of stored items
 	int totalHits;
 	int totalEvents;
 	int totalFEDs;
  // main storage
  Events storage;
 public:
  Pixel_Store() {}           // constructor
  virtual ~Pixel_Store() {}  // destructor

  // adds a pixel to class
  int add(int event,
          int fed,
          int layer,
          int ch,
          int roc,
          int row,
          int col,
          int adc);
  // checks if pixel is already stored
  bool check(int event,
  					 int fed,
  					 int layer, 
  					 int ch, 
  					 int roc, 
  					 uint32_t rowcol);

  // process data
  // gets highest hit roc and highest avg hit fed
  // populates histograms in future
  // returns number for error checking
  void process();
};

// checks if pixel is already stored and adds to container if not
int Pixel_Store::add(int event,
                     int fed,
                     int layer,
                     int ch,
                     int roc,
                     int row,
                     int col,
                     int adc) {
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
}

// checks if pixel is stored in container
bool Pixel_Store::check(int event,
                        int fed,
                        int layer,
                        int chan,
                        int roc,
                        uint32_t rowcol) {
  // this might not work
  Pixels::iterator pix = 
  		storage[event][fed][layer][chan][roc].find(rowcol);

  if (pix == storage[event][fed][layer][chan][roc].end())
    return false;

  return true;
}

void Pixel_Store::process() {
	hhROChit = 0;
	haFEDhit = 0;
	totalHits = 0;
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
          for (auto const& ch : lay.second) {
            for (auto const& roc : ch.second) {
              if (roc.second.size() > hhROChit) {
                hhROCID.first = ch.first;
                hhROCID.second = roc.first;
                hhROChit = roc.second.size();
              }
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
void encoder(int targetFED, Events& events) {
  std::string filename;
  std::ofstream glibhit[3];
  std::ofstream glibpix[3];

  // These are buffers for writing the data to files
  // 1 for each block of the hit files. 4 blocks per file.
  std::vector<uint32_t> RocHits[12];
  // For the header file of the SRAMhit files
  // indicates the binary format used
  // 2 bits per block
  // 0: 2 rocs, 32bit
  // 1: 4 rocs, 32bit
  // 2: 8 rocs, 32bit
  // 3: 8 rocs, 64bit
  uint32_t BlockType[12];
  // buffer for pixel address binary
  std::vector<uint32_t> PixAdd[3];

  bool rocHitHigh;
  // convert data in map structure to binary format
  // and place in a buffer for file writing.
  for (auto const& evt : events) {
    for (auto const& fed : evt.second) {
      // check for target fed
      if (fed.first == targetFED) {
        for (auto const& lay : fed.second) {
          for (auto const& ch : lay.second) {
          	// for storing hits per roc
          	std::unordered_map<int, uint32_t> hits;
          	// reset high hits on roc checker
          	rocHitHigh = false;
            // loop through pixels on roc
            for (auto const& roc : ch.second) {
              // get rocid and hits on roc
              hits[roc.first] = (uint32_t)(roc.second.size());
              // if hits in roc over 4 bits
              if ( roc.second.size() > 15 )
              	rocHitHigh = true;
              // convert pixel addresses
              for (auto const& pix : roc.second) {
                uint32_t addressBuffer = 0;
                addressBuffer = (pix.first | pix.second);
                if (ch.first < 17)
                  PixAdd[0].push_back(addressBuffer);
                if ((ch.first < 33) && (ch.first > 16))
                  PixAdd[1].push_back(addressBuffer);
                if (ch.first > 32)
                  PixAdd[2].push_back(addressBuffer);
              }
            }
            // use rocid and hits on roc for conversion
            uint32_t hitBuffer1 = 0;
            uint32_t hitBuffer2 = 0;
            // index for pushing hit binary into hit buffer
            int index = (int)ceil((float)ch.first/4.0) - 1;
            // diiferent layers have differenct # of rocs
            switch (lay.first) {
              case 1: // layer 1
                for (int rc = 1; rc < 3; rc++) {
                  if (hits.count(rc) > 0)
                    hitBuffer1 = (hitBuffer1 << 16 | hits[rc]);
                  else
                    hitBuffer1 <<= 16;
                }
                RocHits[index].push_back(hitBuffer1);
                break;
              case 2: // layer 2
                for (int rc = 1; rc < 5; rc++) {
                  if (hits.count(rc) > 0)
                    hitBuffer1 = (hitBuffer1 << 8 | hits[rc]);
                  else
                    hitBuffer1 <<= 8;
                }
                if (BlockType[index] < 1)
                	BlockType[index] = 1;
                RocHits[index].push_back(hitBuffer1);
                break;
              default: // layer 3-4 and fpix
              	if (rocHitHigh) {
              		for (int rc = 1; rc < 5; rc++) {
              	    if (hits.count(rc) > 0)
                	    hitBuffer1 = (hitBuffer1 << 8 | hits[rc]);
                  	else
                    	hitBuffer1 <<= 8;
                	}
                	RocHits[index].push_back(hitBuffer1);
                	for (int rc = 5; rc < 9; rc++) {
              	    if (hits.count(rc) > 0)
                	    hitBuffer2 = (hitBuffer2 << 8 | hits[rc]);
                  	else
                    	hitBuffer2 <<= 8;
                	}
                	RocHits[index].push_back(hitBuffer2);
                	if (BlockType[index] < 3)
                		BlockType[index] = 3;
              	} else {
	                for (int rc = 1; rc < 9; rc++) {
	                  if (hits.count(rc) > 0)
	                    hitBuffer1 = (hitBuffer1 << 4 | hits[rc]);
	                  else
	                    hitBuffer1 <<= 4;
	                }
	                if (BlockType[index] < 2)
                		BlockType[index] = 2;
	                RocHits[index].push_back(hitBuffer1);
	              }
                break;
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
  for (int i = 0; i < 3; i++) {
    filename = "SRAMhit" + std::to_string(i) + ".bin";
    glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    filename = "SRAMpix" + std::to_string(i) + ".bin";
    glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    int count = 0;

    // write the SRAMhit files
    // The SRAMhit files are divided into 4 blocks
    uint32_t header = 0;
    for (int j = 0; j < 4; j++) {
    	int index = j + (i * 4);
      header = (header << 2 | BlockType[index]);
    }
    glibhit[i].write((char*)&header, 1);
    // write the SRAMhit binary data
    for (int j = 0; j < 4; j++) {
      count = 0;
      int index = j + (i * 4);
      for (int k = 0; k < 524288; k++) {
        if (count >= RocHits[index].size())
          count = 0;
        glibhit[i].write((char*)&RocHits[index][count], 4);
        count++;
      }
    }
    // write the SRAMpix files
    count = 0;
    for (int j = 0; j < 2097152; j++) {
      if (count >= PixAdd[i].size())
        count = 0;
      glibpix[i].write((char*)&PixAdd[i][count], 4);
      count++;
    }

    glibhit[i].close();
    glibpix[i].close();
  }
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
  if ( !(file->IsOpen()) ) {
    std::cout << "Couln't open file.\n";
    return 1;
  }

  t1 = clock();
  std::cout << "Program start.\n";
	
	// decode data from TTree
  TTreeReader reader("HighFedData", file);
  TTreeReaderValue<int> event(reader, "Data._eventID");
  TTreeReaderValue<int> fed(reader, "Data._fedID");
  TTreeReaderValue<int> layer(reader, "Data._layer");
  TTreeReaderValue<int> chan(reader, "Data._channel");
  TTreeReaderValue<int> roc(reader, "Data._ROC");
  TTreeReaderValue<int> row(reader, "Data._row");
  TTreeReaderValue<int> col(reader, "Data._col");
  TTreeReaderValue<int> adc(reader, "Data._adc");      

  // declare output file to store output information
  std::ofstream outputFile;
  outputFile.open("output.txt");

  Pixel_Store pStore;

  st1 = clock();
  std::cout << "\nStoring pixels...\n";
  // stores duplicate pixel amount
  int duplicates = 0;
  // loop through TTree and store data in Pixel_Store
  while (reader.Next()) {
    duplicates += pStore.add(*event, *fed, *layer, *chan, *roc, *row, *col, *adc);
  }

  st2 = clock();
  std::cout << "Done storing pixels. Store time of "
            << (((float)st2 - (float)st1) / CLOCKS_PER_SEC)
            << " seconds.\n\nProcessing Pixels...\n\n";

  // process stored data
  pStore.process();

  // output is stored in a string to print both to a file and terminal
  std::string output;
  output = "Total duplicate pixels: " + std::to_string(duplicates) +
           "\nTotal events: " + std::to_string(pStore.totalEvents) +
           "\nTotal hits: " + std::to_string(pStore.totalHits) +
           "\nTotal FEDs: " + std::to_string(pStore.totalFEDs) +
           "\n\nHighest Avg Hit FED Id: " + std::to_string(pStore.haFEDID) +
           "\nWith an avg hit count of: " + std::to_string(pStore.haFEDhit) +
					 "\n\nRoc with highest hits for single event in FED: ch " +
           std::to_string(pStore.hhROCID.first) + " roc " + std::to_string(pStore.hhROCID.second) +
           "\nWith a hit count of: " + std::to_string(pStore.hhROChit);

  std::cout << output;   // print to terminal
  outputFile << output;  // print to file

  et1 = clock();
  std::cout << "\n\nEncoding binary files...\n";
  encoder(pStore.haFEDID, pStore.storage);
  et2 = clock();
  std::cout << "Done encoding with an encoding time of "
            << (((float)et2 - (float)et1) / CLOCKS_PER_SEC) << " seconds.";

  file->close();

  // output process time in seconds
  t2 = clock();
  float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
  std::cout << "\n\nDone!\nProgram runtime: " << seconds << " seconds.\n\n";

  return 1;
}
