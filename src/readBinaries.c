#include <stdio.h>
#include <stdlib.h>

#include "real.h"

void reader(int *rowsPP,  int *nnzPP, size_t *a,size_t *b,size_t *c, int *rows,  int global_n, int global_nnz, int worldSize);

int main(int argc, char *argv[]) 
{
    FILE *fh;
    int worldSize, worldRank;
    // worldRank is the partition I am pretending to be //
    
    if (argc == 4 ) {
        if((fh = fopen(argv[1], "rb")  )   == NULL) {
            printf("No such file\n");
            exit(1);
        } // end if //
        worldRank=atoi(argv[2]);
        worldSize=atoi(argv[3]);
        if (worldRank >= worldSize) {
            printf("worldRank must be smaller than worldSize\n"); exit(-2);
        } // end if //
    } else {
        printf("Use: %s  filename  worldRank worldSize \n", argv[0]);     
        exit(0);
    } // endif //

    //printf("worldRank: %d, worldSize: %d\n", worldRank,worldSize); exit(0);

    int n_global,nnz_global;

    // reading n //
    if ( !fread(&n_global, sizeof(int), 1, fh) ) exit(0); 

    // reading nnz //
    if ( !fread(&nnz_global, sizeof(int), (size_t) 1, fh)) exit(0);
    
    //printf("n: %d, nnz: %d\n",n_global,nnz_global ); exit(0);

    int *rows;
    rows = (int *) malloc((n_global+1)*sizeof(int));    
    
    // reading rows vector (n+1) values //
    if ( !fread(rows, sizeof(int), (size_t) (n_global+1), fh)) exit(0);
    
    size_t *offsetR,*offsetC,*offsetV; 
    offsetR = (size_t *) malloc(worldSize*sizeof(size_t)); 
    offsetC = (size_t *) malloc(worldSize*sizeof(size_t)); 
    offsetV = (size_t *) malloc(worldSize*sizeof(size_t)); 
    
    
    int *nnzPP, *rowsPP; 
    rowsPP = (int *) malloc(worldSize*sizeof(int)); 
    nnzPP = (int *) malloc(worldSize*sizeof(int)); 
    
    reader(rowsPP,nnzPP, offsetR,offsetC,offsetV, rows, n_global, nnz_global, worldSize);
    
    free(rows);
    
    const int n  =  rowsPP[worldRank];
    const int nnz = nnzPP[worldRank];
    
    printf("I am Rank: %d rows: %d, nnz: %d\n",worldRank,n,nnz );
    
    int *cols_Ptr;
    int *rows_Ptr;
    real *vals;
    
    rows_Ptr = (int *) malloc((n+1)*sizeof(int));    
    cols_Ptr = (int *) malloc(nnz*sizeof(int));    
    vals     = (real *) malloc(nnz*sizeof(real));
    
    // reading rows vector (n+1) values //
    fseek(fh, offsetR[worldRank], SEEK_SET);
    if ( !fread(rows_Ptr, sizeof(int), (size_t) (n+1), fh)) exit(0);
    
    // reading cols vector (nnz) values //
    fseek(fh, offsetC[worldRank], SEEK_SET);
    if ( !fread(cols_Ptr, sizeof(int), (size_t) (nnz), fh)) exit(0);

    // reading vals vector (nnz) values //
    fseek(fh, offsetV[worldRank], SEEK_SET);
    if (sizeof(real) == sizeof(double)) {
        if ( !fread(vals, sizeof(real), (size_t) (nnz), fh)) exit(0);
    } else {
        double *temp = (double *) malloc(nnz*sizeof(double)); 
        if ( !fread(temp, sizeof(double), (size_t) (nnz), fh)) exit(0);
        for (int i=0; i<nnz; i++) {
            vals[i] = (real) temp[i];
        } // end for //    
        free(temp);
    } // end if //

/*
    for (int i=0; i<=n; ++i) {
        printf("%2d ",rows_Ptr[i]);
        if (i>0) printf(" this row has %4d non-zeros",  rows_Ptr[i] -rows_Ptr[i-1] );
        printf("\n");
    } // end for //
     printf("\n\n");
*/
    
    for (int i=0; i<nnz; ++i) {
        printf("%d, %f\n",cols_Ptr[i], vals[i]);
    } // end for //
    

    free(offsetR);
    free(offsetC);
    free(offsetV);
    
    free(rows_Ptr);
    free(cols_Ptr);
    free(vals);
    fclose(fh);
    return 0;    
} // end main() //




