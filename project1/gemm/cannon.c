#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <string.h>
#include <time.h>


float **A, **B, **C;     // C = A*B
float *a, *b, *c, *tmpa, *tmpb;        // small block of A, B, C
int dimA, dima;
int world_rank, world_size, sqrt_world_size;
int my_row, my_col;
MPI_Status status;


// (row, col) to rank
int row_col_to_rank(int row, int col, int dim){
    return ((row+dim)%dim)*dim + (col+dim)%dim;
}


// initial matrix A, B, C
void init_matrix_A_B_C(){
    // srand( (unsigned int)time(NULL));
    for(int i=0; i<dimA; i++){
        for(int j=0; j<dimA; j++){
            A[i][j] = rand()%10;
            B[i][j] = rand()%10;
            C[i][j] = 0.;
        }
    }
}


// scatter matrix A and B to each process
void scatter_A_B(){
    int min_row, max_row, min_col, max_col;
    int idx;

    for(int p=0; p<world_size; p++){
        min_col = (p % sqrt_world_size) * dima;
        max_col = min_col + dima;
        min_row = (p / sqrt_world_size) * dima;
        max_row = min_row + dima;
        idx = 0;

        for(int i=min_row; i<max_row; i++){
            for(int j=min_col; j<max_col; j++){
                tmpa[idx] = A[i][j];
                tmpb[idx] = B[i][j];
                idx ++;
            }
        }

        if(p == 0){
            // copy tmpa, tmpb to a, b
            memcpy(a, tmpa, (dima*dima)*sizeof(float));
            memcpy(b, tmpb, (dima*dima)*sizeof(float));
        }
        else{
            // process 0 send tmpa, tmpb to other process
            MPI_Send(tmpa, dima*dima, MPI_FLOAT, p, 1, MPI_COMM_WORLD);
            MPI_Send(tmpb, dima*dima, MPI_FLOAT, p, 2, MPI_COMM_WORLD);
        }
    }
}


// cannon algorithm step 1. A left-offset, B up-offset
void cannon_init_alignment(){
    MPI_Sendrecv(a, dima*dima, MPI_FLOAT, row_col_to_rank(my_row, my_col-my_row, sqrt_world_size), 0, 
                tmpa, dima*dima, MPI_FLOAT, row_col_to_rank(my_row, my_col+my_row, sqrt_world_size), 0, MPI_COMM_WORLD, &status);
    memcpy(a, tmpa, (dima*dima)*sizeof(float));

    MPI_Sendrecv(b, dima*dima, MPI_FLOAT, row_col_to_rank(my_row-my_col, my_col, sqrt_world_size), 0, 
                tmpb, dima*dima, MPI_FLOAT, row_col_to_rank(my_row+my_col, my_col, sqrt_world_size), 0, MPI_COMM_WORLD, &status);
    memcpy(b, tmpb, (dima*dima)*sizeof(float));
}


// cannon algorithm step 2. 
void cannon_main_shift(){
    for(int p=0; p<sqrt_world_size; p++){

        for(int i=0; i<dima; i++){
            for(int j=0; j<dima; j++){
                for(int k=0; k<dima; k++){
                    c[i*dima+j] += (a[i*dima+k]*b[k*dima+j]);
                }
            }
        }

        MPI_Sendrecv(a, dima*dima, MPI_FLOAT, row_col_to_rank(my_row, my_col-1, sqrt_world_size), 0, 
                    tmpa, dima*dima, MPI_FLOAT, row_col_to_rank(my_row, my_col+1, sqrt_world_size), 0, MPI_COMM_WORLD, &status);
        memcpy(a, tmpa, (dima*dima)*sizeof(float));

        MPI_Sendrecv(b, dima*dima, MPI_FLOAT, row_col_to_rank(my_row-1, my_col, sqrt_world_size), 0, 
                    tmpb, dima*dima, MPI_FLOAT, row_col_to_rank(my_row+1, my_col, sqrt_world_size), 0, MPI_COMM_WORLD, &status);
        memcpy(b, tmpb, (dima*dima)*sizeof(float));
    }
}


// process 0 gathers results from other processes
void gather_result(){
    int min_row, max_row, min_col, max_col;
    int idx_row, idx_col;
    
    // fill C with result of process 0
    for(int i=0; i<dima; i++){
        for(int j=0; j<dima; j++){
            C[i][j] = c[i*dima+j];
        }
    }

    for(int p=1; p<world_size; p++){
        MPI_Recv(c, dima*dima, MPI_FLOAT, p, 0, MPI_COMM_WORLD, &status);
       
        min_col = (p % sqrt_world_size) * dima;
        max_col = min_col + dima;
        min_row = (p / sqrt_world_size) * dima;
        max_row = min_row + dima;
        idx_row = 0;

        for(int i=min_row; i<max_row; i++){
            idx_col = 0;
            for(int j=min_col; j<max_col; j++){
                C[i][j] = c[idx_row*dima+idx_col];
                idx_col ++;
            }
            idx_row ++;
        }
    }
}


// print matrix
void print_matrix(float **m){
    for(int i=0; i<dimA; i++){
        for(int j=0; j<dimA; j++){
            printf("%5.0f", m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}


// free A B C
void free_matrix(float **m){
    for(int i=0; i<dimA; i++){
        free(m[i]);   
    }
    free(m); 
}


int main(int argc, char* argv[]){
    double run_time = 0.;
    MPI_Init(&argc, &argv);  
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    sqrt_world_size = sqrt(world_size);
    if (sqrt_world_size*sqrt_world_size != world_size){
        if (world_rank == 0)
            printf("Number of processors is not a quadratic number!\n");
        MPI_Finalize();
        exit(1);
    }
    if (argc != 2){
        if (world_rank == 0)
            printf("usage: mpiexec -n ProcNum ./cannon MatrixDimension\n");
        MPI_Finalize();
        exit(1);
    }
    dimA = atoi(argv[1]);
    dima = dimA / sqrt_world_size;

    MPI_Barrier(MPI_COMM_WORLD);
    run_time -= MPI_Wtime();

    if(dima*sqrt_world_size!=dimA){
        if(world_rank == 0){
            printf("Sorry, matrix dim %d cannot be divided by sqrt process size %d totally\n", dimA, world_size);
            printf("will fix it on the future work.\n");
        }
        MPI_Finalize();
        exit(1);
    }
    
    // rank to (row, col)
    my_col = world_rank % sqrt_world_size;
    my_row = world_rank / sqrt_world_size;
    
    // allocate space for a, b, c. init c as 0.
    a = (float *)malloc(sizeof(float) * (dima*dima));
    b = (float *)malloc(sizeof(float) * (dima*dima));
    c = (float *)malloc(sizeof(float) * (dima*dima));
    tmpa = (float *)malloc(sizeof(float) * (dima*dima));
    tmpb = (float *)malloc(sizeof(float) * (dima*dima));
    for(int i=0; i<dima*dima; i++) c[i] = 0.;

    if(world_rank==0){
        // allocate space for A, B, C
        A = (float **)malloc(sizeof(float*) * dimA);
        B = (float **)malloc(sizeof(float*) * dimA);
        C = (float **)malloc(sizeof(float*) * dimA);
        for(int i=0; i<dimA; i++){
            A[i] = (float *)malloc(sizeof(float) * dimA);
            B[i] = (float *)malloc(sizeof(float) * dimA);
            C[i] = (float *)malloc(sizeof(float) * dimA);
        }      
  
        // random init A, B. And set C's elements are all 0s.
        init_matrix_A_B_C();
        // scatter A, B to each process
        scatter_A_B();
    }
    else{
        // receive a block of A, B
        MPI_Recv(a, dima*dima, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(b, dima*dima, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);
    }
    
    // cannon algorithm
    cannon_init_alignment();
    cannon_main_shift();

    if(world_rank==0){
        // process 0 gathers 'c' from each process to generate 'C' 
        // show results 
        gather_result();
    }
    else{
        MPI_Send(c, dima*dima, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    run_time += MPI_Wtime();
    if (world_rank==0){
        printf("random matrix A: \n");
        print_matrix(A);
        printf("random matrix B: \n");
        print_matrix(B);
        printf("C=A*B, matrix C: \n");
        print_matrix(C);
        
        printf("Process size %d, Matrix dim %d\n", world_size, dimA);
        printf("Cannon algorithm running time: %lf\n", run_time);
        free_matrix(A);
        free_matrix(B);
        free_matrix(C);
    }

    free(a);
    free(b);
    free(c);
    free(tmpa);
    free(tmpb);
    
    MPI_Finalize();
}

