// getHits.C
// Root macro that returns a Histogram of the hit distro
// per ch for the FED with highest average hits
// ASM, May 2018

#include <TFile.h>
#include <TKey.h>
#include <TF2.h>

// Declare variables	
double h_avgHitNum = 0.0; // highest number of average hits for all feds
double h_totalHit = 0.0; // total hits for highest average hit FED
TKey *h_HitFed; // key for fed with hights number of average hits
const char * NAME = "hFEDChannel"; // Constant used to search for correct histograms in root file
TH2D *histo;

// Root file location and internal directory of root file to be examined
const char * path_to_root = "raw.root";
const char * root_directory = "d/hits";
const char * output_file = "Highest-dat.root";


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
	double avgHitNum = 0.0; // The average hits for a single fed
	if (sub) {
		histo = (TH2D*)key->ReadObj(); // read the histogram associated with the key
		// get the number of bins from the histogram
		int numBinX = histo->GetXaxis()->GetNbins();
		int numBinY = histo->GetYaxis()->GetNbins();
		// loop through channels / x axis
		for(int i = 1; i <= numBinX; i++) {
			hitNum = 0; // reset hitnum for each channel
			
			// loop through the number of hits / y axis
			for(int j = 1; j <= numBinY; j++) {
				hitNum += histo->GetBinContent(i,j) * j; // pull the amount of events from the bin and multiply by number of hits
			}
			
			// if hits are detected in channel, increase active channel count
			if(hitNum > 0) {
				chNum++;
				t_hitNum += hitNum;
			}
		}
		// if FED recorded hits, calculate average number of hits over active channels
		if (chNum > 0) {
			avgHitNum = t_hitNum / chNum;
		}
		// check if avg hit is highest
		if(avgHitNum > h_avgHitNum) {
			h_avgHitNum = avgHitNum; // new highest avg hit
			h_totalHit = t_hitNum; // new total hits
			h_HitFed = key;	// record the key of fed with highest avg hits		
		}
		// check if even number of avg hits with highest and throw a warning if true
		else if(avgHitNum == h_avgHitNum){
			cout << key->GetName() << " is equal to " << h_HitFed->GetName() << '\n';
		}
	}	
}


// Main function
// load a root file according to the path_to_root variable
// navigate to target directory to analyze with root_directory variable
// get list of keys in directory and pass to processKey function
// output results in terminal and save target histogram to seperate root file
void getHitsData() {
	TFile *raw = new TFile(path_to_root); // open root file
	raw->Cd(root_directory); // change directory to hit folder in root file
	TDirectory *hits = gDirectory; // create new directory type with hit directory
	TIter next(hits->GetListOfKeys());
	TKey *key;

	// loop through each key in hit directory and process the key
	while ((key = (TKey*)next())) {
		processKey(key);
	}

	// output highest average hits
	cout << "\nFED with highest average hits is:  FED #"
	     << (atoi(h_HitFed->GetName() + 11) + 1200)
	     << "\nWith an average hit count of:  " << h_avgHitNum 
	     << "\nWith a total hit count of:  " << h_totalHit << "\n\n";

	// Write the hit distro histogram to a new file and draw it
	TFile *highest = new TFile(output_file, "recreate");
	histo = (TH2D*)h_HitFed->ReadObj();
	highest->WriteTObject(histo);
	histo->Draw();
	//hits->Close();
	//raw->Close();
	//highest->Close();
}

