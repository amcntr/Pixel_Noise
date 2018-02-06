#include <TTree.h>
#include <map>
#include <TObject.h>
#include <TMap.h>

class Pixel_Store : public TObject {
private:
	// Event, FED, Ch, Roc, row, col
	typedef pair<int, int> Pixel;
	typedef map<int, Pixel> Roc;
	typedef map<int, Roc> Ch;
	typedef map<int, Ch> Fed;
	typedef map<int, Fed> Events;
	Events storage_;
	
public:
	Pixel_Store();
	~Pixel_Store();
	void Add_Pixel(int event, int fed, int ch, int roc, int row, int col);
	bool Check_Pixel(int event, int fed, int ch, int roc, int row, int col);
};

Pixel_Store::Pixel_Store() {
	// nothing
}

Pixel_Store::~Pixel_Store() {
	// nothing
}

Pixel_Store::Add_Pixel(int event, int fed, int ch, int roc, int row, int col){
	// description
	if (!Check_Pixel(event, fed, ch, roc, row, col)) {
		storage_[event][fed][ch][roc] = make_pair(row, col);
	}
	else cout<<"Warning: Duplicate Pixel.\n";
}

Pixel_Store::Check_Pixel(int event, int fed, int ch, int roc, int row, int col) {
	// description
	Events::iterator f = storage_.find(event);
	if (f != storage_.end()) {
		Fed::iterator c = f.find(fed);
		if (c != f.end()) {
			Ch::iterator r = c.find(ch);
			if (r != c.end()) {
				Roc::iterator p = r.find(roc);
				if (p != r.end()) {
					if (p == make_pair(row, col) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

