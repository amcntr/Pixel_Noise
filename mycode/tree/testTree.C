#include <TTree.h>
#include <TFile.h>

struct Pixel {
	int col;
	int row;
};

struct Ch {
	vector<Pixel> pixels;
	int id;
}

struct FED {
	vector<Ch> chans;
	int id;
}

struct Event {
	map<int, map<int, vector<Pixel> > > events;
	int id;
}



Event populateEvent(int id, int hitNum) {
	Event event;
	event.id = id;
	for (int j = 0; j<hitNum; j++) {
		int fedid = (rand()%10) + 1;
		int chid = (rand()%10) + 1;
		int row = (rand()%10) + 1;
		int col = (rand()%10) + 1;
		if (event.events.find(fedid) == event.events.end()) {
			event.events[fedid][chid] =
		}
		else {
			if (event.events[fedid].find(chid) == event.events[fedid].end()) {
				pass
			}
			else {
				pass
			}
		}
	}
	return event;
}

void testTree() {
	TTree *tree = new TTree("t", "Test Tree");
	event Event;
	int eventNum = 100;
	int hitNum = 0;
	for (int i = 0; i<eventNum; i++){
		hitNum = (rand()%1000)+1;
		event = populateEvent(i, hitNum);
		tree->Branch(i, &event);
		tree->Fill();
	}
	tree->StartViewer();
}
