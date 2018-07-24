// PixelEncoder Header
// Anthony McIntyre - June 2018
// Using pixel data stored in a root TTree;
// finds the fed id with the highest avg hits,
// encodes pixel addresses into a binary format
// for FED testing. 

#ifndef ENCODER_H
#define ENCODER_H

#include "Includes.h"


// Container types
// row, col as key, adc as value
//    row, col is bit shifted
//    row is shifted 16, col is shifted 8
using Pixels = std::unordered_map<uint32_t, uint32_t>;
// roc id as key and pixel map as value
using ROCs = std::map<int, Pixels>;
// channel id as key and roc map
using Chans = std::map<int, ROCs>;
// event id as key and channel map
using Events = std::map<int, Chans>;
// fed id as key and event map
using FEDs = std::map<int, Events>;

class Encoder {
  // Multiple Pixel Storage Class
  // stores pixels in a nested map structure
 private:
  // hits per fed
  // map of total hits per fed
  std::unordered_map<int, int> hitspFED_; // hits per fed
  // roc highest hits per block
  // id: block id in hits files
  // value: if a roc in block has irregularly high hits
  bool rocHigHitpFile_ = false;
  // Which layer each channel is in.
  // <channel id, layer id>
  std::map<int, int> ChannelLayer_;
  // store events in a hash
  // for event counting
  std::unordered_map<int,int> eventStore_;
  // store events with no hits
  // <fedid, <event id, 1 >
  std::unordered_map<int, std::unordered_map<int, int>> zeroEvents_;
 public:
  // highest average fed id
  // highest avg hit per event fed id
  int haFEDID;
  // highest average fed hits
  // avg number of hits in above fed
  int haFEDhit;
  // highest hits in a channel
  int hhChan;
  // highest hits in a roc
  int hhRoc;
  // total count of stored items
  int totalHits;
  int totalEvents;
  int totalFEDs;
  int totalZeroEvents;
  // main storage
  FEDs storage;
 public:
  Encoder() { }           // constructor
  virtual ~Encoder() { }  // destructor

  // adds a pixel to class
  int add(int event,
          int fed,
          int layer,
          int ch,
          int roc,
          int row,
          int col,
          int adc);

  // process data
  // gets highest hit roc and highest avg hit fed
  // populates histograms in future
  // returns number for error checking
  void process();
  // generate binary files
  void encode(int targetFED);
  // create histogram from source data
  void graph();
};

#endif
