#include "bf.h"
typedef struct Group{
  BF_Block *Block;
  char *data;
  int block_index;
  int index;
  int ending_block_index;
} Group;
int Is_More_ThanValues(void* ,void* ,int );

//void Copy_File(const char*,const char*);
SR_ErrorCode Copy_File(const char*,const char*);
BF_Block** Create_BFBlock_Array(int,int ,int );


int Is_More_ThanRecords(char* ,char* ,int );


char* Get_Record(BF_Block** ,int ,int );

void Swap(void*,void* );


int Partition(BF_Block** ,int  ,int ,int );

void  Quiksort(BF_Block** ,int  ,int ,int );

//MERGE Sort_Methods
SR_ErrorCode Merge_Sort(int bufferSize, int number_of_blocks, int fieldNo, const char *output_filename);
Group *GroupArrayInit(int fileDesc, int bufferSize, int valleysize, int* size,int* starting_block);
void findMin(Group *GroupArray,int fieldNo, int *size);
