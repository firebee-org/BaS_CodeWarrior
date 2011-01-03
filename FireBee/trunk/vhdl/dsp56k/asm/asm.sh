#!/bin/bash
export DSP_PATH=~/.wine/drive_c/Programme/Motorola/DSP56300/clas

wine $DSP_PATH/asm56300.exe -b -g -l $1.asm
wine $DSP_PATH/dsplnk.exe $1.cln
wine $DSP_PATH/cldlod.exe $1.cld > $1.lod
