/* Copyright 2003,2004 Matt Flax <flatmax,:AT:,ieee.org>
   This file is part of the MFFM Bit Stream library

   MFFM Bit Stream library is free software; you can 
   redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   MFFM Bit Stream library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You have received a copy of the GNU General Public License
   along with the MFFM Bit Stream library
*/

#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

//Use this to output debug info.
//#define DEBUG_BSB

/** This class is common to bit stream reading and writing.
    Paradigm of this system:
    data is the whole data pointer.
    data1 points to the LSB data bits
    data2 points to the MSB data bits
    data1 is written/read to/from file when required
    The bits are extraced from the 'data' holder by masking it.
    Bit count returned is changed using 'resetBitCnt(new bit count)'.
*/

///These definitions are for a 32 bit architecture
#define DATA_TYPE int
#define HALF_DATA_TYPE short int
// Define 16,64 and other architecture dependant type here instead of the
// default 32 bit structures.

class BitStreamBase {
protected:
  DATA_TYPE data; ///The real data holder
  HALF_DATA_TYPE *data1, *data2; ///Points to raw stream data
  DATA_TYPE mask; ///Masks the raw stram data to gen. the correct bit cnt.

  int bitCnt; ///The number of bits we are currently working with
  int maskLoc; ///mask location in the data
  long dataLoc; ///data location in the stream
  ///The number of bits which the data can hold.
  const static int dataBitCnt=sizeof(DATA_TYPE)*8;
  ///The number of bytes which the data can hold.
  const static int dataByteCnt=sizeof(DATA_TYPE);
  ///The number of bits which the half data (data1&data2) can hold.
  const static int halfDataBitCnt=sizeof(HALF_DATA_TYPE)*8;
  ///The number of bytes which the half data (data1&data2) can hold.
  const static int halfDataByteCnt=sizeof(HALF_DATA_TYPE);

  int file; ///The file descriptor we are working with

  /** This function is used to regenerate the proper mask size and location
   */
  inline void regenMask(void){
    mask=0x1;
    for (int i=0;i<bitCnt-1;i++){
      mask=mask<<1;
      mask|=0x1;
    }
    //printf("BitStreamBase::resetBitCnt: %d  mask: 0x%x maskLoc: 0x%x\n",bitCnt,mask,maskLoc);
    mask=mask<<maskLoc;
  }

public:

  /** Constructor:
      Takes the bit count we are working with.
  */
  BitStreamBase(int cnt=1){
#ifdef DEBUG_BSB
    cout<<"BitStreamBase::BitStreamBase"<<endl;
#endif
    if (dataBitCnt!=(2*halfDataBitCnt)){
      cerr<<"BitStreamBase::BitStreamBase : \t\theader file #define ERROR\n";
      cerr<<"BitStreamBase::BitStreamBase : \n\tMake sure sizeof(DATA_TYPE)=2*sizeof(HALF_DATA_TYPE)"<<endl;
      exit(-1);
    }
    //cout<<"Data width = "<<dataBitCnt<<" bits"<<endl;
    //cout<<"Half data width = "<<halfDataBitCnt<<" bits"<<endl;

    //Set up the data pointers
    data1=(HALF_DATA_TYPE*)&data;
    data2=data1+1;//this is actually one int shift, not data1+sizeof(data)/2
    //    cout<<"data mem loc "<<&data<<" data1 mem loc "<<data1<<" data2 mem loc "<<data2<<endl;
    //Set the bit count
    maskLoc=0;
    data=0;
    resetBitCnt(cnt);
  }

  /**Overload the read function for new classes which require read
     operations. See bitStreamRead for examples.
  */
  virtual int read(void){return 0;}
  /**Overload the write function for new classes which require write
     operations. See bitStreamWrite for examples.
  */
  virtual int write(void){return 0;}

  /** resets the mask to output/input 'cnt' bits
      returns <0 on error.
      returns -1 for a count too small
      returns -2 for a count too large
  */
  int resetBitCnt(int cnt){
#ifdef DEBUG_BSB
    cout<<"BitStreamBase::resetBitCnt"<<endl;
#endif
    if (cnt<1) return -1;
    if (cnt>halfDataBitCnt) return -2;
    if (bitCnt==cnt) return cnt;

    //Case a] the requested bit cnt is larger then the end of the data
    if ((maskLoc+bitCnt)>dataBitCnt){ //must read or write 
      // In the case of a writing class, an empty read function is called (see above)
      this->read(); //this is called for reading classes
      // In the case of a reading class, an empty write function is called (see above)
      this->write(); //this is called for writing classes
    }

    bitCnt=cnt;

    regenMask();//reconstruct the mask

    //printf("BitStreamBase::resetBitCnt: %d  mask: 0x%x maskLoc: 0x%x\n",bitCnt,mask);
    return bitCnt;
  }

  ///This returns the data bit location indexed from the beginning of the stream
  long getDataLoc(void){
    return dataLoc;
  }

  ///Returns the maximum number of bits which may be requested in resetBitCnt
  int getMaxBitCnt(void){
    return halfDataBitCnt;
  }
};

class BitStreamRead : public BitStreamBase {
  /** This private function controls reading from disk
   */
  virtual int read(void){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::read : enter "<<endl;
#endif
    //cout<<"maskloc, databitCnt"<<maskLoc<<", "<<dataBitCnt<<endl;
    //shift the mask, maskIndex and data back, inc the data bit index
    //printf("mask b4 : %x\n",mask);
    maskLoc-=halfDataBitCnt; //shift mask loc
    regenMask(); // regenerate it incase
    //printf("maskAfter b4 : %x\n",mask);

    //mask=mask>>halfDataBitCnt;
    data=data>>halfDataBitCnt;
    dataLoc+=halfDataBitCnt;

    int ret=::read(file, data2, halfDataByteCnt);
    //cout<<"contOK, ret"<<contOK<<'\t'<<ret<<endl;
    if (ret!=halfDataByteCnt){//either at end of file or error
      if (ret<0)
	cerr<<"BitStreamRead::read : couldn't read data from file."<<endl;
      else
	if (contOK==ret){
	  cerr<<"BitStreamRead::read EOF reached. Indicating"<<endl;
	  contOK-=ret;
	}
      
    } else
      contOK-=ret;
    //  printf("data: 0x%x\n",data);
    return ret;
  }

  ///Fills the data buffer on opening
  int firstRead(void){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::firstRead"<<endl;
#endif
    int ret=::read(file, &data, dataByteCnt);
    if (ret!=dataByteCnt)
      cerr<<"BitStreamRead::firstRead : couldn't read data from file."<<endl;
    //printf("data: 0x%x\n",data);
    contOK-=ret;
    return ret;
  }

  /** returns the file poter on success, <0 otherwise
   */
  int open(char *fileName){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::open"<<endl;
#endif
    file=-1;
    if ((file=::open(fileName,O_RDONLY))<0)
      cerr<<"BitStreamRead::open : file open \"" <<fileName<<"\" for reading fail"<<endl;
    else {
      FILE *ftemp=fopen(fileName, "r");
      //get to the end of the file
      if (fseek(ftemp, 0, SEEK_END)<0)
	cerr<<"BitStreamRead::open : end of file seek fail."<<endl;
      contOKOrig=contOK=(int)ftell(ftemp);
      rewind(ftemp);
      fclose(ftemp);
    }
    return file;
  }

  /** Member to indicate that the stream has run out of bits.
      actually is the byte count of the file. contOKOrig is
      left unaltered so that the byte count may be remembered.
  */
  int contOK,contOKOrig;

public:

  /**Constructor:
     Takes the file name (fName) and the initial bit count to operate
     with. This bit count will be the number of bits returned from the
     bit stream with each 'get' function call.
  */
  BitStreamRead(char *fName, int cnt) : BitStreamBase(cnt){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::BitStreamRead"<<endl;
#endif
    dataLoc=contOK=0;//Initialised correctly in open.
    open(fName);
    firstRead();//ensure the data buffer is full
  }

  ///Destructor
  ~BitStreamRead(void){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::~BitStreamRead"<<endl;
#endif
    close(file);
  }

  /** This will get data from data stream.
      It will also shift the mask to the next location.
  */
  DATA_TYPE get(void){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::get"<<endl;
#endif
    //Check for space to get
    if ((dataBitCnt-maskLoc)<bitCnt) // no space so ...
      read();//read half data from the file and shift data
    DATA_TYPE readData=(data&mask)>>maskLoc;
    //printf("mask: 0x%x\n",mask);
    mask=mask<<bitCnt;    //progress the mask
    //printf("mask: 0x%x\n",mask);
    //Shift maskLoc index on and Check not past the end
    if ((maskLoc+=bitCnt)>dataBitCnt)//if past the end then ... 
      read();//read half data from the file and shift data
    return readData;
  }


  /** Notifies the user to continue. If the stream is finished, clears out the
      bit buffer.
      returns : <0 if the stream isn't complete
                int>=0 if the stream is finished and there are int bits left
                       in the buffer
  */
  int lastBitCnt(void){
#ifdef DEBUG_BSR
    cout<<"BitStreamRead::lastBitCnt"<<endl;
#endif
    int bitsLeft=-1;
    //cout<<contOK<<endl;
    if (contOK){//not ready to finish
      //cerr<<"BitStreamRead::lastBitCnt : More bytes to read from stream"<<endl;
    } else {
      cout<<8*contOKOrig<<'\t'<<dataLoc<<'\t'<<maskLoc<<endl;
      bitsLeft=8*contOKOrig-(dataLoc+maskLoc);
      //      dataBitCnt      
    }
    return bitsLeft;
  }

  /** Tells the user how many bits total in the file.
   */
  int totalBitCnt(void){return contOKOrig*8;}
};

class BitStreamWrite : public BitStreamBase {

  virtual int write(void){
#ifdef DEBUG_BSW
    cout<<"BitStreamWrite::write : enter "<<endl;
#endif
    int ret=::write(file, data1, halfDataByteCnt);
    if (ret!=halfDataByteCnt){
      cerr<<"BitStreamWrite::write : couldn't write data to file."<<endl;
    }
    //shift the mask back
    mask=mask>>halfDataBitCnt;
    maskLoc-=halfDataBitCnt;
    //shift the data back
    data=data>>halfDataBitCnt;
    dataLoc+=halfDataBitCnt;
	  
	return ret;
  }

  int lastWrite(void){
#ifdef DEBUG_BSW
    cout<<"BitStreamWrite::lastWrite"<<endl;
#endif
    //buffer the last bits of the last byte
    resetBitCnt(maskLoc%8); put(0);
    //Write the remaining bytes to file
    size_t bytes=maskLoc/8;
    //    cout<<"bytes left : "<<bytes<<endl;
    int ret=::write(file, &data, bytes);
    if (ret!=(int)bytes)
      cerr<<"BitStreamWrite::lastWrite : couldn't write data to file."<<endl;
	  
	return ret;
  }

public:
  ///Constructor
  BitStreamWrite(char *fName, int cnt) : BitStreamBase(cnt){
#ifdef DEBUG_BSW
    cout<<"BitStreamWrite::BitStreamWrite"<<endl;
#endif
    open(fName);
  }

  ///Destructor
  ~BitStreamWrite(void){
#ifdef DEBUG_BSW
    cout<<"BitStreamWrite::~BitStreamWrite"<<endl;
#endif
    lastWrite(); // flush the data buffer
    close(file);
  }

  /** returns the file poter on success, <0 otherwise
   */
  int open(char *fileName){
#ifdef DEBUG_BSW
    cout<<"BitStreamWrite::open"<<endl;
#endif
    file=-1;
    if ((file=::open(fileName,O_WRONLY|O_CREAT,0666))<0)
      cerr<<"BitStreamBase::open : file open \"" <<fileName<<"\" for writing fail"<<endl;
    return file;
  }

  /** This will put from 'dataToPut' to the data stream.
      It will also shift the mask to the next free location
  */
  DATA_TYPE put(DATA_TYPE dataToPut){
    //Check for space to put
    if ((dataBitCnt-maskLoc)<bitCnt){
      write();//write half data to file and shift data
    }
    //printf("dataToPut : 0x%x\t",dataToPut);
    //printf("data 0x%x : mask 0x%x\n",data,mask);
    data|=(dataToPut<<maskLoc)&mask;
    if ((maskLoc+2*bitCnt)>dataBitCnt){//have to write to file
      write();
    }
    //shift the mask on
    maskLoc+=bitCnt;
    mask=mask<<bitCnt;
    //printf("data 0x%x : mask 0x%x\n",data,mask);
    //printf("data1 0x%x : data2 0x%x\n",*data1,*data2);
	  
	return file;
  }
};
#endif