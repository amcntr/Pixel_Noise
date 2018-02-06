#include <TFile.h>
#include <TTree.h>
#include <TObjArray.h>


class pix : public TObject {
	public:
	Int_t col = 0;
	Int_t row = 0;
};

void testtree() {
	TFile *f = new TFile("t.root", "recreate");
	TTree *tree = new TTree("why", "live");
	TObjArray what;
	for (int i = 0; i < 140; i++) {
		// add feds
		TObjArray FED;
		for (int j = 0; j < 48; j++) {
			TObjArray Ch;
			int eventNum = (rand()%100) + 1;
			for (int m = 0; m < eventNum; m++) {
				TObjArray event;
				int hitNum = (rand()%100) + 1;
				for (int n = 0; n < hitNum; n++) {
					pix pixel;
					pixel.col = (rand()%52) + 1;
					pixel.row = (rand()%80) + 1;
					event.Add(pixel);
				}
				Ch.Add(event);
			}
			FED.Add(Ch);
		}
		what.Add(FED);
	}
	tree->Branch("Stuff", &what);
	tree->Fill();
	tree->Write();
}
