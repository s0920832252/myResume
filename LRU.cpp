#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

//page size is 32byte
#define PAGESIZE 32
// 32KB in shared memory
#define PHYSICAL_MEM_SIZE 32768
//128KB in global
#define STORAGE_SIZE 131072
//#define MEMORY_SEGMENT 32768

#define DATAFILE "./data.bin"
#define OUTFILE "./snapshot.bin"
//#define INVALID 0xFFFFFFFF

//#define __LOCAK(); for(int p=0;p<4;P++){if(threadIdx.x==p){
//#define __UNLOCAK(); } __syncthreads();}

//#define __GET_BASE()  p*MEMORY_SEGMENT

typedef unsigned char uchar;
typedef uint32_t u32;
typedef uint8_t u8;

//page table entries
__device__ __managed__ int PAGE_ENTRIES;
//count the pagefault times
__device__ __managed__ int PAGEFAULT;



//secondary memory
//__device__ __managed__ uchar storage[STORAGE_SIZE];
// data input and output
__device__ __managed__ uchar results[STORAGE_SIZE];
__device__ __managed__ uchar input[STORAGE_SIZE];

//page table
extern __shared__ u32 pt[]; //程式128KB / pagesize 32byte =4096個entry
extern __shared__ u8 ptm[];

int load_binaryFile (char* datafile, uchar input[], int storage_size)
{ FILE *fh_d = fopen(datafile,"rb"); /* open as binary */
  if (fh_d == NULL)
  { 
    fprintf(stderr, "Illegal testcase: Cannot open \"data.bin\"\n");
    exit(2);
	}
  fseek(fh_d, 0L, SEEK_END);
  int fileSize_d = ftell(fh_d);    /* find where it is */
  fseek(fh_d, 0L, SEEK_SET); /* go to the beginning */
  
  if(fileSize_d>131072)
    fileSize_d=131072;

  /*size_t mem_code_d = malloc(fileSize_d);
  if (mem_code_d == NULL )
  {
    fprintf(stderr, "Cannot allocate dynamic memory\n");
    exit(3);
  }*/
  
  fread(input,fileSize_d,1, fh_d);
  fclose(fh_d);
  return fileSize_d; 
}

void write_binaryFile(char* outfile,uchar results[],int input_size)
{ FILE *fh_d = fopen(outfile,"wb"); /* open as binary */
  /*if (fh_d == NULL)
  { 
    fprintf(stderr, "Illegal testcase: Cannot open \"snapshot.bin\"\n");
    exit(2);
  }*/

  fwrite(results,input_size,1,fh_d);  
   
  fclose(fh_d); 
}

__device__ void init_pageTable(int pt_entries)
{ 
  for(int i=0;i<pt_entries;i++)
  { 
    pt[i]=10000;           //每個pagetable entry 初始化為5000  使用STACK法來實作LRU  反轉分業表
  }
 
}

__device__ int paging(uchar *buffer,u32 page_num,u32 offset)  //使用分業表
{ int index=0;
  int index2=0;
  int frame_num=page_num%1024;
  for(index=0;index<1024;index++)
  {
    if(pt[index]!=10000)
    {
      index2++;
    }
    if(pt[index]==page_num)
    {
      break;
    }
  }

  if(index==1024) //不再記憶體內 PAGEFAULT
  { int temp1=pt[1023]; 
    PAGEFAULT++;
    for(int i=index2-1;i>=0;i--)
    {
      pt[i+1]=pt[i];
    }
    pt[0]=page_num;

    if(index2==1024)//記憶體滿
    { for(int i=0;i<32;i++)
       input[temp1*PAGESIZE+i]=buffer[(temp1%1024)*PAGESIZE+i];
      for(int i=0;i<32;i++)
       buffer[frame_num*PAGESIZE+i]=input[page_num*PAGESIZE+i]; //把disk 一個page搬出來到memory 
      return frame_num*PAGESIZE+offset;
    }
    else //記憶體未滿
    { for(int i=0;i<32;i++)
        buffer[frame_num*PAGESIZE+i]=input[page_num*PAGESIZE+i]; //把disk 一個page搬出來到memory 
      return frame_num*PAGESIZE+offset;
    }
  }
  else  //PAGE HIT
  {
    int temp2=pt[index];
    for(int i=index-1;i>=0;i++)
    {
      pt[i+1]=pt[i];
    }
    pt[0]=temp2;
    
    return frame_num*PAGESIZE+offset;
  }
}

__device__ uchar Gread(uchar *buffer, u32 addr)
{ u32 page_num =addr/PAGESIZE;
  u32 offset    =addr%PAGESIZE;
  addr=paging(buffer,page_num,offset);
  return buffer[addr];

}

__device__ void Gwrite(uchar *buffer,u32 addr , uchar value)
{ u32 page_num = addr/PAGESIZE;
  u32 offset    = addr%PAGESIZE;
  
  addr = paging(buffer,page_num,offset);
  buffer[addr]=value;

}



__device__ void snapshot(uchar *results,uchar* buffer,int offset, int input_size)
{ for(int i=0;i<input_size;i++)
   results[i]=Gread(buffer,i+offset);
}

__global__ void mykernel(int input_size)
{   //take shared memory as physical memory 
	__shared__ uchar data[PHYSICAL_MEM_SIZE];
	// get page table entries
	int pt_entries =PHYSICAL_MEM_SIZE/PAGESIZE;
    
  //before first Gwrite or Gread 
	init_pageTable(pt_entries);
  //init PAGEFAULT
  PAGEFAULT=0;
	//####Gwrite/Gread code section start####
	for(int i=0;i<input_size;i++)
		Gwrite(data,i,input[i]);
	for(int i=input_size-1;i>=input_size-10;i--)
	  	int value=Gread(data,i);
    
    //the last line of Gwrite/Gread code section should be snapshot()
    snapshot(results,data,0,input_size); 
    //####Gwrite/Gread code section end#### 
  
  
  printf("pagefault times=%d\n",PAGEFAULT);
}



int main()
{ int input_size=load_binaryFile((char*)DATAFILE,input,STORAGE_SIZE);
   printf("fileSize = %d\n",input_size); 
  cudaSetDevice(0);  

  mykernel<<<1,1,16384>>>(input_size);
  cudaDeviceSynchronize();
  cudaDeviceReset();

  write_binaryFile((char*)OUTFILE,results,input_size);
  printf("pagefault times=%d\n",PAGEFAULT);


  return 0;
}