#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "real.h"

void reader(int *rowsP,int *nnz, size_t *offsetR,size_t *offsetC,size_t *offsetV, int *rows,  int global_n, int global_nnz, int worldSize )
{

    int *countRows,*cumulativeRows;
    int *count_nnz,*cumulative_nnz;
    countRows      = (int *) malloc(worldSize*sizeof(int)); 
    cumulativeRows = (int *) malloc(worldSize*sizeof(int)); 
    
    count_nnz      = (int *) malloc(worldSize*sizeof(int)); 
    cumulative_nnz = (int *) malloc(worldSize*sizeof(int)); 

    countRows[0]=0;
    cumulativeRows[0]=0;
    count_nnz[0]=0;
    cumulative_nnz[0]=0;

    float nnzIncre = (float ) global_nnz/ (float) worldSize;
    float lookingFor=nnzIncre;
    int startRow=0, endRow;
    int partition=0;    
    
    for (int row=0; row<global_n; ++row) {    
        if ( (float) rows[row+1] >=  lookingFor ) { 
        
            // search for smallest difference
            if (fabs ( lookingFor - rows[row+1])  <= fabs ( lookingFor - rows[row])   ) {
                endRow = row;
            } else {
                endRow = row-1;
            } // end if //
            
            count_nnz[partition] = rows[endRow+1] - rows[startRow];            
            countRows[partition] = endRow-startRow+1;
            
            nnz[partition]=count_nnz[partition];
            rowsP[partition] = countRows[partition];
            /*
            printf("partition %4d, rows:(%4d,%4d), means %4d rows and %4d non-zeros --> cumulative (%4d, %4d)\n", 
                   partition, startRow, endRow, 
                   countRows[partition], rows[endRow+1] - rows[startRow],
                   cumulativeRows[partition], cumulative_nnz[partition] 
                   );
            */       
            startRow = endRow+1;
            offsetR[partition] = ( 2 + cumulativeRows[partition])*sizeof(int);
            offsetC[partition] = ( 2 +  (global_n+1) + cumulative_nnz[partition])*sizeof(int);
            offsetV[partition] = ( 2 +  (global_n+1) + global_nnz )*sizeof(int) + cumulative_nnz[partition] * sizeof(real) ;

            ++partition;
            
            cumulative_nnz[partition] = cumulative_nnz[partition-1] + count_nnz[partition-1];
            cumulativeRows[partition] = cumulativeRows[partition-1] + countRows[partition-1];
            
            
            if (partition < worldSize-1) {
               lookingFor += nnzIncre;
            } else {
                lookingFor=global_nnz;
            } // end if //   
        } // end if // 
    } // end for //
    
    free(countRows);
    free(cumulativeRows);
    free(count_nnz);
    free(cumulative_nnz);
} // end of reader //
