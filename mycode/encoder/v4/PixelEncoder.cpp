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

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <string>
#include <vector>

#include <TROOT.h>
#include <TFile.h>
#include <TH1.h>
#include <TF2.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TStyle.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>


// Container types
using Pixels = std::unordered_map<uint32_t, uint32_t>; 	// row, col as key, adc as value
														//    row, col is bit shifted
														//	  row is shifted 16, col is shifted 8
using ROCs = std::unordered_map<int, Pixels>; 	// roc id as key and row map as value
using Chans = std::unordered_map<int, ROCs>; 	// channel id as key and roc map
using Layers = std::unordered_map<int, Chans>; 	// layer number as key, chans map as value
												//     0 is unknown, 1-4 is BPix, 5+ is FPix
using FEDs = std::unordered_map<int, Layers>; 	// fed id as key and channel map
using Events = std::unordered_map<int, FEDs>; 	// event id as key and fed map for list of feds


class Pixel_Store {
// Multiple Pixel Storage Class
// stores pixels in a nested map structure
public:
	Events storage; // main storage
	//std::ofstream errorFile;
public:
	Pixel_Store() { } //errorFile.open("warnings.txt"); } // constructor
	virtual ~Pixel_Store() { } // destructor

	int add(int event, int fed, int layer, int ch, int roc, uint32_t row, uint32_t col, uint32_t adc); // adds a pixel to class
	bool check(int event, int fed, int layer, int ch, int roc, uint32_t rowcol); // checks if pixel is already stored

	// returns number of elements in each level of container
	// if total argument is true, returns total count of element
	// if total argument is false, returns count of element at location in container
	int Get_EventCount() { return storage.size(); }
	int Get_FEDCount(bool total = true, int EventID = 0);
	int Get_ChanCount(bool total = true, int EventID = 0, int FEDID = 0);
	int Get_ROCCount(bool total = true, int EventID = 0, int FEDID = 0, int layer = 0, int ChanID = 0);
	int Get_PixelCount(bool total = true, int EventID = 0, int FEDID = 0, int layer = 0, int ChanID = 0, int ROCID = 0);

	// returns hits in fed.
	// if total = true, then return total hits for all events
	// if total = false, then return hits in fed for event id
	int Get_FEDhits(int FEDID, bool total = true, int eventID = 0); // returns total hits in fed
	std::vector<int> Get_RocWithHighestHits(int FEDID);		
	std::vector<int> Get_RocHits(int FEDID);
	int Get_FEDhighest(); // returns id of fed with highest avg hits
	int Get_EventWithHighestHitsFED(int FEDID); // returns event id with highests hits in fedid.
};


int Pixel_Store::add(int event, int fed, int layer, int ch, int roc, uint32_t row, uint32_t col, uint32_t adc) {
// checks if pixel is already stored and adds to container if not
	uint32_t rowcol = (row << 16 | col << 8); // merge row and col into unique number by bit shifting them
	
	if (!check(event, fed, layer, ch, roc, rowcol)) {

		storage[event][fed][layer][ch][roc][rowcol] = adc;
		
		return 0; // for duplicate counting
	}
	else {
		// adds duplicate pixel information to warnings.txt
		/*errorFile<<"Duplicate Pixel. Stored adc of "<<
			storage[event][fed][ch][roc][rowcol] <<
			". New adc of "<<adc<<".\tAddress: Event#"<<event<<" FED#"<<
			fed<<" Ch#"<<ch<<" ROC#"<<roc<<" Row#"<<row<<" Col#"<<col<<'\n';*/
		return 1; // for duplicate counting
	}
}

bool Pixel_Store::check(int event, int fed, int layer, int chan, int roc, uint32_t rowcol) {
// checks if pixel is stored in container
	Events::iterator ev = storage.find(event);
	if (ev == storage.end()) 
		return false;
	FEDs::iterator fe = ev->second.find(fed);
	if (fe == ev->second.end()) 
		return false;
	Layers::iterator ly = fe->second.find(layer);
	if (ly == fe->second.end())
		return false;
	Chans::iterator ch = ly->second.find(chan);
	if (ch == ly->second.end()) 
		return false;
	ROCs::iterator rc = ch->second.find(roc);
	if (rc == ch->second.end()) 
		return false;
	Pixels::iterator pi = rc->second.find(rowcol);
	if (pi == rc->second.end()) 
		return false;
	return true;
}


int Pixel_Store::Get_FEDCount(bool total, int EventID) {
// returns count of FEDs stored
// if total = true, returns total feds from all events
// if total = false, returns fed count from single event
	int FEDCt = 0;
	if (total) {
	  for (auto const &event : storage) {
	    FEDCt += event.second.size();
	  }
	}
	else
	  FEDCt = storage[EventID].size();
	
	return FEDCt;
}

int Pixel_Store::Get_ChanCount(bool total, int EventID, int FEDID) {
// returns count of channels stored
	int ChCt = 0;
	if (total) {
	  for (auto const &event : storage) {
	    for (auto const &fed : event.second) {
	      for (auto const &lay : fed.second) {
		ChCt += lay.second.size();
	      }
	    }
	  }
	}
	else {
	  for (auto const &lay : storage[EventID][FEDID]) {
	    ChCt += lay.second.size();
	  }
	}
	return ChCt;
}

int Pixel_Store::Get_ROCCount(bool total, int EventID, int FEDID, int layer, int ChanID) {
// returns count of rocs stored
	int ROCCt = 0;
	if (total) {
	  for (auto const &event : storage) {
	    for (auto const &fed : event.second) {
	      for (auto const &lay : fed.second) {	
		for (auto const &ch : lay.second) {
		  ROCCt += ch.second.size();
		}
	      }
	    }
	  }
	}
	else 
	  ROCCt = storage[EventID][FEDID][layer][ChanID].size();

	return ROCCt;
}

int Pixel_Store::Get_PixelCount(bool total, int EventID, int FEDID, int layer, int ChanID, int ROCID) {
// returns count of pixels stored 
	int PixCt = 0;
	// if total true, return count of all pixels
	if (total) {
	  for (auto const &event : storage) {
	    for (auto const &fed : event.second) {
	      for (auto const &ly : fed.second) {
		for (auto const &ch : ly.second) {
		  for (auto const &roc : ch.second) {
		    PixCt += roc.second.size();
		  }
		}
	      }
	    }
	  }
	}
	// if total false then return pixel count inside roc
	else 
	  PixCt = storage[EventID][FEDID][layer][ChanID][ROCID].size();
	
	return PixCt;
}

int Pixel_Store::Get_FEDhits(int FEDID, bool total, int eventID) {
// returns total hits for all events found in a single fed
	int hitNum = 0;
	if (total) {
	  for (auto const &event : storage) {
	    for (auto const &fed : event.second) {
	      if (fed.first == FEDID) {
		for (auto const &lay : fed.second) {
		  for (auto const &chan : lay.second) {
		    for (auto const &roc : chan.second) {
		      hitNum += roc.second.size();
		    }
		  }
		}
	      }
	    }
	  }	
	}
	// returns hits in fed for single event
	else {
	  for (auto const &chan : storage[eventID][FEDID]) {
	    for (auto const &lay : chan.second) {
	      for (auto const &roc : lay.second) {
		hitNum += roc.second.size();
	      }
	    }
	  }
	}
	return hitNum;
}

int Pixel_Store::Get_FEDhighest() {
// return fedid of fed with highest avg hits per event
	int FEDID = 1200;
	int FEDCount = 1338;
	int EventCount = Get_EventCount();
	int hits = 0;
	int avg = 0;
	int highest_avg = 0;
	int highest_fed = 0;
	for (int i = FEDID; i < FEDCount; i++) {
	    hits = Get_FEDhits(i);
	    if (hits > 0) {
	        avg = hits / EventCount;
	        if (avg > highest_avg) {
		    highest_avg = avg;
		    highest_fed = i;
	        }
	    }
	}
	return highest_fed;
}

std::vector<int> Pixel_Store::Get_RocHits(int FEDID) {
	std::vector<int> rochits;
	rochits.push_back(0);
	rochits.push_back(0);
	rochits.push_back(0);
	for (auto const &event : storage) {
		for (auto const &fed: event.second) {
			if (fed.first == FEDID) {
				for (auto const lay : fed.second) {
					for (auto const ch : lay.second) {
						for (auto const roc : ch.second) {
							if (roc.second.size() > rochits[2]) {
								rochits[0] = ch.first;
								rochits[1] = roc.first;
								rochits[2] = roc.second.size();
							}
						}
					}
				}
			}
		}
	}
	return rochits;
}

std::vector<int> Pixel_Store::Get_RocWithHighestHits(int FEDID) {
	std::map<std::pair<int, int>, int> rocHit;
	for (auto const &event : storage) {
		for (auto const &fed : event.second) {
			if (fed.first == FEDID) {
				for (auto const lay : fed.second) {
					for (auto const ch : lay.second) {
						for (auto const roc : ch.second) {
							rocHit[std::make_pair(ch.first, roc.first)] += roc.second.size();
						}
					}
				}
			}
		}
	}
	std::vector<int> rocID;
	rocID.push_back(0);
	rocID.push_back(0);
	rocID.push_back(0);
	for (auto const rc : rocHit) {
		if (rc.second > rocID[2]) {
			rocID[0] = rc.first.first;
			rocID[1] = rc.first.second;
			rocID[2] = rc.second;
		}
	}
	return rocID;
}

int Pixel_Store::Get_EventWithHighestHitsFED(int FEDID) {
// return event id for event with highest hits in fed with fedid
	int eventID;
	int hits = 0;
	int highestHits = 0;
	for (auto const &event : storage) {
		hits = Get_FEDhits(FEDID, false, event.first);
		if (hits > highestHits) {
			highestHits = hits;
			eventID = event.first;
		}
	}
	return eventID;
}

void encoder ( int targetFED, Events &events ) {
// outputs 12 binary files of "hits per roc" and pixel addresses
// for the fed during all events in data file. Half of the files
// are looped to the max file size. The other half are not looped.
	std::string filename;
	std::ofstream glibhit[3];
	std::ofstream glibpix[3];

	std::unordered_map<int, uint32_t> hits;

	// These are buffers for writing the data to files
	std::vector<uint32_t> RocHits[12]; // 1 for each block of the hit files. 4 blocks per file.
	std::vector<uint32_t> PixAdd[3];
	
	std::unordered_map<int, std::unordered_map<int, int>> chPlay;
	
	// convert data in map structure to binary format
	// and place in a buffer for file writing.
	for (auto const evt : events){
	  for (auto const fed : evt.second) {
	    if (fed.first == targetFED) { // check for target fed
	      for (auto const lay : fed.second) {
		for (auto const ch : lay.second) {
		  chPlay[lay.first][ch.first] += 1;
		  for (auto const roc : ch.second) {
		    // get rocid and hits on roc
		    hits[roc.first] = (uint32_t)(roc.second.size()); 
		    // convert pixel addresses
		    for (auto const pix : roc.second) {
			uint32_t addressBuffer = 0;
			addressBuffer = (pix.first | pix.second);
			if (ch.first < 17)
				PixAdd[0].push_back(addressBuffer);
			if ((ch.first <33) && (ch.first > 16))
				PixAdd[1].push_back(addressBuffer);
			if (ch.first > 32)
				PixAdd[2].push_back(addressBuffer);
		    }
		  }
		  //use rocid and hits on roc for conversion
		  uint32_t hitBuffer = 0;
		  // diiferent layers have differenct # of rocs
		  switch(lay.first) { 
		  case 1:	
			for(int rc = 1; rc < 3; rc++) {
			   if (hits.count(rc)>0)
				hitBuffer = (hitBuffer << 16 | hits[rc]);
			   else
				hitBuffer <<= 16;
			}
			break;
		  case 2:
			for(int rc = 1; rc < 5; rc++) {
			   if (hits.count(rc)>0)
				hitBuffer = (hitBuffer << 8 | hits[rc]);
			   else
				hitBuffer <<= 8;
			}
			break;
		  default:
			for(int rc = 1; rc < 9; rc++) {
			   if (hits.count(rc)>0)
				hitBuffer = (hitBuffer << 4 | hits[rc]);
			   else
				hitBuffer <<= 4;
			}
			break;
		  }
		  hits.clear();
		  // push hit buffer onto correct block of binary file.
		  if (ch.first < 5)
			RocHits[0].push_back(hitBuffer);
		  if ((ch.first > 4) && (ch.first < 9))
			RocHits[1].push_back(hitBuffer);
		  if ((ch.first > 8) && (ch.first < 13))
			RocHits[2].push_back(hitBuffer);
		  if ((ch.first > 12) && (ch.first < 17))
			RocHits[3].push_back(hitBuffer);
		  if ((ch.first > 16) && (ch.first < 21))
			RocHits[4].push_back(hitBuffer);
		  if ((ch.first > 20) && (ch.first < 25))
			RocHits[5].push_back(hitBuffer);
		  if ((ch.first > 24) && (ch.first < 29))
			RocHits[6].push_back(hitBuffer);
		  if ((ch.first > 28) && (ch.first < 33))
			RocHits[7].push_back(hitBuffer);
		  if ((ch.first > 32) && (ch.first < 37))
			RocHits[8].push_back(hitBuffer);
		  if ((ch.first > 36) && (ch.first < 41)) 
			RocHits[9].push_back(hitBuffer);
		  if ((ch.first > 40) && (ch.first < 45))
			RocHits[10].push_back(hitBuffer);
		  if (ch.first > 44)
			RocHits[11].push_back(hitBuffer);
		}
	      }
	    }
	  }
	}

	// begin writing the files
		
	std::ofstream chDistro("Channels_per_Layer.txt");
	std::string output;
	for (auto const ly : chPlay){
		output = "Layer " + std::to_string(ly.first) + ": Chs:";
		for (auto const c : ly.second){
			output += ' ' + std::to_string(c.first);
		}
		output += '\n';
		chDistro << output.c_str();
	}	
	chDistro.close();
	
	// These files have to be an exact file size. 
	// So it loops over the data until the file size is met.
	// The size in this case is 2^21 32bit blocks or around 8.39 MB
	for (int i = 0; i < 3; i++){ 
		filename = "SRAMhit" + std::to_string(i) + ".bin";
		glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
		filename = "SRAMpix" + std::to_string(i) + ".bin";
		glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);
		int count = 0;
		
		// write the SRAMhit files	
		// The SRAMhit files are divided into 4 blocks
		for (int j = 0; j < 4; j++) {
			count = 0;
			for (int k = 0; k < 524288; k++) { // big loop, it goes fast
		    		if (count >= RocHits[j + (i*4)].size())
					count = 0;
		    		glibhit[i].write((char*)&RocHits[j + (i*4)][count], 4);
		    		count++;
		  	}
		}
		// write the SRAMpix files
		count = 0;
		for (int j = 0; j < 2097152; j++) { // bigger loop, still fast
			if (count >= PixAdd[i].size())
				count = 0;
			glibpix[i].write((char*)&PixAdd[i][count], 4);
			count++;
		}

		glibhit[i].close();
		glibpix[i].close();
	}
	
	// this loop is for the non looped files
	// these files are only constrained to a max file size of around 8.4 MB
	for (int i = 0; i < 3; i++){ 
		filename = "SRAMhit" + std::to_string(i) + "_noloop.bin";
		glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
		filename = "SRAMpix" + std::to_string(i) + "_noloop.bin";
		glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);
		
		// SRAMhit files
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < RocHits[j + (i*4)].size(); k++) {
				glibhit[i].write((char*)&RocHits[j + (i*4)][k], 4);
			}
		}
		// SRAMpix files
		for (int j = 0; j < PixAdd[i].size(); j++) {
			glibpix[i].write((char*)&PixAdd[i][j], 4);
		}

		glibhit[i].close();
		glibpix[i].close();
	}
}

void PixelPacker (TFile *f) { 
// the real main function
	
	// clock to record process time	
	clock_t t1, t2, st1, st2, et1, et2; 
	t1 = clock();

	// decode data from TTree
	TTreeReader reader("HighFedData", f);
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
	int duplicates = 0; //stores duplicate pixel amount
	while( reader.Next()) { // loop through TTree and store data in Pixel_Store
		duplicates += pStore.add(*event, *fed, *layer, *chan, *roc, (uint32_t)*row, (uint32_t)*col, (uint32_t)*adc);
	}
	st2 = clock();
	std::cout << "Done storing pixels. Store time of " << (((float)st2 - (float)st1) / CLOCKS_PER_SEC)
		<<" seconds.\nProcessing Pixels...\n\n";
	
	int eventCt = pStore.Get_EventCount(); // get total events
	int highest_FEDId = pStore.Get_FEDhighest(); // get fedid with highest average hits per event
	int hitCtFED = pStore.Get_FEDhits(highest_FEDId); // get total hits for fed with highest avg hits
	int hitCtTotal = pStore.Get_PixelCount(); // get total hits stored
	std::vector<int> rochits = pStore.Get_RocWithHighestHits(highest_FEDId);
	// output is stored in a string to print both to a file and terminal
	std::string output;
	output = "Total duplicate pixels: " + std::to_string(duplicates) +
		"\nTotal events: " + std::to_string(eventCt) + "\nTotal hits: " + 
		std::to_string(hitCtTotal) + "\n\nHighest Avg Hit FED Id: " + 
		std::to_string(highest_FEDId) + "\nWith an avg hit count of: " + 
		std::to_string(hitCtFED / eventCt) + "\n\nRoc with highest total hits in FED is: ch " +
		std::to_string(rochits[0]) + " roc " + std::to_string(rochits[1]) + 
		"\nWith a total hit count of: " + std::to_string(rochits[2]);
	rochits = pStore.Get_RocHits(highest_FEDId);
	output += "\n\nRoc with highest hits for single event in FED: ch " +
		std::to_string(rochits[0]) + " roc " + std::to_string(rochits[1]) +
		"\nWith a hit count of: " + std::to_string(rochits[2]);


	std::cout<<output; // print to terminal
	outputFile<<output; // print to file

	et1 = clock();
	std::cout<<"\n\nEncoding binary files...\n";
	encoder(highest_FEDId, pStore.storage);
	et2 = clock();
	std::cout<<"Done encoding with an encoding time of "
		<< (((float)et2 - (float)et1) / CLOCKS_PER_SEC) <<" seconds.";

	// output process time in seconds
	t2 = clock();
	float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
	std::cout<<"\n\nDone!\nProgram runtime: " << seconds << " seconds.\n\n";
}

int main( int argc, char *argv[] ) {
// takes file argument and checks it before passing to main function	
	if (argc != 2) { // if no arguments
		std::cout<<"usage: "<<argv[0] <<" <filename>\n";
	}
	else {
        	TFile *file = new TFile(argv[1]);
		if ( file->IsOpen() ) { // check if file loaded correctly
			std::cout<<"Program start.\n";
			PixelPacker(file); // pass file to main function
		}
		else std::cout<<"Couln't open file.\n";
	}
	return 1;
}

