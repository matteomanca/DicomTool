//
//  
//  DicomPreProcessor
//
//  Created by Matteo Manca on 12/09/14.
//  Copyright (c) 2014 Matteo Manca. All rights reserved.
//

#include <iostream>



#include <iostream>
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcistrmf.h"
#include "dcmtk/oflog/oflog.h"

#include "dcmtk/dcmimage/diregist.h"   // Support for color images
#include "dcmtk/dcmdata/dcrledrg.h"    // Support for RLE images
#include "dcmtk/dcmjpeg/djdecode.h"    // Support for JPEG images
#include "dcmtk/dcmjpeg/djrplol.h"

#include "dcmtk/ofstd/ofconsol.h"  /* for ofConsole */
#include "dcmtk/dcmimage/diqtid.h"    /* for DcmQuantIdent */
#include "dcmtk/dcmimage/diqtcmap.h"  /* for DcmQuantColorMapping */
#include "dcmtk/dcmimage/diqtpix.h"   /* for DcmQuantPixel */
#include "dcmtk/dcmimage/diqthash.h"  /* for DcmQuantColorHashTable */
#include "dcmtk/dcmimage/diqtctab.h"  /* for DcmQuantColorTable */
#include "dcmtk/dcmimage/diqtfs.h"    /* for DcmQuantFloydSteinberg */
#include "dcmtk/dcmdata/dcswap.h"    /* for swapIfNecessary() */
#include "dcmtk/dcmdata/dcitem.h"    /* for DcmItem */
#include "dcmtk/dcmimgle/dcmimage.h"  /* for DicomImage */
#include "dcmtk/dcmdata/dcdeftag.h"  /* for tag constants */
#include "dcmtk/dcmdata/dcpixel.h"   /* for DcmPixelData */
#include "dcmtk/dcmdata/dcsequen.h"  /* for DcmSequenceOfItems */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmGenerateUniqueIdentifier() */
#include <dirent.h>  /*To access files in dir*/
#include <string>
#include <stdio.h>

#include <sys/stat.h>

using namespace std;

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>




/**
 *  decompress a JPEG-compressed DICOM image file:
 */
void decompressDicom(string fIn, string fOut){
    DJDecoderRegistration::registerCodecs(); // register JPEG codecs
    DcmFileFormat fileformat;
    if (fileformat.loadFile(fIn.c_str()).good())
    {
        DcmDataset *dataset = fileformat.getDataset();
        
        // decompress data set if compressed
        dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
        
        // check if everything went well
        if (dataset->canWriteXfer(EXS_LittleEndianExplicit))
        {
            fileformat.saveFile(fOut.c_str(), EXS_LittleEndianExplicit);
        }
        
//        //BITMAP
//        DicomImage bmImg(dataset,EXS_LittleEndianExplicit);
//        bmImg.writeBMP("testBmp.bmp");// Trying to get the bitmap image from the first frame
    }
    DJDecoderRegistration::cleanup(); // deregister JPEG codecs
}


void saveAsBitmap(string fIn, string fOut){
    DcmFileFormat fileformat;
    if (fileformat.loadFile(fIn.c_str()).good()){
        
        //check if the iput dicom is compressed
        DicomImage *imageq = new DicomImage(fIn.c_str());
        
        string compressedOut;
        if (imageq->getStatus() != EIS_Normal) {
            printf("\n Ignore previous errors, they appear because the input dicom is compressed \n  DICOM file decompression in progress...\n\n");
            compressedOut = fIn.substr(0,fIn.size()-4) + "_decompressed.dcm";
            decompressDicom(fIn, compressedOut);
            OFCondition status = fileformat.loadFile(compressedOut.c_str());
            imageq = new DicomImage(compressedOut.c_str());
            
        }
        
        DcmDataset *dataset = fileformat.getDataset();
        
        //BITMAP
        DicomImage bmImg(dataset,EXS_LittleEndianExplicit);
        bmImg.writeBMP(fOut.c_str());
        remove(compressedOut.c_str());
    }
}

/**
 Remove all person names
 */
DcmFileFormat anonymizeAllPersonNames(DcmFileFormat fileformat){
    DcmItem dset = *fileformat.getDataset();
    DcmStack stack;
    DcmObject *dobj = NULL;
    DcmTagKey tag;
    OFCondition status1 = dset.nextObject(stack, OFTrue);
    while (status1.good())
    {
        dobj = stack.top();
        tag = dobj->getTag();

        if (tag.getGroup() & 1){ // private tag ? // all private data has an odd group number
                stack.pop();
            delete ((DcmItem *)(stack.top()))->remove(dobj);
        }
        DcmVR VR = dobj->getVR();
        OFString VRName = VR.getVRName();
        if(VRName == "PN"){
            //            delete ((DcmItem *)(stack.top()))->remove(dobj);
            fileformat.getDataset()->putAndInsertString(tag, "Anonymous");
//            cout << VRName << endl;
        }

        status1 = dset.nextObject(stack, OFTrue);
    }
    return fileformat;

}

/**
  Remove all private data ( all private data has an odd group number)
 */
DcmFileFormat removeAllPrivateTags(DcmFileFormat fileformat){
    //veryfy if a tag is private:
    //    DcmTagKey tagName = DCM_PatientName;
    //    cout << "isPrivate " << tagName.isPrivate() << endl;

    DcmItem dset = *fileformat.getDataset();
      DcmStack stack;
      DcmObject *dobj = NULL;
      DcmTagKey tag;
      OFCondition status = dset.nextObject(stack, OFTrue);
      while (status.good()){
                dobj = stack.top();
                tag = dobj->getTag();
              if (tag.getGroup() & 1){ // private tag ? // all private data has an odd group number
                  stack.pop();
                  delete ((DcmItem *)(stack.top()))->remove(dobj);
                }
                status = dset.nextObject(stack, OFTrue);
              }
    return fileformat;
    }




/**
 * Remove all data Patients, all private names and all private tags
 */
void anonymize(string fIn, string fOut, int removePatientData, int removeInstitutionData){
    //veryfy if a tag is private:
    printf("\n %s - Anonymize...\n ", fIn.c_str());
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(fIn.c_str());
    
    DicomImage *imageq = new DicomImage(fIn.c_str());
    
    string compressedOut;
    if (imageq->getStatus() != EIS_Normal) {
        printf("\n Ignore previous errors, they appear because the input dicom is compressed \n  DICOM file decompression in progress...\n\n");
        compressedOut = fIn.substr(0,fIn.size()-4) + "_decompressed.dcm";
        decompressDicom(fIn, compressedOut);
        status = fileformat.loadFile(compressedOut.c_str());
        imageq = new DicomImage(compressedOut.c_str());
        
    }

    if(removePatientData ==1){
		fileformat.getDataset()->putAndInsertString(DCM_PatientID, " ");
		fileformat.getDataset()->putAndInsertString(DCM_OtherPatientIDs, " ");
		fileformat.getDataset()->putAndInsertString(DCM_OtherPatientNames, " ");
		fileformat.getDataset()->putAndInsertString(DCM_PatientBirthDate, " ");
		fileformat.getDataset()->putAndInsertString(DCM_PatientSex, " ");
        fileformat.getDataset()->putAndInsertString(DCM_PatientAddress, " ");
        fileformat.getDataset()->putAndInsertString(DCM_PatientAge, " ");
        fileformat.getDataset()->putAndInsertString(DCM_PatientBirthName, " ");
        


	    fileformat = anonymizeAllPersonNames(fileformat);
    }

    if(removeInstitutionData == 1){
		fileformat.getDataset()->putAndInsertString(DCM_InstitutionName, " ");
		fileformat.getDataset()->putAndInsertString(DCM_InstitutionAddress, " ");
		fileformat.getDataset()->putAndInsertString(DCM_InstitutionalDepartmentName, " ");
		fileformat.getDataset()->putAndInsertString(DCM_InstitutionCodeSequence, " ");
    }

    fileformat = removeAllPrivateTags(fileformat);
    removeAllPrivateTags(fileformat.getDataset());


    OFCondition status3 = fileformat.saveFile(fOut.c_str(), EXS_LittleEndianExplicit);
    if (status3.bad())
        cerr << "Error: cannot write DICOM file (" << status3.text() << ")" << endl;
    
    remove(compressedOut.c_str());

}


void printUint8(int width, int height, string fOutAscii, string fOutBin, const Uint8 *pixel){
//    const Uint8 *pixel = NULL;

        /* do whatever you want with the 'pixel' array */
        if (pixel != NULL){

            /*create the ASCII file*/
            fstream rawData;
            rawData.open(fOutAscii.c_str(), ios::out);

            /*create the binary file*/
            ofstream rawDataBin(fOutBin.c_str(),ios::binary);


            //            cout << width*height <<endl;

            for (int i=0; i < width*height;i++) {
                rawData << (int)pixel[i] ;  //write in txt file
                rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file

                if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                    rawData << "\n";
                    rawDataBin.write("\n", sizeof(char));

                }else{
                    rawData << "," ;
                    rawDataBin.write(",", sizeof(char));
                }

            }
            rawData.close();
            rawDataBin.close();
        }
}


void printUint16(int width, int height, string fOutAscii, string fOutBin, const Uint16 *pixel){
    //    const Uint8 *pixel = NULL;

    /* do whatever you want with the 'pixel' array */
    if (pixel != NULL){

        /*create the ASCII file*/
        fstream rawData;
        rawData.open(fOutAscii.c_str(), ios::out);

        /*create the binary file*/
        ofstream rawDataBin(fOutBin.c_str(),ios::binary);


        //            cout << width*height <<endl;

        for (int i=0; i < width*height;i++) {
            rawData << (int)pixel[i] ;  //write in txt file
            rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file

            if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                rawData << "\n";
                rawDataBin.write("\n", sizeof(char));

            }else{
                rawData << "," ;
                rawDataBin.write(",", sizeof(char));
            }

        }
        rawData.close();
        rawDataBin.close();
    }
}


void printUint32(int width, int height, string fOutAscii, string fOutBin, const Uint32 *pixel){
    //    const Uint8 *pixel = NULL;

    /* do whatever you want with the 'pixel' array */
    if (pixel != NULL){

        /*create the ASCII file*/
        fstream rawData;
        rawData.open(fOutAscii.c_str(), ios::out);

        /*create the binary file*/
        ofstream rawDataBin(fOutBin.c_str(),ios::binary);


        //            cout << width*height <<endl;

        for (int i=0; i < width*height;i++) {
            rawData << (int)pixel[i] ;  //write in txt file
            rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file

            if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                rawData << "\n";
                rawDataBin.write("\n", sizeof(char));

            }else{
                rawData << "," ;
                rawDataBin.write(",", sizeof(char));
            }

        }
        rawData.close();
        rawDataBin.close();
    }
}


/////////NEW 10/01/2014
void printSint8(int width, int height, string fOutAscii, string fOutBin, const Uint8 *pixel){
    //    const Uint8 *pixel = NULL;
    
    /* do whatever you want with the 'pixel' array */
    if (pixel != NULL){
        
        /*create the ASCII file*/
        fstream rawData;
        rawData.open(fOutAscii.c_str(), ios::out);
        
        /*create the binary file*/
        ofstream rawDataBin(fOutBin.c_str(),ios::binary);
        
        
        //            cout << width*height <<endl;
        
        for (int i=0; i < width*height;i++) {
            rawData << (int)pixel[i] ;  //write in txt file
            rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file
            
            if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                rawData << "\n";
                rawDataBin.write("\n", sizeof(char));
                
            }else{
                rawData << "," ;
                rawDataBin.write(",", sizeof(char));
            }
            
        }
        rawData.close();
        rawDataBin.close();
    }
}


void printSint16(int width, int height, string fOutAscii, string fOutBin, const Sint16 *pixel){
    //    const Uint8 *pixel = NULL;
    
    /* do whatever you want with the 'pixel' array */
    if (pixel != NULL){
        
        /*create the ASCII file*/
        fstream rawData;
        rawData.open(fOutAscii.c_str(), ios::out);
        
        /*create the binary file*/
        ofstream rawDataBin(fOutBin.c_str(),ios::binary);
        
        
        //            cout << width*height <<endl;
        
        for (int i=0; i < width*height;i++) {
            rawData << (int)pixel[i] ;  //write in txt file
            rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file
            
            if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                rawData << "\n";
                rawDataBin.write("\n", sizeof(char));
                
            }else{
                rawData << "," ;
                rawDataBin.write(",", sizeof(char));
            }
            
        }
        rawData.close();
        rawDataBin.close();
    }
}


void printSint32(int width, int height, string fOutAscii, string fOutBin, const Sint32 *pixel){
    //    const Uint8 *pixel = NULL;
    
    /* do whatever you want with the 'pixel' array */
    if (pixel != NULL){
        
        /*create the ASCII file*/
        fstream rawData;
        rawData.open(fOutAscii.c_str(), ios::out);
        
        /*create the binary file*/
        ofstream rawDataBin(fOutBin.c_str(),ios::binary);
        
        
        //            cout << width*height <<endl;
        
        for (int i=0; i < width*height;i++) {
            rawData << (int)pixel[i] ;  //write in txt file
            rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file
            
            if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                rawData << "\n";
                rawDataBin.write("\n", sizeof(char));
                
            }else{
                rawData << "," ;
                rawDataBin.write(",", sizeof(char));
            }
            
        }
        rawData.close();
        rawDataBin.close();
    }
}


////////////



/**
 * Given a .dcm (DICOM) file save the raw pixel intensities in ascci and binary format
 */

void dmc2Raw(string fIn, string fOutAscii, string fOutBin){
    printf("\n %s - Dicom to Raw data... \n", fIn.c_str());
    
    DcmFileFormat dff;
    
    OFCondition status = dff.loadFile(fIn.c_str());
    DicomImage *imageq = new DicomImage(fIn.c_str());
    
    string compressedOut;
    if (imageq->getStatus() != EIS_Normal) {
        printf("\n Ignore previous errors, they appear because the input dicom is compressed \n  DICOM file decompression in progress...\n\n");
        compressedOut = fIn.substr(0,fIn.size()-4) + "_decompressed.dcm";
        decompressDicom(fIn, compressedOut);
        OFCondition status = dff.loadFile(compressedOut.c_str());
        imageq = new DicomImage(compressedOut.c_str());

    }

    int width = (int)imageq->getWidth();
    //    cout << "image width =  " << width <<endl;

    int height = (int)imageq->getHeight();
    //    cout << "image height =  " << height <<endl;

    DcmDataset *dataset = dff.getDataset();


    bool convert = false;
    EP_Representation pixelRep = imageq->getInterData()->getRepresentation();
    const Uint8 *pixel8 = NULL;
    const Uint16 *pixel16 = NULL;
    const Uint32 *pixel32 = NULL;
    
    ///////new
    const Uint8 *pixelS8 = NULL;
    const Sint16 *pixelS16 = NULL;
    const Sint32 *pixelS32 = NULL;
    ////////
    
    switch(pixelRep)
    {
        case EPR_Uint8:{
                // UCHAR;

            if (dataset->findAndGetUint8Array(DCM_PixelData, pixel8).good()){
                convert = true;
            }
            break;
        }
        case EPR_Uint16:{
                //USHORT

                if (dataset->findAndGetUint16Array(DCM_PixelData, pixel16).good()){
                    convert = true;
                }
            break;}
        case EPR_Uint32:{
                //UINT

                if (dataset->findAndGetUint32Array(DCM_PixelData, pixel32).good()){
                    convert = true;
                }

            break;}
         ///////new
        case EPR_Sint8:{
            // SCHAR;
            
            if (dataset->findAndGetUint8Array(DCM_PixelData, pixelS8).good()){
                convert = true;
            }
            break;
        }
        case EPR_Sint16:{
            //USHORT
            
            if (dataset->findAndGetSint16Array(DCM_PixelData, pixelS16).good()){
                convert = true;
            }
            break;}
        case EPR_Sint32:{
            //UINT
            
            if (dataset->findAndGetSint32Array(DCM_PixelData, pixelS32).good()){
                convert = true;
            }
            
            break;}
    
        ///////////
        default:{
                cout << " representation not implemented " << endl;
            return ;
        }
    }

    if (convert) {
        if (pixel8 != NULL) {
            printUint8(width, height, fOutAscii, fOutBin, pixel8);
        }else if (pixel16 != NULL){
            printUint16(width, height, fOutAscii, fOutBin, pixel16);
        }else if (pixel32 != NULL){
            printUint32(width, height, fOutAscii, fOutBin, pixel32);
        }
        
        /////NEW
        else if (pixelS8 != NULL){
            printSint8(width, height, fOutAscii, fOutBin, pixelS8);
        }else if (pixelS16 != NULL){
            printSint16(width, height, fOutAscii, fOutBin, pixelS16);
        }else if (pixelS32 != NULL){
            printSint32(width, height, fOutAscii, fOutBin, pixelS32);
        }
        //////
    }

    remove(compressedOut.c_str());
}



/**
 * bakup
 *
 * Given a .dcm (DICOM) file save the raw pixel intensities in ascci and binary format
 */

/*
void dmc2Raw(string fIn, string fOutAscii, string fOutBin){
    DcmFileFormat dff;

    OFCondition status = dff.loadFile(fIn.c_str());
    DicomImage *imageq = new DicomImage(fIn.c_str());

    int width = (int)imageq->getWidth();
//    cout << "image width =  " << width <<endl;

    int height = (int)imageq->getHeight();
//    cout << "image height =  " << height <<endl;

    DcmDataset *dataset = dff.getDataset();
    const Uint8 *pixel = NULL;

    if (dataset->findAndGetUint8Array(DCM_PixelData, pixel).good()){

         do whatever you want with the 'pixel' array
        if (pixel != NULL){

            create the ASCII file
            fstream rawData;
            rawData.open(fOutAscii.c_str(), ios::out);

            create the binary file
            ofstream rawDataBin(fOutBin.c_str(),ios::binary);


//            cout << width*height <<endl;

            for (int i=0; i < width*height;i++) {
                rawData << int(pixel[i]) ;  //write in txt file
                rawDataBin.write((char*)(&pixel[i]), sizeof(pixel[i]));  //write in bin file

                if ((i+1) % width == 0 && i+1 != width*height) { //End of array not reached
                    rawData << "\n";
                    rawDataBin.write("\n", sizeof(char));

                }else{
                    rawData << "," ;
                    rawDataBin.write(",", sizeof(char));
                }

            }
            rawData.close();
            rawDataBin.close();
        }
    }


}
*/


/**
 *
 * Read the binary file created with the dmc2Raw function
 */
void read_binary_file(string fileName)
{


    ifstream file(fileName.c_str(),ios::binary);
    if (file.is_open())
    {
        while( !file.eof() ){
            Uint16 toRestore;
            file.read((char*)&toRestore,sizeof(Uint16));      //read the Uint value
            cout << toRestore << endl;

            char coma;
            file.read((char*)&coma,sizeof(char));  //read the comma char
//            cout << toRestore << endl;

        }

    }
    else cout << "Unable to open file";



}

/**
 Returns the image size in mm
 */
void getPixelSpacing(string fIn){
    
    
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(fIn.c_str());
    if (status.good()){
        
        bool tagSpac = true;
        double dcmPixelSpacing[3];
        if (fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, dcmPixelSpacing[0],0).bad()){
            cerr << "Cannot access pixelSpacing tag (0028,0030) -row spacing- " << endl;
            tagSpac = false;
        }
        
        if (fileformat.getDataset()->findAndGetFloat64(DCM_PixelSpacing, dcmPixelSpacing[1],1).bad()){
            cerr << "Cannot access pixelSpacing tag (0028,0030) -column spacing- " << endl;
        }
        
        if (!tagSpac) {
            cout << "\nTry with ImagerPixelSpacing tag (0018,1164):\n " << endl;
            
            if (fileformat.getDataset()->findAndGetFloat64(DCM_ImagerPixelSpacing, dcmPixelSpacing[0],0).bad()){
                cerr << "Cannot access ImagerPixelSpacing tag (0018,1164) -row spacing- " << endl;
            }
            
            if (fileformat.getDataset()->findAndGetFloat64(DCM_ImagerPixelSpacing, dcmPixelSpacing[1],1).bad()){
                cerr << "Error: cannot access ImagerPixelSpacing tag (0018,1164) -column spacing- " << endl;
            }
            
            cout << "Imager ";
        }
        
        cout << "Pixel Spacing : " << dcmPixelSpacing[0] << " , " <<  dcmPixelSpacing[1] << endl;
        
        long width; //number of row pixels
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Columns, width).bad()){
            cerr << "Cannot access DCM_Columns !" << endl;
        }
        
        long height; //number of column pixels
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Rows, height).bad()){
            cerr << "Cannot access DCM_Rows !" << endl;
        }
        
        cout << "\npixel image width =  " << width  <<endl;
        cout << "pixel image height =  " << height  <<endl;
        
        Float64 rowSpacing = dcmPixelSpacing[0];
        Float64 colSpacing = dcmPixelSpacing[1];
        
        Float64 width_mm = width * colSpacing;
        Float64 height_mm = height * rowSpacing;
        
        //        cout << "Pixel Spacing : " << dcmPixelSpacing[0] << " , " <<  dcmPixelSpacing[1] << endl;
        
        cout << "\nImage width =  " << width_mm << " mm"  <<endl;
        cout << "Image height =  " << height_mm << " mm"  <<endl;
        
        
    } else
        cerr << "Error: cannot read DICOM file (" << status.text() << ")" << endl;
}


static void show_usage(std::string name){
    std::cerr << "Usage: " << name << " <option(s)> SOURCES"
              << "Options:\n"
              << "\t-c <INPUT_DICOM_FILE>	<OUT_RAW_ASCI_FILE>	<OUT_RAW_BIN_FILE> Given an input dicom file INPUT_DICOM_FILE - [convert it in asci and binary format] \n"
              << "\t-a <INPUT_DICOM_FILE>	<OUT_DICOM_ANONYMOUS_FILE> Given an input dicom file INPUT_DICOM_FILE - [creates an anonymous version of the same dicom] \n"
              << "\t-ca <INPUT_DICOM_FILE>	<OUT_RAW_ASCI_FILE>	<OUT_RAW_BIN_FILE>	<OUT_DICOM_ANONYMOUS_FILE> - [create asci and binary files and the anonymous version]\n"
              << "\t-dc <DICOM_DIR_PATH>	<OUT_DIR_PATH> <NEW_FILE_NAMES> - [Convert all files contained in DICOM_DIR_PATH to ascii and binary format and save them in OUT_DIR_PATH with the name <NEW_FILE_NAMES#i>] \n"
              << "\t-da <DICOM_DIR_PATH>	<OUT_DIR_PATH> <NEW_FILE_NAMES> [Create an anonymous version of the dicom files contained in DICOM_DIR_PATH and save them in OUT_DIR_PATH with the name <NEW_FILE_NAMES#i>]  \n"
              << "\t-dca <DICOM_DIR_PATH> 	<OUT_DIR_PATH> <NEW_FILE_NAMES> [Create an anonymous version of the dicom files contained in DICOM_DIR_PATH and Convert all files in ascii and binary format. All new files are saved in OUT_DIR_PATH with the name <NEW_FILE_NAMES#i>]  \n"
              << "\t-s <INPUT_DICOM_FILE> - [Returns the size of the pixel spacing and the image size in millimeters]\n"
              << "\t-bmp <INPUT_DICOM_FILE> <OUTPUT_BITMAP_FILE> - [Save the Dicom image as Bitmap file (.bmp)]\n"
                << "\t-dbmp  <DICOM_DIR_PATH>	<OUT_DIR_PATH> <OUTPUT_BITMAP_FILE> - [Save all Dicom images contained in DICOM_DIR_PATH as Bitmap file (.bmp) in OUT_DIR_PATH]\n"
    
    << std::endl;
}


int main(int argc, const char * argv[])
{
    
//    string in = "/Users/Matteo/Documents/Projects/Qt/DicomPreProcessorGUI/DicomPreProcessor-2014-10-30/test-dicom/14510.dcm";
//    string out="/Users/Matteo/Documents/Projects/Qt/DicomPreProcessorGUI/DicomPreProcessor-2014-10-30/test-dicom/14510-decompressed.dcm";
//    
//    decompressDicom(in,out);
//    
//    return 1;
    
////    ///////////
    
	setenv("DCMDICTPATH","../lib/dcmtk-3.6.1/share/dcmtk/dicom.dic",1);
	 if (argc < 3) {
	        show_usage(argv[0]);
	        return 1;
	    }

	 string dicomPath;
	 string outDicomAscii;
	 string outDicomBin;
	 string outDicomPathAnonymized;
     string outBmp;

	 //for(int i=0; i < argc; i++){
	 	 int i=1;
		 std::string arg = argv[i];
	      /*option h show the help infomation*/
		 if(arg == "-h"){
	    	  show_usage(argv[0]);
	      /*option u present the username*/
		 }else if(arg == "-c"){
	    	  dicomPath = argv[i+1];
	    	  outDicomAscii = argv[i+2];
	    	  outDicomBin = argv[i+3];
	    	  dmc2Raw(dicomPath,outDicomAscii, outDicomBin);//IMAGE 2 RAW DATA ASCII FORMAT
             printf ("\n\n Task Completed \n\n");
	      /*option p present the password*/
         }else if(arg == "-bmp"){
             dicomPath = argv[i+1];
             outBmp = argv[i+2];
             saveAsBitmap(dicomPath,outBmp);//create bitmap file
             printf ("\n\n Task Completed \n\n");
             /*option p present the password*/
         }else if(arg == "-s"){
             dicomPath = argv[i+1];
             getPixelSpacing(dicomPath);
             /*option p present the password*/
         }else if(arg == "-a"){
             char pd = ' ';
             string rpd = "";
             if(argc == 6){
                 rpd = argv[i+3]; //remove patient data gui
             }
             if(rpd == ""){
                 printf ("Do you want to remove patient data ? [y/n] ");
                 scanf (" %c",&pd);
             }
			 int removePData = 0;
			 if(pd == 'y' || rpd == "y")
				 removePData = 1;

			 fflush(stdin);

			 char id = ' ';
             string rid = "";
             if(argc == 6){
               rid = argv[i+4]; //remove institution data gui
             }
             if(rid==""){
                 printf ("Do you want to remove institution data ? [y/n] ");
                 scanf (" %c",&id);
             }
			 int removeIData = 0;
			if(id == 'y' || rid == "y")
				removeIData = 1;

			 dicomPath = argv[i+1];
			 outDicomPathAnonymized = argv[i+2];
			 anonymize(dicomPath, outDicomPathAnonymized,removePData, removeIData); //ANONYMIZATION
            printf ("Task Completed");
		 }else if(arg == "-ca"){
	    	  dicomPath = argv[i+1];
	    	  outDicomAscii = argv[i+2];
	    	  outDicomBin = argv[i+3];
	    	  outDicomPathAnonymized = argv[i+4];
	    	  dmc2Raw(dicomPath,outDicomAscii, outDicomBin);//IMAGE 2 RAW DATA ASCII FORMAT
             
             printf ("Ascii and binary files created\n");

             char pd = ' ';
             string rpd = "";
//            printf("\n\n %lu \n\n " , argc);
             if (argc == 8) {
                 rpd = argv[i+5]; //remove patient data gui

             }
             
              if(rpd == ""){
                  printf ("Do you want to remove patient data ? [y/n] ");
                  scanf (" %c",&pd);
              }
			  int removePData = 0;
			  if(pd == 'y' || rpd == "y")
				 removePData = 1;

			  fflush(stdin);

			  char id = ' ';
             string rid = "";
             if(argc == 8){
                 rid = argv[i+6]; //remove institution data gui
             }
              if(rid==""){
                  printf ("Do you want to remove institution data ? [y/n] ");
                  scanf (" %c",&id);
              }
             
			 int removeIData = 0;
			if(id == 'y' || rid == "y")
              removeIData = 1;
            
             anonymize(dicomPath, outDicomPathAnonymized, removePData, removeIData); //ANONYMIZATION
              printf ("Task Completed");
             
             
	      /*option p present the password*/
		 }else if(arg == "-dc"){
			 DIR *dir;
			 struct dirent *ent;
			 if ((dir = opendir (argv[i+1])) != NULL) {
			/* print all the files and directories within directory */
				 string dirName = argv[i+1];  //directory that contains the files to convert
				 string outFolder = argv[i+2]; //directory that will contain the created files
				 string newFilesName = argv[i+3]; //the neame of the created files will be newFileName1, newFileName2,...newFileNamen
				 int fileNumber = 1;
                 
                 /////
                 if (dirName == outFolder) {
                     string newOutputDirPath = outFolder + "output/";
                     struct stat st = {0};
                     
                     if (stat(newOutputDirPath.c_str(), &st) == -1) {
                         mkdir(newOutputDirPath.c_str(), 0700);
                     }
                     outFolder = newOutputDirPath;
                     printf ("\nNew directory \"output\" created. The generated files will be stored there.");

                 }
                 /////

				 /**
				  *log file that dump the created names and the original names in an ascii file
				  */
				 fstream logFile;
				 time_t ct;
				 ct = time(NULL);
				 std::ostringstream datetime;
				 datetime << ctime(&ct);
				 string datetimeString = "log-conv" + datetime.str();
				 replace( datetimeString.begin(), datetimeString.end(), ' ', '_'); // replace all 'x' to 'y'
				 //string temp = "conversion_log_";
				 string logFileName = outFolder + datetimeString + ".txt";
				 logFile.open(logFileName.c_str(), ios::out);

				 logFile << "DATA : " << datetime.str() << endl;
				 logFile << "Original File \t,\t Created File" << endl;


				 while ((ent = readdir (dir)) != NULL) {
					 dicomPath = dirName + ent->d_name;
					 string fileName = ent->d_name;
					 struct stat st_buf;
					 stat (dicomPath.c_str(), &st_buf);
					 if(fileName.substr(0,1) != "." && S_ISREG (st_buf.st_mode) ){
						 printf("file: %s \n", dicomPath.c_str());
						 std::ostringstream fileNumberString;
						 fileNumberString << fileNumber;
						 outDicomAscii = outFolder + newFilesName + fileNumberString.str() + ".txt";
						 outDicomBin = outFolder + newFilesName + fileNumberString.str() + ".bin";
						 dmc2Raw(dicomPath,outDicomAscii, outDicomBin);//IMAGE 2 RAW DATA ASCII FORMAT

						 logFile << dicomPath << "\t,\t" << outDicomAscii.substr(0,outDicomAscii.size()-4) + ".[txt|bin]" << endl;

						 fileNumber++;
					 }
				 }
			 closedir (dir);
			 logFile.close();
            printf ("\n\n Task Completed \n\n");
			 }
         }else if(arg == "-dbmp"){
             DIR *dir;
             struct dirent *ent;
             if ((dir = opendir (argv[i+1])) != NULL) {
                 /* print all the files and directories within directory */
                 string dirName = argv[i+1];  //directory that contains the files to convert
                 string outFolder = argv[i+2]; //directory that will contain the created files
                 string newFilesName = argv[i+3]; //the neame of the created files will be newFileName1, newFileName2,...newFileNamen
                 int fileNumber = 1;
                 /////
                 if (dirName == outFolder) {
                     string newOutputDirPath = outFolder + "output/";
                     struct stat st = {0};
                     
                     if (stat(newOutputDirPath.c_str(), &st) == -1) {
                         mkdir(newOutputDirPath.c_str(), 0700);
                     }
                     outFolder = newOutputDirPath;
                     printf ("\nNew directory \"output\" created. The generated files will be stored there.");
                     
                 }
                 /////
                 
                 /**
                  *log file that dump the created names and the original names in an ascii file
                  */
                 fstream logFile;
                 time_t ct;
                 ct = time(NULL);
                 std::ostringstream datetime;
                 datetime << ctime(&ct);
                 string datetimeString = "log-bitmap" + datetime.str();
                 replace( datetimeString.begin(), datetimeString.end(), ' ', '_'); // replace all 'x' to 'y'
                 //string temp = "conversion_log_";
                 string logFileName = outFolder + datetimeString + ".txt";
                 logFile.open(logFileName.c_str(), ios::out);
                 
                 logFile << "DATA : " << datetime.str() << endl;
                 logFile << "Original File \t,\t Created File" << endl;
                 
                 
                 while ((ent = readdir (dir)) != NULL) {
                     dicomPath = dirName + ent->d_name;
                     string fileName = ent->d_name;
                     struct stat st_buf;
                     stat (dicomPath.c_str(), &st_buf);
                     if(fileName.substr(0,1) != "." && S_ISREG (st_buf.st_mode) ){
                         printf("file: %s \n", dicomPath.c_str());
                         std::ostringstream fileNumberString;
                         fileNumberString << fileNumber;
                         outBmp = outFolder + newFilesName + fileNumberString.str() + ".bmp";
                         saveAsBitmap(dicomPath,outBmp);//IMAGE 2 RAW DATA ASCII FORMAT
                         
                         logFile << dicomPath << "\t,\t" << outDicomAscii.substr(0,outBmp.size()-4) + ".bmp" << endl;
                         
                         fileNumber++;
                     }
                 }
                 closedir (dir);
                 logFile.close();
                 printf ("\n\n Task Completed \n\n");
             }
         }
    
    
         else if(arg == "-da"){
				 DIR *dir;
				 struct dirent *ent;
				 if ((dir = opendir (argv[i+1])) != NULL) {
				/* print all the files and directories within directory */
					 string dirName = argv[i+1];  //directory that contains the files to convert
					 string outFolder = argv[i+2]; //directory that will contain the created files
					 string newFilesName = argv[i+3]; //the neame of the created files will be newFileName1, newFileName2,...newFileNamen
					 int fileNumber = 1;
                     /////
                     if (dirName == outFolder) {
                         string newOutputDirPath = outFolder + "output/";
                         struct stat st = {0};
                         
                         if (stat(newOutputDirPath.c_str(), &st) == -1) {
                             mkdir(newOutputDirPath.c_str(), 0700);
                         }
                         outFolder = newOutputDirPath;
                         printf ("\nNew directory \"output\" created. The generated files will be stored there.");
                         
                     }
                     /////
                     
					 char pd = ' ' ;
                     string rpd = "";
                     if(argc == 7){
                         rpd = argv[i+4]; //remove patient data gui
                     }
                     
					 if(rpd==""){
                         printf ("Do you want to remove patient data ? [y/n] ");
                         scanf (" %c",&pd);
                     }
					 int removePData = 0;
					 if(pd == 'y'|| rpd == "y"){
						 removePData = 1;
					 }
					 fflush(stdin);

					 char id = ' ';

                     string rid = "";
                     if(argc == 7){
                         rid = argv[i+5]; //remove institution data gui
                     }
					 if(rid==""){
                         printf ("Do you want to remove institution data ? [y/n] ");
                         scanf (" %c",&id);
                     }
					 int removeIData = 0;
					
                    if(id == 'y'|| rid == "y"){
						removeIData = 1;
					}


					/**
					  *log file that dump the created names and the original names in an ascii file
					  */
					 fstream logFile;
					 time_t ct;
					 ct = time(NULL);
					 std::ostringstream datetime;
					 datetime << ctime(&ct);
					 string datetimeString = "log-Anon-" + datetime.str();
					 replace( datetimeString.begin(), datetimeString.end(), ' ', '_'); // replace all 'x' to 'y'
					 //string temp = "conversion_log_";
					 string logFileName = outFolder + datetimeString + ".txt";
					 logFile.open(logFileName.c_str(), ios::out);

					 logFile << "DATA : " << datetime.str() << endl;
					 logFile << "Original File \t,\t Created File" << endl;


					while ((ent = readdir (dir)) != NULL) {
						 dicomPath = dirName +  ent->d_name;
						 string fileName = ent->d_name;
						 struct stat st_buf;
						 stat (dicomPath.c_str(), &st_buf);
						 if(fileName.substr(0,1) != "." && S_ISREG (st_buf.st_mode) ){
							 printf("file: %s \n", dicomPath.c_str());
							 std::ostringstream fileNumberString;
							 fileNumberString << fileNumber;
							 outDicomPathAnonymized = outFolder + newFilesName + fileNumberString.str() +".dcm";
							 anonymize(dicomPath, outDicomPathAnonymized, removePData, removeIData); //ANONYMIZATION
							 fileNumber++;
							 logFile << dicomPath << "\t,\t" << outDicomPathAnonymized.substr(0,outDicomPathAnonymized.size()-4) + ".dcm" << endl;

						 }
					 }
				 closedir (dir);
				 logFile.close();
                printf ("\n\n Task Completed \n\n");
				 }
			 }else if(arg == "-dca"){
				 DIR *dir;
				 struct dirent *ent;

				 if ((dir = opendir (argv[i+1])) != NULL) {
				/* print all the files and directories within directory */
					 string dirName = argv[i+1];  //directory that contains the files to convert
					 string outFolder = argv[i+2]; //directory that will contain the created files
					 string newFilesName = argv[i+3]; //the name of the created files will be newFileName1, newFileName2,...newFileNamen
					 int fileNumber = 1;
                     
                     /////
                     if (dirName == outFolder) {
                         string newOutputDirPath = outFolder + "output/";
                         struct stat st = {0};
                         
                         if (stat(newOutputDirPath.c_str(), &st) == -1) {
                             mkdir(newOutputDirPath.c_str(), 0700);
                         }
                         outFolder = newOutputDirPath;
                         printf ("\nNew directory \"output\" created. The generated files will be stored there.");
                         
                     }
                     /////

					 char pd = ' ';
                     string rpd = "";
                     if(argc == 7){
                         rpd = argv[i+4]; //remove patient data gui
                     }
                    if(rpd == ""){
                        printf ("Do you want to remove patient data ? [y/n] ");
                        scanf (" %c",&pd);
                    }
					 int removePData = 0;
					 if(pd == 'y'|| rpd == "y"){
						 removePData = 1;
					 }
					 fflush(stdin);

					 char id = ' ';
                    string rid = "";
                    if(argc == 7){
                        rid = argv[i+5]; //remove institution data gui
                    }
                    if(rid==""){
                        printf ("Do you want to remove institution data ? [y/n] ");
                        scanf (" %c",&id);
                    }
                 
                    int removeIData = 0;
                    if(id == 'y' || rid == "y"){
						removeIData = 1;
					}

					 /**
					  *log file that dump the created names and the original names in an ascii file
					  */
					 fstream logFile;
					 time_t ct;
					 ct = time(NULL);
					 std::ostringstream datetime;
					 datetime << ctime(&ct);
					 string datetimeString = "log-conv-anon" + datetime.str();
					 replace( datetimeString.begin(), datetimeString.end(), ' ', '_'); // replace all 'x' to 'y'
					 //string temp = "conversion_log_";
					 string logFileName = outFolder + datetimeString + ".txt";
					 logFile.open(logFileName.c_str(), ios::out);

					 logFile << "DATA : " << datetime.str() << endl;
					 logFile << "Original File \t,\t Created File" << endl;


					 while ((ent = readdir (dir)) != NULL) {
						 dicomPath = dirName + ent->d_name;
						 string fileName = ent->d_name;
						 struct stat st_buf;
						 stat (dicomPath.c_str(), &st_buf);
						 if(fileName.substr(0,1) != "." && S_ISREG (st_buf.st_mode) ){

							 printf("\n\nfile: %s \n", dicomPath.c_str());
							 std::ostringstream fileNumberString;
							 fileNumberString << fileNumber;
							 outDicomAscii = outFolder + newFilesName + fileNumberString.str() + ".txt";
							 outDicomBin = outFolder + newFilesName + fileNumberString.str() + ".bin";
							 dmc2Raw(dicomPath,outDicomAscii, outDicomBin);//IMAGE 2 RAW DATA ASCII FORMAT
							 outDicomPathAnonymized = outFolder + newFilesName + fileNumberString.str() + ".dcm";
							 anonymize(dicomPath, outDicomPathAnonymized, removePData, removeIData); //ANONYMIZATION
							 fileNumber++;

							 logFile << dicomPath << "\t , \t" << outDicomAscii.substr(0,outDicomAscii.size()-4) + ".[txt|bin|dcm]" << endl;

						 }
					 }
				 closedir (dir);
				 logFile.close();
            printf ("\n\n Task Completed \n\n");
				 }
		 }else{
			 show_usage(argv[0]);
		 }


	 return 0;


   return 0;
}




