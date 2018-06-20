// getHits.C
// Root macro that returns a Histogram of the hit distro
// per ch for the FED with highest average hits
// ASM, May 2018

#include <TFile.h>
#include <TKey.h>
#include <TF2.h>

// Declare variables	
double h_avgHitCh = 0.0; // highest number of average hits for all feds
double h_totalHitCh = 0.0; // total hits for highest average hit FED
TKey *h_hitFedCh; // key for fed with hights number of average hits
double h_avgHitEvent = 0.0;
double h_totalHitEvent = 0.0;
TKey *h_hitFedEvent;
int eventCount = 0;

const char * NAME = "hFEDChannel"; // Constant used to search for correct histograms in root file
TH2D *histo2D;
TH1 *histo1D;

// Root file location and internal directory of root file to be examined
const char * path_to_root = "~/../hkt2/raw5.root";
const char * root_directory = "an/hits";
const char * output_file = "Highest-MC.root";


// checks if key is a targeted histogram by comparing key name to NAME variable
// Processes target histogram from a key in root file
// loop through each bin of a TH2D histogram
// add up bins according to number of hits ( multiply bin content by y-axis )
// find average hits per channel and store the highest average
void processKey(TKey* key) {	
	char *sub = strstr(key->GetName(), NAME); // Check if NAME is in keys name, if not returns NULL
	int hitNum = 0; // number of hits in the channel
	int t_hitNum = 0; // number of hits for the FED
	int chNum = 0; // number of active channels
	double avgHitCh = 0.0; // The average hits for a single fed
	double avgHitEvent = 0.0;
	if (sub) {
		histo2D = (TH2D*)key->ReadObj(); // read the histogram associated with the key
		// get the number of bins from the histogram
		int numBinX = histo2D->GetXaxis()->GetNbins();
		int numBinY = histo2D->GetYaxis()->GetNbins();
		// loop through channels / x axis
		for(int i = 1; i <= numBinX; i++) {
			hitNum = 0; // reset hitnum for each channel
			
			// loop through the number of hits / y axis
			for(int j = 1; j <= numBinY; j++) {
				hitNum += histo2D->GetBinContent(i,j) * j; // pull the amount of events from the bin and multiply by number of hits
			}
			
			// if hits are detected in channel, increase active channel count
			if(hitNum > 0) {
				chNum++;
				t_hitNum += hitNum;
			}
		}
		// if FED recorded hits, calculate average number of hits over active channels
		// and average hits per event
		if (t_hitNum > 0) {
			avgHitCh = t_hitNum / chNum;
			avgHitEvent = t_hitNum / eventCount;
			//cout << t_hitNum << " in " << key->GetName() << '\n';
		}
		else {
			//cout << '\t' << key->GetName() << " is empty.\n";
		}
		// check if avg hit is highest
		if(avgHitCh > h_avgHitCh) {
			h_avgHitCh = avgHitCh; // new highest avg hit
			h_totalHitCh = t_hitNum; // new total hits
			h_hitFedCh = key;	// record the key of fed with highest avg hits		
		}
		
		// check if even number of avg hits with highest and throw a warning if true
		else if((avgHitCh == h_avgHitCh) && (avgHitCh > 0.0)) {
			cout << key->GetName() << " is equal to " << h_hitFedCh->GetName() << '\n';
		}

		if(avgHitEvent > h_avgHitEvent) {
			h_avgHitEvent = avgHitEvent;
			h_totalHitEvent = t_hitNum;
			h_hitFedEvent = key;
			
		}
		else if ((avgHitEvent == h_avgHitEvent) && (avgHitEvent > 0.0)) {
			cout << key->GetName() << " is equal to " << h_hitFedEvent->GetName() << '\n';	
		}
	}
	
}


// Main function
// load a root file according to the path_to_root variable
// navigate to target directory to analyze with root_directory variable
// get list of keys in directory and pass to processKey function
// output results in terminal and save target histogram to seperate root file
void getHitsMC2() {
	TFile *raw = new TFile(path_to_root); // open root file
	raw->Cd(root_directory); // change directory to hit folder in root file
	TDirectory *hits = gDirectory; // create new directory type with hit directory
	TIter next(hits->GetListOfKeys());
	TKey *key;
	
	key = hits->FindKey("EventCount");
	histo1D = (TH1*)key->ReadObj(); // read the histogram associated with the key
	eventCount = histo1D->GetBinContent(1);
	//cout << '\n' << eventCount << " events in file.\n";

	// loop through each key in hit directory and process the key
	while ((key = (TKey*)next())) {
		processKey(key);
	}

	// output highest average hits
	if ((h_totalHitCh == 0.0) && (h_totalHitEvent == 0.0)) {
		cout << "\n\nWarning: No hits read from file.\n\n";
	}
	else {
		cout << "\n\nFED with highest average hits per channel is:  FED #"
		     << (atoi(h_hitFedCh->GetName() + 11) + 1200)
		     << "\nWith an average hit count of:  " << h_avgHitCh 
		     << "\nWith a total hit count of:  " << h_totalHitCh << "\n\n";

		cout << "FED with highest average hits per event is:  FED #"
		     << (atoi(h_hitFedEvent->GetName() + 11) + 1200)
		     << "\nWith an average hit count of:  " << h_avgHitEvent 
		     << "\nWith a total hit count of:  " << h_totalHitEvent << "\n\n";
	}	
	// Write the hit distro histogram to a new file and draw it
	cout << "Drawing histogram of hit distribution on FED #"
	     << (atoi(h_hitFedCh->GetName() + 11) + 1200)
	     << "\n\n";

	TFile *highest = new TFile(output_file, "recreate");
	histo2D = (TH2D*)h_hitFedCh->ReadObj();
	histo2D->Draw();
	highest->WriteTObject(histo2D);
}

