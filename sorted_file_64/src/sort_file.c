#include "sort_file.h"
#include "bf.h"
#include "Sort_Methods.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int Heap_id=1234321;


SR_ErrorCode SR_Init() {
  // Your code goes here

  return SR_OK;
}
///////////////////////////////////////////////////////////
SR_ErrorCode SR_CreateFile(const char *fileName) {
    if(BF_CreateFile(fileName)==BF_OK)
  	{
  		int fd;
  		if(BF_OpenFile(fileName,&fd)==BF_OK)
  		{
			BF_Block *block;
			BF_Block_Init(&block);
  			if(BF_AllocateBlock(fd,block)==BF_OK)
  			{
  				char* data;
  				data=BF_Block_GetData(block);
  				memcpy(data,&Heap_id,4);
  				BF_Block_SetDirty(block);
  				if(BF_UnpinBlock(block)!=BF_OK){return SR_ERROR;}
  				if(BF_CloseFile(fd)!=BF_OK){return SR_ERROR;}
  				BF_Block_Destroy(&block);
  				return SR_OK;
  			}
  			return SR_ERROR;
  		}
  		return SR_ERROR;
  	}
  	return SR_ERROR;
}
///////////////////////////////////////////////////////////
SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
    static unsigned int open_files=0;
	int heap_fileid;


	if(BF_OpenFile(fileName,fileDesc)==BF_OK && open_files<=BF_MAX_OPEN_FILES)
	{
			BF_Block *block;
			BF_Block_Init(&block);

			if(BF_GetBlock(*fileDesc,0,block)==BF_OK)
			{
					char* data;

					data=BF_Block_GetData(block);
					memcpy(&heap_fileid,data,4);
					if(heap_fileid==Heap_id)
					{
							open_files++;
							BF_UnpinBlock(block);
							BF_Block_Destroy(&block);
							return SR_OK;
					}
					return SR_ERROR;
			}
			 return SR_ERROR;
	}
	return SR_ERROR;
}
//////////////////////////////////////////////////////////
SR_ErrorCode SR_CloseFile(int fileDesc) {
    if(BF_CloseFile(fileDesc)!=BF_OK){return SR_ERROR;}
    return SR_OK;
}

/////////////////////////////////////////////////////////


SR_ErrorCode SR_InsertEntry(int fileDesc,	Record record) {
    char *data;
    int maxcap, counter;
    int *records_ptr/**maxcap_ptr*/;
    BF_Block *block;
    BF_Block_Init(&block);
    //
    //BF_GetBlock(fileDesc, 0, block);
    //data=BF_Block_GetData(block);
    //
    //maxcap_ptr=(int*)data;
    maxcap=(BF_BLOCK_SIZE-4)/sizeof(Record);

    if(BF_GetBlockCounter(fileDesc, &counter)!=BF_OK){
      BF_Block_Destroy(&block);
      return SR_ERROR;
    }
    if(BF_GetBlock(fileDesc, counter-1, block)!=BF_OK){
      BF_Block_Destroy(&block);
      return SR_ERROR;
    }

    data=BF_Block_GetData(block);

    records_ptr=(int*)data;

    if(counter>1 && maxcap>(*records_ptr)){
       memcpy(data+sizeof(int)+sizeof(Record)*(*records_ptr),&record,sizeof(Record));
       (*records_ptr)++;
       BF_Block_SetDirty(block);
       if(BF_UnpinBlock(block)!=BF_OK){
         BF_Block_Destroy(&block);
         return SR_ERROR;
       }
    }
    else{
       BF_UnpinBlock(block);
       if(BF_AllocateBlock(fileDesc, block)!=BF_OK){
         BF_Block_Destroy(&block);
         return SR_ERROR;
       }
       data=BF_Block_GetData(block);
       records_ptr=(int*)data;

       memcpy(data+sizeof(int),&record,sizeof(Record));
       *records_ptr=1;
       BF_Block_SetDirty(block);
       if(BF_UnpinBlock(block)!=BF_OK){
         BF_Block_Destroy(&block);
         return SR_ERROR;
       }
    }
    BF_Block_Destroy(&block);
    return SR_OK;
}
/////////////////////////////////////////////////////////////////////////////////
SR_ErrorCode SR_SortedFile(const char* input_filename,const char* output_filename,int fieldNo,int bufferSize) {
//First Step

  int blockindex;
  int number_of_blocks=0,fileDesc,number_of_records,blocksinbuffer;
  BF_Block** BF_BlockArray,*Block;
  char* data;
  int records_in_block=(BF_BLOCK_SIZE-4)/sizeof(Record);

  if(BF_CreateFile("tmp_sort.db")!=BF_OK){return SR_ERROR;}                         //Create tmpfile
  if(Copy_File("tmp_sort.db",input_filename)!=SR_OK){return SR_ERROR;}              //Copy contents from unsorted

  if(BF_OpenFile("tmp_sort.db",&fileDesc)!=BF_OK){return SR_ERROR;}

  if(BF_GetBlockCounter(fileDesc,&number_of_blocks)!=BF_OK){return SR_ERROR;}


  //for every buffer sized blocks do quiksort

  if(bufferSize>number_of_blocks)                                                   //Case number_of_blocks in buffer more than blocks in file
  {
      bufferSize=number_of_blocks;
  }
  blocksinbuffer=bufferSize;
  //for every buffer sized blocks do quiksort
  for(blockindex=0;blockindex<=number_of_blocks-1;blockindex+=blocksinbuffer)
  {
      //Create BF_Block Array ,calculate number of records in every quiksort and buffersize

    if(number_of_blocks-blockindex-1<bufferSize)                                      //if there are remaining blocks
    {
        int remaining_blocks=number_of_blocks-blockindex;
        int lastblocksize;

        blocksinbuffer=remaining_blocks;
        BF_BlockArray=Create_BFBlock_Array(remaining_blocks,fileDesc,blockindex);
        BF_Block_Init(&Block);                                                      //add size of last block
        if(BF_GetBlock(fileDesc,number_of_blocks-1,Block)!=BF_OK){return SR_ERROR;}
        data=BF_Block_GetData(Block);
        memcpy(&lastblocksize,data,sizeof(int));
        if(BF_UnpinBlock(Block)!=BF_OK){return SR_ERROR;}
        BF_Block_Destroy(&Block);
        number_of_records=records_in_block*(remaining_blocks-1)+lastblocksize;
    }
    else                                                                                //if there are no remaining blocks
    {
        blocksinbuffer=bufferSize;
        BF_BlockArray=Create_BFBlock_Array(bufferSize,fileDesc,blockindex);
        number_of_records=records_in_block*bufferSize;
    }
    //Do Quiksort
    Quiksort(BF_BlockArray,0,number_of_records-1,fieldNo);

    for(int i=0;i<=blocksinbuffer-1;i++)                                                //free buffer
    {
        BF_Block_SetDirty(BF_BlockArray[i]);
        if(BF_UnpinBlock(BF_BlockArray[i])!=BF_OK){return SR_ERROR;}
        BF_Block_Destroy(&BF_BlockArray[i]);
    }
    free(BF_BlockArray);
  }
  if(BF_CloseFile(fileDesc)!=BF_OK){return SR_ERROR;}

///////////////////////////////////////////////////////////////
//Step 2. Merge sort everything
  return Merge_Sort(bufferSize,number_of_blocks,fieldNo,output_filename);
}
/////////////////////////////////////////////////////////////////////////////////////////////

SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
  char *data;
  void *tmp_ptr;
  int i,j,counter;
  int *records_ptr;
  Record *record;
  BF_Block *block;
  BF_Block_Init(&block);
  if(BF_GetBlockCounter(fileDesc, &counter)!=BF_OK){
    BF_Block_Destroy(&block);
    return SR_ERROR;
  }
  for(i=1;i<counter;i++){
    if(BF_GetBlock(fileDesc, i, block)!=BF_OK){
      BF_Block_Destroy(&block);
      return SR_ERROR;
    }
    data=BF_Block_GetData(block);

    records_ptr=(int*)data;

    for(j=0;j<(*records_ptr);j++){
      tmp_ptr=data+sizeof(int)+j*sizeof(Record);
      record=tmp_ptr;
      printf("%d,\"%s\",\"%s\",\"%s\"\n",
      	record->id, record->name, record->surname, record->city);
    }
    if(BF_UnpinBlock(block)!=BF_OK){
      BF_Block_Destroy(&block);
      return SR_ERROR;
    }
  }
  BF_Block_Destroy(&block);
  return SR_OK;
}
