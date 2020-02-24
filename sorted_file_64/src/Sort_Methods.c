#include "bf.h"
#include "sort_file.h"
#include "Sort_Methods.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int records_in_block=(BF_BLOCK_SIZE-4)/sizeof(Record);

int Is_More_ThanValues(void* value1,void* value2,int datatype) //Compare values
{
    if(datatype==0)
    {
        int a,b;
        a=*(int*)value1;
        b=*(int*)value2;
        return a>b;
    }
    else
    {
        return (strcmp((char*)value1,(char*)value2)>0);

    }
}

BF_Block** Create_BFBlock_Array(int bufferSize,int fileDesc,int block_index) //Creae arreay of bF_block*
{
    BF_Block** BF_BlockArray;

    BF_BlockArray=malloc(bufferSize*sizeof(BF_Block*));
    for(int i=0;i< bufferSize;i++)
    {
        BF_Block_Init(&BF_BlockArray[i]);
        if(BF_GetBlock(fileDesc,i+block_index,BF_BlockArray[i])!=BF_OK){exit(1);}

    }
    return BF_BlockArray;

}


int Is_More_ThanRecords(char* Record1,char* Record2,int datatype)   //Compare records based on datatype
{
    Record R1,R2;
    int offset=0;

    switch (datatype) {
        case 0:                                                   //for id
        offset=offset+0;
        memcpy(&R1.id,&Record1[offset],sizeof(R1.id));
        memcpy(&R2.id,&Record2[offset],sizeof(R2.id));
        return  Is_More_ThanValues(&R1.id,&R2.id,datatype);
        case 1:                                                     //for name
        offset=offset+sizeof(R1.id);
        memcpy(&R1.name,&Record1[offset],sizeof(R1.name));
        memcpy(&R2.name,&Record2[offset],sizeof(R2.name));
        return  Is_More_ThanValues(&R1.name,&R2.name,datatype);
        case 2:                                                     //for surname
        offset=offset+sizeof(R1.id)+sizeof(R1.name);
        memcpy(&R1.surname,&Record1[offset],sizeof(R1.surname));
        memcpy(&R2.surname,&Record2[offset],sizeof(R2.surname));
        return  Is_More_ThanValues(&R1.surname,&R2.surname,datatype);
        case 3:                                                     //fro city
        offset=offset+sizeof(R1.id)+sizeof(R1.name)+sizeof(R1.surname);
        memcpy(&R1.city,&Record1[offset],sizeof(R1.city));
        memcpy(&R2.city,&Record2[offset],sizeof(R2.city));
        return  Is_More_ThanValues(&R1.city,&R2.city,datatype);
    }

    return 0;
}

SR_ErrorCode Copy_File(const char* target_file,const char* source_file)  //Copy from source_file to target_file
{
    char* data_source,*data_target;
    int fileDesc_source,fileDesc_target,number_of_blocks;

   if(BF_OpenFile(source_file,&fileDesc_source)!=BF_OK){return SR_ERROR;}
   if(BF_OpenFile(target_file,&fileDesc_target)!=BF_OK){return SR_ERROR;}

   if(BF_GetBlockCounter(fileDesc_source,&number_of_blocks)!=BF_OK){return SR_ERROR;}

   BF_Block* target_Block,*source_Block;
   BF_Block_Init(&target_Block);
   BF_Block_Init(&source_Block);

   for(int i=1;i<number_of_blocks;i++)
   {
      if(BF_GetBlock(fileDesc_source,i,source_Block)!=BF_OK){return SR_ERROR;}
      if(BF_AllocateBlock(fileDesc_target,target_Block)!=BF_OK){return SR_ERROR;}

       data_source=BF_Block_GetData(source_Block);
       data_target=BF_Block_GetData(target_Block);

       memcpy(data_target,data_source,BF_BLOCK_SIZE);

       BF_Block_SetDirty(target_Block);
       if(BF_UnpinBlock(source_Block)!=BF_OK){return SR_ERROR;}
       if(BF_UnpinBlock(target_Block)!=BF_OK){return SR_ERROR;}

   }

   BF_Block_Destroy(&target_Block);
   BF_Block_Destroy(&source_Block);

   if(BF_CloseFile(fileDesc_target)!=BF_OK){return SR_ERROR;}
   if(BF_CloseFile(fileDesc_source)!=BF_OK){return SR_ERROR;}

   return SR_OK;
}
char* Get_Record(BF_Block** Array,int i,int datatype)
{
    char* data;
    BF_Block* Block;
    int blockindex,offset,record_index;


    blockindex=i/records_in_block;              //get block with i-th record
    Block=Array[blockindex];
    data=BF_Block_GetData(Block);
    record_index=i%records_in_block;        //get i-th record
    offset=4+record_index*sizeof(Record);

    return &data[offset];
}
void Swap(void* Record1,void* Record2)
{
    void* tmp;


    tmp=malloc(sizeof(Record));
    memcpy(tmp,Record1,sizeof(Record));
    memcpy(Record1,Record2,sizeof(Record));
    memcpy(Record2,tmp,sizeof(Record));

    free(tmp);
}

int Partition(BF_Block** Array,int start ,int end,int datatype)
{
    char *record1,*record2,*record3,*pivot;
    int index,j;

    pivot=Get_Record(Array,end,datatype);
    index=start-1;
    for(j=start;j<=end-1;j++)
    {
        record1=Get_Record(Array,j,datatype);
        if(!Is_More_ThanRecords(record1,pivot,datatype))
        {
            index=index+1;
            record2=Get_Record(Array,index,datatype);

            Swap(record1,record2);

        }
    }

    record3=Get_Record(Array,index+1,datatype);\
    Swap(record3,pivot);

    return index+1;

}

void  Quiksort(BF_Block** Array,int start ,int end,int datatype)
{
    int index;

    if(start<end)
    {
        index=Partition(Array,start,end,datatype);
        Quiksort(Array,start,index-1,datatype);
        Quiksort(Array,index+1,end,datatype);
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////






//MERGE Sort_Methods

SR_ErrorCode Merge_Sort(int bufferSize, int number_of_blocks, int fieldNo, const char *output_filename){
  if(bufferSize<3)    //If buffer is too small for merge sort, preemptively terminate the function.
    return SR_ERROR;
  int GroupSize=bufferSize, RfileDesc, WfileDesc;
  Group *GroupArray;
  Record *tmp_record;
  int size=0;
  int starting_block=0;

  do{                 //first loop; ends when the file has been sorted

    BF_OpenFile("tmp_sort.db",&RfileDesc);
    remove("tmp_sort2.db");
    SR_CreateFile("tmp_sort2.db");
    SR_OpenFile("tmp_sort2.db",&WfileDesc);
    starting_block=0;
    while((GroupArray=GroupArrayInit(RfileDesc,bufferSize-1,GroupSize,&size,&starting_block))!=NULL){  //second loop; ends when the merge sort algorithm passed once the entire file
      findMin(GroupArray,fieldNo,&size);
      while(GroupArray[0].data!=NULL){  //third loop; ends when *bufferSize-1* groups have been merge=-orted
        if(GroupArray[0].index+sizeof(Record)<=BF_BLOCK_SIZE && ((GroupArray[0].index))/sizeof(Record)<*(int*)GroupArray[0].data){    //check if index "pointer" reached the end of a block
          tmp_record=(Record*)(GroupArray[0].data+GroupArray[0].index);
          SR_InsertEntry(WfileDesc, *tmp_record); //Insert Entry uses only one block, so buffer restrictions are not violated
          GroupArray[0].index+=sizeof(Record);
        }
        if(!(GroupArray[0].index+sizeof(Record)<=BF_BLOCK_SIZE && ((GroupArray[0].index))/sizeof(Record)<*(int*)GroupArray[0].data)){//check again so that the next loop iteration goes smoothly
          GroupArray[0].block_index++;
          if(GroupArray[0].block_index<(number_of_blocks) && GroupArray[0].block_index<GroupArray[0].ending_block_index){ //check if the block_index reached the end of life or end of the Group
            BF_UnpinBlock(GroupArray[0].Block);
            BF_GetBlock(RfileDesc,GroupArray[0].block_index,GroupArray[0].Block);
            GroupArray[0].data=BF_Block_GetData(GroupArray[0].Block);
            GroupArray[0].index=sizeof(int);
          }
          else{
            BF_UnpinBlock(GroupArray[0].Block);
            BF_Block_Destroy(&GroupArray[0].Block);
            GroupArray[0].data=NULL;
          }
        }
        findMin(GroupArray,fieldNo,&size);
      }

      free(GroupArray);
    }

    BF_CloseFile(RfileDesc);
    remove("tmp_sort.db");

    Copy_File("tmp_sort.db","tmp_sort2.db");
    SR_CloseFile(WfileDesc);

  }while(GroupSize<(number_of_blocks) && ((GroupSize*=bufferSize-1)));
  SR_CreateFile(output_filename);
  Copy_File(output_filename,"tmp_sort2.db");
  remove("tmp_sort.db");
  remove("tmp_sort2.db");
  return SR_OK;
}

Group *GroupArrayInit(int fileDesc, int bufferSize, int valleysize, int* size, int *starting_block){
  Group* GroupArray=malloc(sizeof(Group)*bufferSize);
  int i;
  int block_index;
  for(i=0;i<bufferSize;i++){
    BF_Block_Init(&GroupArray[i].Block);
    BF_GetBlockCounter(fileDesc,&GroupArray[i].block_index);
    block_index=i*valleysize+(*starting_block);
    if(block_index>=GroupArray[i].block_index){
      BF_Block_Destroy(&GroupArray[i].Block);
      break;
    }
//    BF_GetBlock(fileDesc,i*valleysize+(*starting_block),GroupArray[i].Block);
//    for(int j=0;j<GroupArray[i].block_index-1;j++){
//      BF_GetBlock(fileDesc,j,GroupArray[i].Block);
//    }
    BF_GetBlock(fileDesc,block_index,GroupArray[i].Block);
    //
    GroupArray[i].block_index=block_index;
    GroupArray[i].data=BF_Block_GetData(GroupArray[i].Block);
    GroupArray[i].index=sizeof(int);
    GroupArray[i].ending_block_index=block_index+valleysize;
  }

  *starting_block=GroupArray[(i-1)*(i>0)].ending_block_index;
  *size=i;
  if((*size)<=0){
    return NULL;
  }
  return GroupArray;
}

void findMin(Group *GroupArray,int fieldNo,int *size){
  Group tmp_Group;
  if(*size<=1)
    return;
  if(GroupArray[0].data==NULL){
    GroupArray[0]=GroupArray[*size-1];
    *size=*size-1;
  }
  for(int i=1;i<*size;i++){
    if(Is_More_ThanRecords(GroupArray[0].data+GroupArray[0].index, GroupArray[i].data+GroupArray[i].index,fieldNo)){
      tmp_Group=GroupArray[0];
      GroupArray[0]=GroupArray[i];
      GroupArray[i]=tmp_Group;
    }
  }
}
