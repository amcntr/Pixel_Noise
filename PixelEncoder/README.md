# Pixel Binary Encoder

This program that takes a root file with a TTree that holds pixdigi data and converts it into a binary format.

## How to use

Run ```make``` to compile the program as is. The program accepts a root file as input.

```
./PixelEncoder /path to file/file.root
```

### Input Format

The input must be a root file with 2 TTrees. 

#### HighFedData TTree

The first TTree must be named HighFedData, with one branch called Data. The branch has 8 leaves: ```_eventID, _fedID, _layer, _channel, _ROC, _row, _col, _adc```

This TTree is for storing hit data.
```
\_eventID : Event number
\_fedID : Fed number 1-139
\_layer : Layer number 1-5
          5 represents FPix layers
\_channel : Channel number 1-48
\_ROC : ROC number 1-8
\_row : row number 0-79
\_col : col number 0-51
\_adc : adc number 0-255
```
#### ZeroData TTree

The second TTree must be named ZeroData, with one branch called Data. It can have the same leaves as the HighFedData TTree, but at least 3 leaves are required: ```_eventID, _fedID, and _layer```

This TTree stores events with zero hits.
```
\_eventID : Event number
\_fedID : Fed number 1-139
\_layer : Layer number 0
          To indicate no hits.
```
### Output

The program outputs 8 files: 2 Histograms, 3 SRAMPix binary files, and 3 SRAMhit binary files.

#### Histograms

#### SRAMpix Files

#### SRAMhit Files
##### Header
##### Body

## Class Structure

Description.

### Encoder.h

Description.

### Decoder.h

Description.

## Authors

Description.

## Acknowledgments

Description.