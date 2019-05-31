
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define INT_MAX 2147483647

typedef struct Cache{
    int * cache;
    int * priority;
    int * dirty;
}Cache;

Cache *c;
Cache *c1;

int L1_writeback=0,L1_hit=0,L1_reads_miss=0,L1_reads=0,L1_write=0,L1_write_miss,L1_memory_traffic=0;
double L1_miss_rate=0.0,L2_miss_rate=0.0;
int L2_writeback=0,L2_hit=0,L2_reads_miss=0,L2_reads=0,L2_write=0,L2_write_miss,L2_memory_traffic=0;


void L2_LRUInsert(int L2_sets,int L2_assoc,int tag,int index1,char action){
    int temp;
    //first check hit in 1 loop
    int flag=1;
    for(int j=0;j<L2_assoc;j++)
    {
        if(c1[index1].cache[j]==tag)
        {
            flag++;
            L2_hit++;
            for(int i=0;i<L2_assoc;i++)
            {
                if(c1[index1].priority[j]>c1[index1].priority[i])
                {
                    c1[index1].priority[i]++;
                }
            }
            c1[index1].priority[j]=1;
            if(action=='w')
                c1[index1].dirty[j]=1;
            break;
        }
    }
    //second loop check any empty space
    for(int j=0;j<L2_assoc;j++)
    {    if(flag==2)
    {
        break;
    }
        if(c1[index1].cache[j]==-1)
        {
            // L1_miss++;
            c1[index1].cache[j]=tag;
            flag++;
            if(action=='w')
            {
                L2_write_miss++;
                c1[index1].dirty[j]=1;
            }
            else
            {
                L2_reads_miss++;
                c1[index1].dirty[j]=0;
            }
            for(int i=0;i<L2_assoc;i++)
            {
                if(c1[index1].priority[i]<c1[index1].priority[j])
                {
                    c1[index1].priority[i]++;
                }
            }
            c1[index1].priority[j]=1;
            break;
        }
    }
    //which to evict
    for(int j=0;j<L2_assoc;j++)
    {
        if(flag==2){
            break;
        }
        if(c1[index1].priority[j]==L2_assoc)
        {
            if(c1[index1].dirty[j]==1)
                L2_writeback++;
            c1[index1].cache[j]=tag;
            if(action=='w')
            {
                L2_write_miss++;
                c1[index1].dirty[j]=1;
            }
            else
            {
                c1[index1].dirty[j]=0;
                L2_reads_miss++;
            }
            for(int i=0;i<L2_assoc;i++)
            {
                flag++;
                if(c1[index1].priority[i]<c1[index1].priority[j])
                {
                    c1[index1].priority[i]++;
                }
            }
            c1[index1].priority[j]=1;
            break;
        }
    }
}

void init_L2(int L2_size,int L2_assoc,int L2_sets,char *filename,int offset,int L2_indexbits){
    c1=(Cache *)malloc(sizeof(Cache)*L2_sets);
    for(int i=0;i<L2_sets;i++){
        c1[i].cache=malloc(sizeof(int)*L2_assoc);
        c1[i].priority=malloc(sizeof(int)*L2_assoc);
        c1[i].dirty=malloc(sizeof(int)*L2_assoc);
    }
    for(int i=0;i<L2_sets;i++){
        for(int j=0;j<L2_assoc;j++){
            c1[i].cache[j]=-1;
            c1[i].priority[j]=j+1;
            c1[i].dirty[j]=0;
        }
    }
}
void printout(int L1_sets,int L1_ASSOC,int L2_sets,int L2_assoc){
    L1_miss_rate=(double)(L1_reads_miss+L1_write_miss)/(L1_reads+L1_write);
    L1_memory_traffic=L1_reads_miss+L1_write_miss+L1_writeback;
    L2_memory_traffic=L2_reads+L2_write+L2_writeback;
    printf("===== L1 contents =====\n");
    for(int i=0;i<L1_sets;i++){
            printf("Set\t%d:\t",i);
        for(int j=0;j<L1_ASSOC;j++){
            if(c[i].dirty[j]==1){
                printf("%x D  ",c[i].cache[j]);
        }
            else{
                printf("%x    ",c[i].cache[j]);
            }
        }
        printf("\n");
    }
    if((L2_sets!=0)&&(L2_assoc!=0)){
        L1_memory_traffic=L2_memory_traffic;
        printf("===== L2 contents =====\n");
        for(int i=0;i<L2_sets;i++){
            printf("Set\t%d:\t",i);
            for(int j=0;j<L2_assoc;j++){
                if(c1[i].dirty[j]==1){
                    printf("%x D  ",c1[i].cache[j]);
                }
                else{
                    printf("%x    ",c1[i].cache[j]);
                }
            }
            printf("\n");
        }
    }
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads:\t\t%d\n",L1_reads);
    printf("b. number of L1 read misses:\t%d\n",L1_reads_miss);
    printf("c. number of L1 writes:\t\t%d\n",L1_write);
    printf("d. number of L1 write misses:\t%d\n",L1_write_miss);
    printf("e. L1 miss rate:\t\t%f\n",L1_miss_rate);
    printf("f. number of L1 writebacks:\t%d\n",L1_writeback);
    printf("g. number of L2 reads:\t\t%d\n",L2_reads);
    printf("h. number of L2 read misses:\t%d\n",L2_reads_miss);
    printf("i. number of L2 writes:\t\t%d\n",L2_write);
    printf("j. number of L2 write misses:\t%d\n",L2_write_miss);
    printf("k. L2 miss rate:\t\t%f\n",L2_miss_rate);
    printf("l. number of L2 writebacks:\t%d\n",L2_writeback);
    printf("m. total memory traffic:\t%d\n",L1_memory_traffic);
}

void L1_LRUInsert(int L1_sets,int L1_assoc,int L2_sets,int L2_assoc,int tag,int tag1,int index,int index1,char action,int L1_indexbits,int tag_bit1,int offset,int L2_indexbits){
    int temp;
    //first check hit in 1 loop
	int flag=1;
    int flag1=0;
    for(int j=0;j<L1_assoc;j++)
	{
		if(c[index].cache[j]==tag)
		{
			flag++;
			L1_hit++;
			for(int i=0;i<L1_assoc;i++)
			{
				if(c[index].priority[j]>c[index].priority[i])
				{		                   
					c[index].priority[i]++;
				}
			}
			c[index].priority[j]=1;	
			if(action=='w')
				c[index].dirty[j]=1;
            printf("%d\n",index1);
            L2_LRUInsert(L2_sets,L2_assoc,tag1,index1,'r');
			break;
		}
    }						              
    //second loop check any empty space
    for(int j=0;j<L1_assoc;j++)
	{	if(flag==2)
        {
			break;
        }
        if(c[index].cache[j]==-1)
		{
		   // L1_miss++;
		   c[index].cache[j]=tag;
		   flag++;
		   if(action=='w')
		   {
               L1_write_miss++;
			   c[index].dirty[j]=1;
		   }
		   else
		   {
               L1_reads_miss++;
			   c[index].dirty[j]=0;
		   }
			for(int i=0;i<L1_assoc;i++)
			{
				if(c[index].priority[i]<c[index].priority[j])
				{
					c[index].priority[i]++;
				}
			}
			c[index].priority[j]=1;
            L2_LRUInsert(L2_sets,L2_assoc,tag1,index1,'r');
			break;
		}
 	}
    //which to evict
    for(int j=0;j<L1_assoc;j++)
	{
        if(flag==2){
            break;
        }
	    if(c[index].priority[j]==L1_assoc)
		{
            if(c[index].dirty[j]==1)
            {
                L1_writeback++;
                if((L2_assoc!=0)&&(L2_sets!=0)){
                    L2_write++;
                    unsigned address=tag<<L1_indexbits|index;
                    tag1=address>>L1_indexbits;
                    unsigned int r=INT_MAX;
                    r=r<1;
                    r=r>>(tag+offset);
                    index1=address&r;
                    L2_LRUInsert(L2_sets,L2_assoc,tag1,index1,'w');
                    flag1++;
                }
            }
		    c[index].cache[j]=tag;
			if(action=='w')
		   {
			   L1_write_miss++;
			   c[index].dirty[j]=1;
		   }
		   else
		   {
			   c[index].dirty[j]=0;
			   L1_reads_miss++;
		   }
		    for(int i=0;i<L1_assoc;i++)
			{
				flag++;
	    		if(c[index].priority[i]<c[index].priority[j])
				{
	               	c[index].priority[i]++;							                
				}
			}
			c[index].priority[j]=1;
            if((flag1==0)&&(L2_assoc!=0)&&(L2_sets!=0)){
                L2_reads++;
                 L2_LRUInsert(L2_sets,L2_assoc,tag1,index,'r');
            }
			break;
		}
    }
}


void init_L1(int L1_size,int L1_assoc,int L1_sets,int L2_size,int L2_assoc,int L2_sets,char *filename,int offset,int L1_indexbits,int L2_indexbits){
    c=(Cache *)malloc(sizeof(Cache)*L1_sets);
    for(int i=0;i<L1_sets;i++){
        c[i].cache=malloc(sizeof(int)*L1_assoc);
        c[i].priority=malloc(sizeof(int)*L1_assoc);
        c[i].dirty=malloc(sizeof(int)*L1_assoc);
    }
    for(int i=0;i<L1_sets;i++){
        for(int j=0;j<L1_assoc;j++){
            c[i].cache[j]=-1;
            c[i].priority[j]=j+1;
            c[i].dirty[j]=0;
        }
    }
    FILE * ptr;
    ptr=fopen(filename,"r");
    if(ptr==NULL){
        perror("cannot open the file");
        exit(0);
    }
    else{
        char action;
        int address=0x0;
        int tag,tag1,ones=1,sum=0,sum1=0;
        int index;
        while(!feof(ptr)){
            fscanf(ptr,"%c %x\n",&action,&address);
            sum=L1_indexbits+offset;
            sum1=L2_indexbits+offset;
            tag=address>>sum;
            tag1=address>>sum1;
            int tag_bit=32-sum;
            int tag_bit1=32-sum1;
            unsigned int r=INT_MAX;
            r=INT_MAX;
            r=r<<1;
            r=r>>(tag_bit+offset);
            unsigned int r1=INT_MAX;
            r1=INT_MAX;
            r1=r1<<1;
            r1=r1>>(tag_bit1+offset);
            unsigned int index1=address<<tag_bit;
            index=index1>>tag_bit;
            index=index>>offset;
            unsigned int index2 = address <<tag_bit1;
            index1=index2>>tag_bit1;
            index1=index1>>offset;
            index= index & r;
            //index1=index & r1;
            if(action=='r')
                L1_reads++;
            else if(action=='w')
                L1_write++;
            L1_LRUInsert(L1_sets,L1_assoc,L2_sets,L2_assoc,tag,tag1,index,index1,action,L1_indexbits,tag_bit1,offset,L2_indexbits);
            }
        fclose(ptr);
        }
}



int main(int argc, char * argv[]){
    char ch;
    if(argc>0){
        int blocksize= atoi(argv[1]);
        int L1_size=atoi(argv[2]);
        int L1_assoc=atoi(argv[3]);
        int L2_size=atoi(argv[4]);
        int L2_assoc=atoi(argv[5]);
        int rep_policy=atoi(argv[6]);
        int incl_policy=atoi(argv[7]);
        char * filename=argv[8];
        int L1_sets=L1_size/(L1_assoc * blocksize);
        int offset=log(blocksize)/log(2);
        int L1_indexbits=log(L1_sets)/log(2);
        int L2_sets,L2_indexbits;
        printf("===== Simulator configuration =====\n");
        printf("BLOCKSIZE:\t\t%d\n",blocksize);
        printf("L1_SIZE:\t\t%d\n",L1_size);
        printf("L1_ASSOC:\t\t%d\n",L1_assoc);
        printf("L2_SIZE:\t\t%d\n",L2_size);
        printf("L2_ASSOC:\t\t%d\n",L2_assoc);
        printf("REPLACEMENT POLICY:\tLRU\n");
        printf("INCLUSION POLICY:\tnon-inclusive\n");
        printf("trace_file:\t\t%s\n",filename);
        if((L2_assoc!=0)&&(L2_size!=0)){
            L2_sets=L2_size/(L2_assoc * blocksize);
            L2_indexbits=log(L2_sets)/log(2);
            init_L2(L2_size,L2_assoc,L2_sets,filename,offset,L2_indexbits);
        }
        init_L1(L1_size,L1_assoc,L1_sets,L2_size,L2_assoc,L2_sets,filename,offset,L1_indexbits,L2_indexbits);
        printout(L1_sets,L1_assoc,L2_sets,L2_assoc);
    }
    else{
        printf("entered 0 values");
        exit(0);
    }
    return 0;
}

