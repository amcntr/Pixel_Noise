#include <TFile.h>
#include <TTree.h>

class pixdigi {
	public:
	Int_t event;
	Int_t FED;
	Int_t Ch;
	Int_t Roc;
	Int_t Column;
	Int_t Row;
};

void testtree3() {
	TFile *f = new TFile("t.root", "recreate");
	TTree *tree = new TTree("tree", "new");
	string ename;
	string fname;
	Int_t id;
	Int_t fed;
	for (int i = 0; i < 2; i++) {
		ename = "event " + to_string(i);
		pixdigi pix;
		tree->Branch(ename.c_str(), &id, "id/B");
		for (int j = 0; j < 5; j++) {
			fname = ename + ".fed " + to_string(j);
			TBranch *b = new TBranch(tree->FindBranch(ename.c_str()), fname.c_str(), &fed, "fed/B");
			cout<<tree->FindBranch(ename.c_str())->GetName()<<'\n';
		}
	}
	tree->Write();
	tree->StartViewer();
}

