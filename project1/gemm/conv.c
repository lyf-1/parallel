#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


// #define I_SIZE 5        // Input Size
#define K_SIZE 4        // Kernel Size
float **A, **B;         // A is matrix, B is kernel matrix
int I_SIZE;


// initial matrix A, B
void init_matrix_A_B(int flag){
    // srand( (unsigned int)time(NULL));
    for(int i=0; i<I_SIZE; i++){
        for(int j=0; j<I_SIZE; j++){
            A[i][j] = rand()%10;
        }
    }

    // flag = 0, cov | flag = 1, sum_pool
    if(flag == 0){
        for(int i=0; i<K_SIZE; i++){
            for(int j=0; j<K_SIZE; j++){
                B[i][j] = rand()%10;
            }
        }
    }
    else{
        for(int i=0; i<K_SIZE; i++){
            for(int j=0; j<K_SIZE; j++){
                B[i][j] = 1.;
            }
        }
    }
}


void cal_position(int rank, int size, int *start, int *end){
    *start = (I_SIZE / size) * rank;
    if(I_SIZE % size > rank) {
        *start += rank;
        *end = *start + (I_SIZE / size) + 1;
    }
    else {
        *start += I_SIZE % size;
        *end = *start + (I_SIZE / size);
    }
}


// 4*4 kernel conv, output shape is same as input shape.
// Thus, need non-symmetric padding.
float convolution(int x, int y){
    int k, j;           
    int i1, j1, i2, j2; 
    float res1, res2;     
    float result = 0.;
    
    
    for(k=-1; k<3; k++) {
    	for(j=-1; j<3; j++) {
                  
            i1 = k+1;
            j1 = j+1;
            i2 = x+k;
            j2 = y+j;
            
            if( ( (i1 >= 0) && (i1 < K_SIZE) ) && ( (j1 >= 0) && (j1 < K_SIZE) ) ) {
                res1 = B[i1][j1];
            }
            else {
                res1 = 0.;
            }
            
            if( ( (i2 >= 0) && (i2 < I_SIZE) ) && ( (j2 >= 0) && (j2 < I_SIZE) ) ) {
                res2 = A[i2][j2];
            }
            else {
                res2 = 0.;
            }
    		result += res1*res2;
    	}
    }
    return result;
}


void print_array(float *m){
    for(int i=0; i<I_SIZE; i++){
        for(int j=0; j<I_SIZE; j++){
            printf("%5.0f", m[i*I_SIZE+j]);
        }
        printf("\n");
    }
    printf("\n");
}


void print_matrix(float **m, int dim){
    for(int i=0; i<dim; i++){
        for(int j=0; j<dim; j++){
            printf("%5.0f", m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}


void free_matrix(float **m, int dim){
    for(int i=0; i<dim; i++){
        free(m[i]);   
    }
    free(m); 
}

int main(int argc, char* argv[]){
    int world_rank, world_size;
    int proc_start, proc_end;
    int conv_or_pool;  // 0 is conv, 1 is sum_pool
    MPI_Status status;
    double run_time = 0.;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(argc != 3){
        if(world_rank==0)
            printf("Usage: mpiexec -n <process_num> ./conv <matrix_dim> <0 or 1(cov or sum_pool)>\n");
        MPI_Finalize();
        exit(1);
    }

    I_SIZE = atoi(argv[1]);
    conv_or_pool = atoi(argv[2]);

    run_time -= MPI_Wtime();

    // allocate memory and init for A and B
    A = (float **)malloc(sizeof(float*) * I_SIZE);
    B = (float **)malloc(sizeof(float*) * K_SIZE);
    for(int i=0; i<I_SIZE; i++){
        A[i] = (float *)malloc(sizeof(float) * I_SIZE);
    }
    for(int i=0; i<K_SIZE; i++){
        B[i] = (float *)malloc(sizeof(float) * K_SIZE);
    }
    init_matrix_A_B(conv_or_pool);

    // calculate proc_start, proc_en
    cal_position(world_rank, world_size, &proc_start, &proc_end);
    printf("Process: %d of %d | Start Point:%d ~ End Point:%d\n\n", world_rank, world_size, proc_start, proc_end);

    float *output;
    int output_size = (proc_end-proc_start) * I_SIZE;
    output = (float *)malloc(sizeof(float) * output_size);      
    for(int i=proc_start; i<proc_end; i++){
        for(int j=0; j<I_SIZE; j++){
            output[(i-proc_start)*I_SIZE+j] = convolution(i, j);
        }
    }

    if(world_rank==0){
        // gather result
        float *all_output;
        all_output = (float *)malloc(sizeof(float) * (I_SIZE*I_SIZE));
        
        // copy output of process 0
        for(int i=0; i<output_size; i++)
            all_output[i] = output[i];

        // receive output of other processes
        int idx = output_size;
        for(int p=1; p<world_size; p++){
            int tmp_start, tmp_end;
            cal_position(p, world_size, &tmp_start, &tmp_end);

            float *tmp_output;
            int tmp_output_size = (tmp_end-tmp_start) * I_SIZE;
            tmp_output = (float *)malloc(sizeof(float) * tmp_output_size);
            
            MPI_Recv(tmp_output, tmp_output_size, MPI_FLOAT, p, 0, MPI_COMM_WORLD, &status);

            for(int i=0; i<tmp_output_size; i++){              
                all_output[idx] = tmp_output[i];
                idx += 1;   
            }

            free(tmp_output);
        }
        run_time += MPI_Wtime();
        printf("Matrix: \n");
        print_matrix(A, I_SIZE);
        printf("4*4 kernel: \n");
        print_matrix(B, K_SIZE);
        printf("Result: \n");
        print_array(all_output);
        free(all_output);
        printf("running time: %lf\n", run_time);
    }
    else{
        MPI_Send(output, output_size, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    free(output);
    free_matrix(A, I_SIZE);
    free_matrix(B, K_SIZE);
    MPI_Finalize();
}