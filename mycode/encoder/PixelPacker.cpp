// PixelPacker
// Anthony McIntyre - June 2018
// Using pixel data stored in a root TTree;
// finds the fed id with the highest avg hits,
// encodes pixel addresses into a binary format
// for FED testing.
//
// Format:
// 	for hits per roc 			for pixel addresses
// 	SRAMhit#.bin 				SRAMpix#.bin
// 	 32 bit blocks				  32 bit blocks
//   0x[roc1][roc2]				  0x[25:16][13:8][7:0]
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
#include <bitset>
#include <string>
#include <deque>

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
	Pixel_Store() { }//errorFile.open("warnings.txt"); } // constructor
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
		/* errorFile<<"Duplicate Pixel. Stored adc of "<<
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
	else FEDCt = storage[EventID].size();
	
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
	
	return ChCt;
	}
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
	else ROCCt = storage[EventID][FEDID][layer][ChanID].size();

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
	else PixCt = storage[EventID][FEDID][layer][ChanID][ROCID].size();
	
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


bool checkData(std::string filename, std::deque<int> buffer) {
// Checks data in file with filename with associated buffer
// return true if everything matches
	bool match = true;
	std::ifstream f(filename.c_str(), std::ios::binary | std::ios::in);
	for(int i = 0; i < (buffer.size()); i++) {
		uint32_t block;
		f.read(reinterpret_cast<char *>(&block), 4);
		if (block != buffer.front()) {
			match = false;
			buffer.clear();
			break;
		}
		else
			buffer.pop_front();
	}
	f.close();
	return match;
}

void encoder ( int targetFED, Events &events ) {
// outputs 6 binary files of "hits per roc" and pixel addresses
// for the targeted fed
	std::string filename;
	std::ofstream glibhit[3];
	std::ofstream glibpix[3];
	
	// open/create the files
	for (int i = 0; i < 3; i++){ 
		filename = "SRAMhit" + std::to_string(i) + ".bin";
		glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
		filename = "SRAMpix" + std::to_string(i) + ".bin";
		glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);
	}

	std::deque<int> buffers[6]; // for checking file after writing
	uint32_t hits = 0;
	uint32_t hitBuffer = 0;
	uint32_t address = 0;
	
	for (auto const evt : events){
		for (auto const fed : evt.second) {
			if (fed.first == targetFED) {
				for (auto const lay : fed.second) {
					for (auto const ch : lay.second) {
						for (auto const roc : ch.second) {
							// get hit count for roc
							hits = (uint32_t)(roc.second.size());
							
							switch(lay.first) { // switch accross layers
							case 1:
								if (hits > 0)
									hitBuffer = (hitBuffer << 16 | hits);
								else
									hitBuffer << 16;
								break;
							case 2:
								if (hits > 0)
									hitBuffer = (hitBuffer << 8 | hits);
								else
									hitBuffer << 8;
								break;
							default:
								if (hits > 0)
									hitBuffer = (hitBuffer << 4 | hits);
								else
									hitBuffer << 4;
								break;
							}
							// get pixel addresses for hit pixels
							// and write to pixel files
							for (auto const pix : roc.second) {
								address = 0;
								address = (pix.first | pix.second);
								if (ch.first < 17) {
									glibpix[0].write((char*)&address, 4);
									buffers[3].push_back(address);
								}
								if ((ch.first <33) && (ch.first > 16)) {
									glibpix[1].write((char*)&address, 4);
									buffers[4].push_back(address);
								}
								if (ch.first > 32) {
									glibpix[2].write((char*)&address, 4);
									buffers[5].push_back(address);
								}
							}
						}
						// write hits to hit files
						if (ch.first < 17) {
							glibhit[0].write((char*)&hitBuffer, 4);
							buffers[0].push_back(hitBuffer);
							hitBuffer = 0;
						}
						if ((ch.first > 16) && (ch.first < 33)) {
							glibhit[1].write((char*)&hitBuffer, 4);
							buffers[1].push_back(hitBuffer);
							hitBuffer = 0;
						}
						if (ch.first > 32) {
							glibhit[2].write((char*)&hitBuffer, 4);
							buffers[2].push_back(hitBuffer);
							hitBuffer = 0;
						}
					}
				}
			}
		}
	}
	// close all files
	for (int i = 0; i < 3; i++){
		glibhit[i].close();
		glibpix[i].close();
	}

	// check file contents with buffer
	for (int i = 0; i < 3; i++) {
		filename = "SRAMhit" + std::to_string(i) + ".bin";
		std::cout<<"\n\tChecking data in "<<filename<<'\n';
		if(checkData(filename, buffers[i]))
			std::cout<<"\t\tData match.\n";
		else
			std::cout<<"\t\tWarning data mismatch.\n";

		filename = "SRAMpix" + std::to_string(i) + ".bin";
		std::cout<<"\n\tChecking data in "<<filename<<'\n';
		if(checkData(filename, buffers[i+3]))
			std::cout<<"\t\tData match.\n";
		else
			std::cout<<"\t\tWarning data mismatch.\n";
	}
}

void makeHistograms ( Pixel_Store &pStore ) {
// creates a series of histograms to analyze the data
	TFile *f = new TFile("analysis.root", "recreate");
	TDirectory *hits = f->mkdir("hits");

	std::string label;
	std::string title;

	int hAvgFED = pStore.Get_FEDhighest();
	int hAvgEvent = pStore.Get_EventWithHighestHitsFED(hAvgFED);
	int chCount = pStore.Get_ChanCount(false, hAvgEvent, hAvgFED);

	TH2D *hFEDChannel[139], *hitmaps[chCount];

	hits->cd();

	for (int i = 0; i < 139; i++) {	
		title = "Hits in FED #" + std::to_string(i+1200) + " in Each Channel";
		label = "hFED" + std::to_string(i+1200);
		hFEDChannel[i] = new TH2D(label.c_str(), title.c_str(), 48, 1., 49., 600, -0.5, 599.5);
		hFEDChannel[i]->GetXaxis()->SetTitle("Channel Number");
		hFEDChannel[i]->GetYaxis()->SetTitle("Number of Hits");
		hFEDChannel[i]->SetOption("COLZ");
	}
	for (auto const &event : pStore.storage) {
		for (auto const &fed : event.second) {
			for (auto const &lay : fed.second) {
				for (auto const &ch : lay.second) {
					for (auto const &roc : ch.second) {
						hFEDChannel[fed.first - 1200]->Fill(ch.first, roc.second.size());
					}
				}
			}
		}
	}

	TDirectory *hmapsf = f->mkdir("hitmaps");
	hmapsf->cd();
	int index = -1;
	uint32_t row, col;
	for (auto const &lay : pStore.storage[hAvgEvent][hAvgFED]){
		for (auto const &chan : lay.second) {
			index++;
			label = "E#" + std::to_string(hAvgEvent)
				+ "F#" + std::to_string(hAvgFED)
				+ "Ch#" + std::to_string(chan.first);
			
			switch(lay.first) { 
				case 1:
					title = "Hitmap of Ch#" + std::to_string(chan.first)
						+ " Layer 1 FED#" + std::to_string(hAvgFED)
						+ " Event#" + std::to_string(hAvgEvent);

					hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 104, 0., 104., 80, 0., 80.);
				case 2:
					title = "Hitmap of Ch#" + std::to_string(chan.first)
						+ " Layer 2 FED#" + std::to_string(hAvgFED)
						+ " Event#" + std::to_string(hAvgEvent);

					hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 208, 0., 208., 80, 0., 80.);

				case 3:
					title = "Hitmap of Ch#" + std::to_string(chan.first)
						+ " Layer 3 FED#" + std::to_string(hAvgFED)
						+ " Event#" + std::to_string(hAvgEvent);

					hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 416, 0., 416., 80, 0., 80.);

				case 4:
					title = "Hitmap of Ch#" + std::to_string(chan.first)
						+ " Layer 4 FED#" + std::to_string(hAvgFED)
						+ " Event#" + std::to_string(hAvgEvent);

					hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 416, 0., 416., 80, 0., 80.);

				default:
					if (lay.first > 4) {	
						title = "Hitmap of Ch#" + std::to_string(chan.first)
							+ " FPix FED#" + std::to_string(hAvgFED)
							+ " Event#" + std::to_string(hAvgEvent);
					}
					else {
						title = "Hitmap of Ch#" + std::to_string(chan.first)
							+ " (Unknown Layer) FED#" + std::to_string(hAvgFED)
							+ " Event#" + std::to_string(hAvgEvent);
					}
					
					hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 416, 0., 416., 80, 0., 80.);
			}

			hitmaps[index]->GetXaxis()->SetTitle("column");
			hitmaps[index]->GetYaxis()->SetTitle("row");
			hitmaps[index]->SetOption("COLZ");
			
			for (auto const &roc : chan.second) {			
				for (auto const &pix : roc.second) {
					row = pix.first >> 16;
					col = pix.first >> 8;
					col <<= 26;
					col >>=26;
					hitmaps[chan.first]->Fill((int)col, (int)row, pix.second);
				}
			}
		}
	}
	
	f->Write();
}


void PixelPacker (TFile *f) { 
// the real main function
	
	// clock to record process time	
	clock_t t1, t2; 
	t1 = clock();

	// decode data from TTree
	TTreeReader reader("DecodedData", f);
        TTreeReaderValue<int> event(reader, "Data._eventID");
        TTreeReaderValue<int> fed(reader, "Data._fedID");
        //TTreeReaderValue<int> layer(reader, "Data._layer");
        TTreeReaderValue<int> chan(reader, "Data._channel");
        TTreeReaderValue<int> roc(reader, "Data._ROC");
        TTreeReaderValue<uint32_t> row(reader, "Data._row");
        TTreeReaderValue<uint32_t> col(reader, "Data._col");
        TTreeReaderValue<uint32_t> adc(reader, "Data._adc");
       
	// declare output file to store output information
	std::ofstream outputFile;
	outputFile.open("output.txt");
	 
	Pixel_Store pStore;
	
	std::cout << "\nStoring pixels...\n";
	int duplicates = 0; //stores duplicate pixel amount
	while( reader.Next()) { // loop through TTree and store data in Pixel_Store
		duplicates += pStore.add(*event, *fed, 0, *chan, *roc, *row, *col, *adc);
	}
	std::cout << "Processing Pixels...\n\n";
	
	int eventCt = pStore.Get_EventCount(); // get total events
	int highest_FEDId = pStore.Get_FEDhighest(); // get fedid with highest average hits per event
	int hitCtFED = pStore.Get_FEDhits(highest_FEDId); // get total hits for fed with highest avg hits
	int hitCtTotal = pStore.Get_PixelCount(); // get total hits stored
	int eventHFid = pStore.Get_EventWithHighestHitsFED(highest_FEDId); // get event id for highest amount of hits in highest avg hit fed
	int eventHFhits = pStore.Get_FEDhits(highest_FEDId, false, eventHFid); // hits in said event id in said fed
	
	// output is stored in a string to print both to a file and terminal
	std::string output;
	output = "Total duplicate pixels: " + std::to_string(duplicates) +
		"\nTotal events: " + std::to_string(eventCt) + "\nTotal hits: " + 
		std::to_string(hitCtTotal) + "\n\nHighest Avg Hit FED Id: " + 
		std::to_string(highest_FEDId) + "\nWith a hit count of: " + 
		std::to_string(hitCtFED / eventCt) + "\n\nEvent with highest hits in FED#" +
		std::to_string(highest_FEDId) +  ": "  + std::to_string(eventHFid) +
		"\nWith a hit count of: " + std::to_string(eventHFhits);
	
	std::cout<<output; // print to terminal
	outputFile<<output; // print to file

	std::cout<<"\n\nMaking histograms...\n";
	makeHistograms(pStore);

	std::cout<<"\n\nEncoding binary files...\n";
	encoder(highest_FEDId, pStore.storage);
	std::cout<<"Done encoding.";

	// output process time in seconds
	t2 = clock();
	float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
	std::cout<<"\n\nDone!\nProgram runtime: " << seconds << " seconds.\n\n";
}

int main( int argc, char *argv[] ) {
// takes file argument and checks it before passing to main function	
	if (argc != 2)  // if no arguments
		std::cout<<"usage: "<<argv[0] <<" <filename>\n";
	else {
        	TFile *file = new TFile(argv[1]);
		if ( file->IsOpen() )  // check if file loaded correctly
			PixelPacker(file); // pass file to main function
		else
			std::cout<<"Couln't open file.\n";
	}
	return 1;
}

