/* Generate a tree in pixel format for testing
 *
 * Tree
 *   Branch FED#
 *      Branch Ch#
 *        Branch Event#
 *          Branch PixelColumn
 *          Branch PixelRow
 * ASM 6/2018 */

#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>

void generateStructure(TTree *tree) {
	tree->Branch
}


void generateEntry(TTree *tree) {
	int FEDID = (rand()%20);
	int ChID = (rand()%10);
	int hitCount = (rand()%10) + 1;
	tree->Branch(
}

void generateTree() {
	// Main function and loop
	int entryCount = 100;
	TTree * tree = new TTree("FED List", "A tree of hit pixels per FED and Event.");
	for (int i = 0; i < entryCount; i++) {
		generateEntry(tree);
	}
	
}
