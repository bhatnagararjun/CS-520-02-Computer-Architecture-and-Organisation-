#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INT_MAX 2147483647

typedef struct prediction_table{
    int address;
    int decision;
}prediction_table;

int mis_bimodal=0;
int mis_gshare=0;
int mis_hybrid=0;

int bimodal_hits=0;
int gshare_hits=0;
int hybrid_hits=0;
int f2=0,f1=0;
int j=0;
prediction_table pt;

int * counter;
int * counter1;


unsigned int global_register=0;
int * chooser_table;

int bimodal_predictor(int m){
    f1=1;
    int prediction=1;
    bimodal_hits++;
    if(counter[pt.address]>=4){
        prediction=1;
    }
    else{
        prediction=0;
    }
    if(pt.decision == 1){
        counter[pt.address]++;
        if(counter[pt.address] >= 7){
            counter[pt.address]=7;
        }
    }
    if(pt.decision == 0){
        counter[pt.address]--;
        if(counter[pt.address] < 0){
            counter[pt.address]=0;
        }
    }
    if(prediction != pt.decision){
        mis_bimodal++;f1=0;
    }
    return prediction;
}

int gshare(int m,int n){
    f1=1;
    unsigned int b= INT_MAX;
    int prediction=1;
    b=b<<1;
    int k= 32-n;
    b =b>>k;
    global_register=(global_register & b);
    unsigned int lower_n_bits=pt.address & b;
    gshare_hits++;
    int address= pt.address ^ global_register;
    pt.address= pt.address ^ global_register;
    if(counter1[address]>=4){
        prediction=1;
    }else{
        prediction=0;
    }
    if(pt.decision==1){
            counter1[address]++;
            if(counter1[address]>7){
                counter1[address]=7;
            }
    }else{
            counter1[address]--;
            if(counter1[address]<0){
                counter1[address]=0;
            }
    }
    if(prediction!=pt.decision){
        mis_gshare++;f2=0;
    }
    global_register=global_register>>1;
    int msb_register;
    if(pt.decision==0){
        msb_register=0;
    }else{
        msb_register=INT_MAX;
    }
    msb_register=msb_register<<1;
    msb_register=msb_register>>31;
    int k2=n-1;
    msb_register=msb_register<<k2;
    global_register=global_register | msb_register;
    return prediction;
}

void hybrid(int m,int n,int k,int m1){
    int c=1;
    hybrid_hits++;
    unsigned int k1=INT_MAX;
    k1=k1<<1;
    k1=k1>>(32-m1);
    int prediction;
    int address=pt.address&k1;
    int temp=pt.address;
    pt.address=address;
    int prediction_bimodal=bimodal_predictor(m1);
    pt.address=temp;
    int prediction_gshare=gshare(m,n);
    k1=INT_MAX;
    k1=k1<<1;
    k1=k1>>(32-k);
    address=pt.address&k1;
    if(chooser_table[address]>=2){
        prediction=prediction_gshare;
    }else{
        prediction=prediction_bimodal;
        global_register=global_register>>1;
        int msb_register;
        if(pt.decision==0){
            msb_register=0;
        }else{
            msb_register=INT_MAX;
        }
        msb_register=msb_register<<1;
        msb_register=msb_register>>31;
        int k2=n-1;
        msb_register=msb_register<<k2;
        global_register=global_register | msb_register;
    }
    if((pt.decision == prediction_gshare)&&(pt.decision != prediction_bimodal) && f1!=f2){
        chooser_table[address]++;
        if(chooser_table[address]>=3){
            chooser_table[address]=3;
        }
    }else if((pt.decision == prediction_bimodal)&&(pt.decision != prediction_gshare) && f1!=f2){
        chooser_table[address]--;
        if(chooser_table[address]<=0){
            chooser_table[address]=0;
        }
        if(prediction_gshare!=pt.decision || prediction_bimodal !=pt.decision){
            mis_hybrid++;
        }
    }
}
void init_BTB(char type[],char filename[],int m,int n,int k,int m1){
    int d=1;
    for(int i=0;i<m;i++){
        d=d*2;
    }
    counter= (int *)malloc(d*sizeof(int));
    counter1=(int *)malloc(d*sizeof(int));
    int c=1;
    chooser_table=(int *)malloc(sizeof(int)*c);
    for(int i=0;i<k;i++){
        c=c*2;
    }
    for(int i=0;i<c;i++){
        chooser_table[i]=1;
    }
    for(int i=0;i<d;i++){
        counter[i]=4;
        counter1[i]=4;
    }
    pt.address=0;
    pt.decision=0;
    FILE * ptr= fopen(filename,"r");
    if(ptr==NULL){
        printf("error opening the file");
        exit(0);
    }
    
    int i=0;
    char * s;
    while(!feof(ptr)){
        char action;
        int address=0x0;
        fscanf(ptr,"%x %c",&address,&action);
        if(address == 0) {
            continue;
        }
        if(action=='t'){
            pt.decision=1;
        }else{
            pt.decision=0;
        }
        pt.address = address>>2;
        
        unsigned int r= INT_MAX;
        r=r<<1;
        int k = 32-m;
        r=r>>k;
        pt.address= pt.address & r;
        if(strcmp(type,"bimodal")==0){
            bimodal_predictor(m);
        }
        else if(strcmp(type,"gshare")==0){
            if(n==0){
                bimodal_predictor(m);
            }
            else if(m<n){
                printf("M should be greater than N");
            }
            else if(n<=m){
                gshare(m,n);
            }
        }
        else{
	   hybrid(m,n,k,m1);
        }
    }
        fclose(ptr);
    
}

void stat(char * type,int k,int m,int m1){
    if(strcmp(type,"bimodal")==0){
        printf("OUTPUT\n");
        printf("number of predictions:\t%d\n",bimodal_hits);
        printf("number of mispredictions:\t%d\n",mis_bimodal);
        double accuracy=(((double)mis_bimodal)/((double)bimodal_hits))*100;
        printf("misprediction rate:\t%f\n",accuracy);
        printf("FINAL BIMODAL CONTENTS\n");
        int d=1;
        for(int i=0;i<m;i++){
            d=d*2;
        }
        for(int i=0;i<d;i++){
            printf("%d\t%d\n",i,counter[i]);
        }
    }
    else if(strcmp(type,"gshare")==0){
        printf("OUTPUT\n");
        printf("number of predictions:\t%d\n",gshare_hits);
        printf("number of mispredictions:\t%d\n",mis_gshare);
        double accuracy=(((double)mis_gshare)/((double)gshare_hits))*100;
        printf("misprediction rate:\t%f\n",accuracy);
        printf("FINAL GSHARE CONTENTS\n");
        int d=1;
        for(int i=0;i<m;i++){
            d=d*2;
        }
        for(int i=0;i<d;i++){
            printf("%d\t%d\n",i,counter1[i]);
        }
    }
    else{
        printf("OUTPUT\n");
        printf("number of predictions:\t%d\n",hybrid_hits);
        printf("number of mispredictions:\t%d\n",mis_hybrid);
        double accuracy=(((double)mis_hybrid)/((double)hybrid_hits))*100;
        printf("misprediction rate:\t%f\n",accuracy);
        printf("FINAL CHOOSER CONTENTS\n");
        int c=1;
        for(int i=0;i<k;i++){
            c=c*2;
        }
        for(int i=0;i<m1;i++){
            printf("%d\t%d\n",i,chooser_table[i]);
        }
        printf("FINAL BIMODAL CONTENTS\n");
        int d=1;
        for(int i=0;i<m1;i++){
            d=d*2;
        }
        for(int i=0;i<d;i++){
            printf("%d\t%d\n",i,counter[i]);
        }
        printf("FINAL GSHARE CONTENTS\n");
        d=1;
        for(int i=0;i<m;i++){
            d=d*2;
        }
        for(int i=0;i<d;i++){
            printf("%d\t%d\n",i,counter1[i]);
        }
    }
}

int main(int argc, char **argv){
    int n=0,m,m1=0,k=0;
    char type[255];
    char filename[255];
    strcpy(type,argv[1]);
    if(strcmp(type,"bimodal")==0){
        m=atoi(argv[2]);
        strcpy(filename,argv[3]);
    }
    else if(strcmp(type,"gshare")==0){
        m=atoi(argv[2]);
        n=atoi(argv[3]);
        strcpy(filename,argv[4]);
    }
    else if(strcmp(type,"hybrid")==0){
        k=atoi(argv[2]);
        m=atoi(argv[3]);
        n=atoi(argv[4]);
        strcpy(filename,argv[6]);
        m1=atoi(argv[5]);
    }
    init_BTB(type,filename,m,n,k,m1);
    stat(type,k,m,m1);
    free(counter);
    free(counter1);
    return 0;
}
