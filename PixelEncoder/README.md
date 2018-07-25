# Pixel Binary Encoder

This program that takes a root file with a TTree that holds pixdigi data and converts it into a binary format for testing CMS Front End Drivers with a Gigabit Link Interface Board.

## How to use

Run ```make``` to compile the program as is. The program accepts a root file as input and outputs 6 binary files and 2 pdfs.

```
./PixelEncoder /path to file/file.root
```

### Input Format

The input must be a root file with 2 TTrees. 

#### HighFedData TTree

The first TTree must be named HighFedData, with one branch called Data. The branch has 8 leaves: ```_eventID, _fedID, _layer, _channel, _ROC, _row, _col, _adc```

This TTree is for storing hit data.
```
_eventID : Event number
_fedID : Fed number 1-139
_layer : Layer number 1-5
         5 represents FPix layers
_channel : Channel number 1-48
_ROC : ROC number 1-8
_row : row number 0-79
_col : col number 0-51
_adc : adc number 0-255
```
#### ZeroData TTree

The second TTree must be named ZeroData, with one branch called Data. It can have the same leaves as the HighFedData TTree, but at least 3 leaves are required: ```_eventID, _fedID, and _layer```

This TTree stores events with zero hits.
```
_eventID : Event number
_fedID : Fed number 1-139
_layer : Layer number 0
         To indicate no hits.
```
### Output

The program outputs 8 files: 2 Histograms, 3 SRAMPix binary files, and 3 SRAMhit binary files.

#### Histograms

The program makes 2 "Hits per channel" histograms. One for the source data and one for the binary files. This is done to check if the source data was converted to binary successfully.

#### SRAMpix Files

This binary file stores 32-bit strings of hit pixel col, row, and adc. 

The program loops over the source data until the file is exactly 8388608 bytes.

```
Col 0-51  6-bits
row 0-79  9-bits
adc 0-255 8-bits
```
The line format is as follows:

```
0x[25:16]00[13:8][7:0]
   row      col   adc
```

#### SRAMhit Files

Each binary file is seperated into 16 blocks and a header. The header specifies the format of the file. The 16 blocks are 32-bit or 64-bit strings of hits per ROC in a channel; one block per channel.

The program loops over the source data until the file is exactly 8388612 bytes.

##### Header

The header is 4 bytes in size. Each 2 bits in the header represents the format for a block.
```
0x(block1)(block2)...(block15)(block16)
```
The format:
```
00 = Layer 1, 2 ROCs, 32-bit
01 = Layer 2, 4 ROCs, 32-bit
10 = Layer 3-4 or FPix, 8 ROCs, 32-bit
11 = Layer 3-4 or FPix, 8 ROCs, 64-bit
```

##### Body

The body is made up of 32-bit or 64-bit strings that represent hits per ROC in a channel per event. Each block represents a channel. There are 16 channels per file; 3 files cover all 48 channels.

The size of each block is exactly 524288 bytes.

Each string lists the hits per roc. The size in bits of each entry in the string changes depending on the format specified in the header.
```
Header  :   Format
00      :   32-bit string, 16-bits per ROC  0x(Roc 1)(Roc 2)
01      :   32-bit string, 8-bits per ROC   0x(Roc 1)...(Roc 4) 
10      :   32-bit string, 4-bits per ROC   0x(Roc 1)...(Roc 8) 
11      :   64-bit string, 8-bits per ROC   0x(Roc 1)...(Roc 8) 
```

## Class Structure

You can use the Encoder.h and Decoder.h files to add binary generation to another program.
