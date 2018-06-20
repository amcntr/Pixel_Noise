#include <TTree.h>
#include <TFile.h>

struct PIX {
	int fed;
	int ch;
	int col;
	int row;
};

class Pixels {
private:
	vector<int> fed;
	vector<int> ch;
	vector<int> col;
	vector<int> row;
public:
	bool check(PIX pix){
		for(int i = 0; i<fed.size(); i++) {
			if ((pix.fed == fed[i]) && (pix.ch == ch[i]) 
				&& (pix.col == col[i]) && (pix.row == row[i])) {
				return true;
			}
		}
		return false;
	}
	void add(PIX pix){
		fed.push_back(pix.fed);
		ch.push_back(pix.ch);
		col.push_back(pix.col);
		row.push_back(pix.row);
	}
	PIX get(int i) {
		PIX pix;
		pix.fed = fed[i];
		pix.ch = ch[i];
		pix.row = row[i];
		pix.col = col[i];
		return pix;
	}
	int size() {
		return fed.size();
	}
};

Pixels generate(int hits) {
	PIX pix;
	Pixels pixels;
	for (int i = 0; i < hits; i++) {	
		pix.fed = (rand()%10) + 1;
		pix.ch = (rand()%10) + 1;
		pix.row = (rand()%10) + 1;
		pix.col = (rand()%10) + 1;
		if (pixels.check(pix)) {
			i -= 1;
		}
		else {
			pixels.add(pix);
		}
	}
	return pixels;
}

void testtree2() {
	TTree *tree = new TTree("why", "live");
	TFile *f = new TFile("t.root", "recreate");
	int hits = 100;
	Pixels pixels;
	PIX pix;
	string name;
	int eventNum = 10;
	for (int j = 0; j < eventNum; j++) {
		pixels = generate((rand()%hits) + 1);
		name = "event " + to_string(j);
		tree->Branch(name.c_str(), &pix, "fed/B:ch/B:col/B:row/B");
		for (int i = 0; i < pixels.size(); i++) {
			pix = pixels.get(i);
			cout<<pix.fed<<' '<<pix.ch<<' '<<pix.col<<' '<<pix.row<<'\n';
			tree->Fill();
		}
	}
	tree->Write();
}
