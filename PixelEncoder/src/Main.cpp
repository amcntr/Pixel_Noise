#include "Encoder.h"
#include "Decoder.h"

int main(int argc, char* argv[]) {
    // clock to record process time
    clock_t t1, t2, st1, st2, et1, et2;

    if (argc != 2) {  // if no arguments
        std::cout << "usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    TFile* file = new TFile(argv[1]);
    // check if file loaded correctly
    if (!(file->IsOpen())) {
        std::cout << "Couln't open " << argv[1] << "\n";
        return 1;
    }

    t1 = clock();
    std::cout << "Program start.\n";

    // decode data from TTree
    TTreeReader readerH("HighFedData", file);
    TTreeReaderValue<int> event(readerH, "Data._eventID");
    TTreeReaderValue<int> fed(readerH, "Data._fedID");
    TTreeReaderValue<int> layer(readerH, "Data._layer");
    TTreeReaderValue<int> chan(readerH, "Data._channel");
    TTreeReaderValue<int> roc(readerH, "Data._ROC");
    TTreeReaderValue<int> row(readerH, "Data._row");
    TTreeReaderValue<int> col(readerH, "Data._col");
    TTreeReaderValue<int> adc(readerH, "Data._adc");

    TTreeReader readerZ("ZeroData", file);
    TTreeReaderValue<int> event0(readerZ, "Data._eventID");
    TTreeReaderValue<int> fed0(readerZ, "Data._fedID");
    TTreeReaderValue<int> layer0(readerZ, "Data._layer");

    Encoder encoder;

    st1 = clock();
    std::cout << "\nStoring pixels...\n";
    // stores duplicate pixel amount
    int duplicates = 0;
    // loop through TTree and store data in the encoder
    while (readerH.Next()) {
        duplicates += encoder.add(*event, *fed, *layer, *chan, *roc, *row, *col, *adc);
    }
    while (readerZ.Next()) {
        duplicates += encoder.add(*event0, *fed0, *layer0, 0, 0, 0, 0, 0);
    }

    st2 = clock();
    std::cout << "Done storing pixels. Store time of "
              << (((float)st2 - (float)st1) / CLOCKS_PER_SEC)
              << " seconds.\n\nProcessing Pixels...\n";

    // process stored data
    encoder.process();

    // output is stored in a string to print both to a file and terminal
    std::string output;
    output = "Total duplicate pixels: " + std::to_string(duplicates) +
             "\nTotal events: " + std::to_string(encoder.totalEvents) +
             "\nTotal events with zero hits: " + std::to_string(encoder.totalZeroEvents) +
             "\nTotal hits: " + std::to_string(encoder.totalHits) +
             "\nTotal FEDs: " + std::to_string(encoder.totalFEDs) +
             "\nHighest hits in a roc: " + std::to_string(encoder.hhRoc) +
             "\nHighest hits in a channel: " +std::to_string(encoder.hhChan) +
             "\n\nHighest Avg Hit FED Id: " + std::to_string(encoder.haFEDID) +
             "\nWith an avg hit count of: " + std::to_string(encoder.haFEDhit);

    et1 = clock();
    std::cout << "\n\nEncoding binary files...\n\n";
    encoder.encode(encoder.haFEDID);
    et2 = clock();
    std::cout << "\nDone encoding with an encoding time of "
              << (((float)et2 - (float)et1) / CLOCKS_PER_SEC)
              << " seconds.\n\nGenerating histogram from source data.\n";
    encoder.graph();
    std::cout << "\nDone generating histogram from source data.\n";
    file->Close();

    // print to terminal
    std::cout << output;

    // output process time in seconds
    
    Decoder decoder;

    std::cout<<"\nChecking binary files.\n";
    if (decoder.open("SRAMhit0.bin", 1) != 1)
        std::cout<<"Error: Missing SRAMhit0.bin in directory.\n";
    if (decoder.open("SRAMhit1.bin", 17) != 1)
        std::cout<<"Error: Missing SRAMhit1.bin in directory.\n";
    if (decoder.open("SRAMhit2.bin", 33) != 1)
        std::cout<<"Error: Missing SRAMhit2.bin in directory.\n";

    std::cout << "Done checking binary files.\n\nGenerating histogram from binary data.\n";

    decoder.graph();
    std::cout << "Done generating histgram from binary data.";

    t2 = clock();
    float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
    std::cout << "\n\nProgram finish with a runtime of " << seconds
              << " seconds.\n\n";

    return 1;
}