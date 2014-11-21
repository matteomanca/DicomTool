INTRODUCTION

DicomTool is a free C++ tool, based on DCMTK lib, that given a dicom image (it also works with compressed dicom images), or a directory containing many dicom images, allows to: 

* Anonymize them; 
* convert them to bitmap format; 
* save their pixel data value in a ascii and/or binary file;



DOWNLOAD AND INSTALLATION
DicomTool has been tested under Ubuntu 64 bit, Mac OSX and Windows 32 and 64 bit using CigWin

git clone git@github.com:matteomanca/DicomTool  
cd DicomTool/Debug
sh configure.sh
make clean
make all

USAGE INFO:

./DicomPreProcessor -h


USAGE EXAMPLES:
  
Anonymize the "test1.dcm" dicom image and save the anonymized version in "test1Anony.dcm". Moreover it saves its pixel values in a ascii file named "test1Asci.txt" and in a 
binary file named "test1Bin.bin".
./DicomPreProcessor -ca /path-of-dicom/test1.dcm /path-of-dicom/out/test1Asci.txt /path-of-dicom/out/test1Bin.bin  ../path-of-dicom/out/test1Anony.dcm


Anonymize all dicom images contained in the "path-of-dicom" directory. Moreover it saves the pixel values in a ascii and binary file. 
All new files will be saved in the "/path-of-dicom/out/" directory.
./DicomPreProcessor -dca /path-of-dicom/ /path-of-dicom/out/ newFileNames




