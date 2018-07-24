#!/bin/bash
FOLDER = $1
LOCATION = /storage1/users/asm12/BinaryData/

mv SRAMhit0.bin $LOCATION$FOLDER/SRAMhit0.bin  
mv SRAMhit1.bin $LOCATION$FOLDER/SRAMhit1.bin
mv SRAMhit2.bin $LOCATION$FOLDER/SRAMhit2.bin
mv SRAMpix2.bin $LOCATION$FOLDER/SRAMpix0.bin
mv SRAMpix1.bin $LOCATION$FOLDER/SRAMpix1.bin
mv SRAMpix0.bin $LOCATION$FOLDER/SRAMpix2.bin
mv histogram_source.pdf $LOCATION$FOLDER/histogram_source.pdf
mv histogram_binary.pdf $LOCATION$FOLDER/histogram_binary.pdf
