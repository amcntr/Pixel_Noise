// PixelPacker
// Using pixel data stored in a root TTree;
// finds the fed id with the highest avg hits,
// encodes pixel addresses into a binary format
// Anthony McIntyre - June 2018

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <time.h>

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
using Pixels = std::map<std::pair<int, int>, int>; // row, col pair as key, adc as value
using ROCs = std::unordered_map<int, Pixels>; // for storing rocs in a channel, roc id as key and row map as value
using Chans = std::unordered_map<int, ROCs>; // for storing channels in a fed, channel id as key and roc map
using FEDs = std::unordered_map<int, Chans>; // for storing feds in an event,  fed id as key and channel map
using Events = std::unordered_map<int, FEDs>; // for storing events, includes an event id as key and fed map for list of feds


class Pixel_Store {
// Multiple Pixel Storage Class
// stores pixels in a nested map structure
public:
	Events storage; // main storage
	std::ofstream errorFile;
public:
	Pixel_Store() { errorFile.open("errors.txt"); } // constructor
	virtual ~Pixel_Store() { } // destructor

	int add(int event, int fed, int ch, int roc, int row, int col, int adc); // adds a pixel to class
	bool check(int event, int fed, int ch, int roc, int row, int col); // checks if pixel is already stored

	// returns contents of container at specified level
	Pixels Get_Pixels(int event, int fed, int ch, int roc) { return storage[event][fed][ch][roc]; }
	ROCs Get_ROCs(int event, int fed, int ch) { return storage[event][fed][ch]; }
	Chans Get_Chans(int event, int fed) { return storage[event][fed]; }
	FEDs Get_FEDs(int event) { return storage[event]; }
	
	// returns number of elements in each level of container
	// if total argument is true, returns total count of element
	// if total argument is false, returns count of element at location in container
	int Get_EventCount() { return storage.size(); }
	int Get_FEDCount(bool total = true, int EventID = 0);
	int Get_ChanCount(bool total = true, int EventID = 0, int FEDID = 0);
	int Get_ROCCount(bool total = true, int EventID = 0, int FEDID = 0, int ChanID = 0);
	int Get_PixelCount(bool total = true, int EventID = 0, int FEDID = 0, int ChanID = 0, int ROCID = 0);

	// returns hits in fed.
	// if total = true, then return total hits for all events
	// if total = false, then return hits in fed for event id
	int Get_FEDhits(int FEDID, bool total = true, int eventID = 0); // returns total hits in fed
		
	int Get_FEDhighest(); // returns id of fed with highest avg hits
	int Get_EventWithHighestHitsFED(int FEDID); // returns event id with highests hits in fedid.
};


int Pixel_Store::add(int event, int fed, int ch, int roc, int row, int col, int adc) {
// checks if pixel is already stored and adds to container if not
	if (!check(event, fed, ch, roc, row, col)) {
		storage[event][fed][ch][roc][std::make_pair(row, col)] = adc;
		
		return 0; // for duplicate counting
	}
	else {
		 errorFile<<"Warning: Duplicate Pixel. Stored adc of "<<
			storage[event][fed][ch][roc][std::make_pair(row ,col)] <<
			". New adc of "<<adc<<".\tAddress: Event#"<<event<<" FED#"<<
			fed<<" Ch#"<<ch<<" ROC#"<<roc<<" Row#"<<row<<" Col#"<<col<<'\n';
		return 1; // for duplicate counting
	}
}

bool Pixel_Store::check(int event, int fed, int chan, int roc, int row, int col) {
// checks if pixel is stored in container
	Events::iterator ev = storage.find(event);
	if (ev == storage.end()) 
		return false;
	FEDs::iterator fe = ev->second.find(fed);
	if (fe == ev->second.end()) 
		return false;
	Chans::iterator ch = fe->second.find(chan);
	if (ch == fe->second.end()) 
		return false;
	ROCs::iterator rc = ch->second.find(roc);
	if (rc == ch->second.end()) 
		return false;
	Pixels::iterator pi = rc->second.find(std::make_pair(row,col));
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
				ChCt += fed.second.size();
			}
		}
	}
	else ChCt = storage[EventID][FEDID].size();
	
	return ChCt;
}

int Pixel_Store::Get_ROCCount(bool total, int EventID, int FEDID, int ChanID) {
// returns count of rocs stored
	int ROCCt = 0;
	if (total) {
		for (auto const &event : storage) {
			for (auto const &fed : event.second) {
				for (auto const &ch : fed.second) {
					ROCCt += ch.second.size();
				}
			}
		}
	}
	else ROCCt = storage[EventID][FEDID][ChanID].size();
	
	return ROCCt;
}

int Pixel_Store::Get_PixelCount(bool total, int EventID, int FEDID, int ChanID, int ROCID) {
// returns count of pixels stored 
	int PixCt = 0;
	if (total) {
		for (auto const &event : storage) {
			for (auto const &fed : event.second) {
				for (auto const &ch : fed.second) {
					for (auto const &roc : ch.second) {
							PixCt += roc.second.size();
					}
				}
			}
		}
	}
	else PixCt = storage[EventID][FEDID][ChanID][ROCID].size();
	
	return PixCt;
}

int Pixel_Store::Get_FEDhits(int FEDID, bool total, int eventID) {
// returns total hits for all events found in a single fed
	int hitNum = 0;
	if (total) {
		for (auto const &event : storage) {
			for (auto const &fed : event.second) {
				if (fed.first == FEDID) {
					for (auto const &chan : fed.second) {
						for (auto const &roc : chan.second) {
							hitNum += roc.second.size();
						}
					}
				}
			}
		}	
	}
	else {
		for (auto const &chan : storage[eventID][FEDID]) {
			for (auto const &roc : chan.second) {
					hitNum = roc.second.size();
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
		//std::cout << "Total hits in FED#" << i <<": "<< hits;
		if (hits > 0) {
			avg = hits / EventCount;
			//std::cout<<" Avg of: "<< avg << '\n';
			if (avg > highest_avg) {
				highest_avg = avg;
				highest_fed = i;
			}
		}
		//else std::cout<<'\n';
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


void makeHistograms ( Pixel_Store &pStore ) {
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
			for (auto const &ch : fed.second) {
				for (auto const &roc : ch.second) {
					hFEDChannel[fed.first - 1200]->Fill(ch.first, roc.second.size());
					std::cout<<"working\n";
				}
			}
		}
	}

//Waiting on code to give out layers
/*
	TDirectory *hmapsf = f->mkdir("hitmaps");
	hmapsf->cd();
	int index = 0;
	for (auto const &chan : pStore.storage[hAvgEvent][hAvgFED]){
		label = "E#" + std::to_string(hAvgEvent)
			+ "F#" + std::to_string(hAvgFED)
			+ "Ch#" + std::to_string(chan.first);
		
		if (chan.second.size() == 2) {
			title = "Hitmap of Ch#" + std::to_string(chan.first)
				+ " Layer 1 FED#" + std::to_string(hAvgFED)
				+ " Event#" + std::to_string(hAvgEvent);

			hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 104, 0., 104., 80, 0., 80.);
		}

		if (chan.second.size() == 4) {
			title = "Hitmap of Ch#" + std::to_string(chan.first)
				+ " Layer 2 FED#" + std::to_string(hAvgFED)
				+ " Event#" + std::to_string(hAvgEvent);

			hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 208, 0., 208., 80, 0., 80.);
		}
		if (chan.second.size() == 8) {
			title = "Hitmap of Ch#" + std::to_string(chan.first)
				+ " Layer (3-4,FPix) FED#" + std::to_string(hAvgFED)
				+ " Event#" + std::to_string(hAvgEvent);

			hitmaps[index] = new TH2D(label.c_str(), title.c_str(), 416, 0., 416., 80, 0., 80.);
		}
		hitmaps[index]->GetXaxis()->SetTitle("column");
		hitmaps[index]->GetYaxis()->SetTitle("row");
		hitmaps[index]->SetOption("COLZ");
		
		for (auto const &roc : chan.second) {			
			for (auto const &pix : roc.second) {
				hitmaps[chan.first]->Fill(pix.first.second, pix.first.first, pix.second);
			}
		}
		index++;
	} */
	f->Write();
}


void PixelPacker (TFile *f) { 
// the real main function

	// decode data from TTree
	TTreeReader reader("DecodedData", f);
        TTreeReaderValue<int> event(reader, "Data._eventID");
        TTreeReaderValue<int> fed(reader, "Data._fedID");
        TTreeReaderValue<int> chan(reader, "Data._channel");
        TTreeReaderValue<int> roc(reader, "Data._ROC");
        TTreeReaderValue<int> row(reader, "Data._row");
        TTreeReaderValue<int> col(reader, "Data._col");
        TTreeReaderValue<int> adc(reader, "Data._adc");
       
	// declare output file to store output information
	std::ofstream outputFile;
	outputFile.open("output.txt");
	 
	Pixel_Store pStore;
	
	// clock to record process time	
	clock_t t1, t2; 
	t1 = clock();
	
	std::cout << "\nStoring pixels...\n";
	int duplicates = 0; //stores duplicate pixel amount
        while( reader.Next()) { // loop through TTree and store data in Pixel_Store
                duplicates += pStore.add(*event, *fed, *chan, *roc, *row, *col, *adc);
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

	
//	std::ofstream glibHitFile;
//	std::ofstream glibPixFile;
	

	// output process time in seconds
	t2 = clock();
	float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
	std::cout<<"\n\nProgram runtime: " << seconds << " seconds.\n\n";
}

int main( int argc, char *argv[] ) {
// takes file argument and checks it before passing to main function	
	if (argc != 2) { // if no arguments
		std::cout<<"usage: "<<argv[0] <<" <filename>\n";
	}
	else {
        	TFile *file = new TFile(argv[1]);
		if ( file->IsOpen() ) { // check if file loaded correctly
			PixelPacker(file); // pass file to main function
		}
		else std::cout<<"Couln't open file.\n";
	}
	return 1;
}



