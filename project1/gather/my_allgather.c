#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>

void my_allgather(void* send_data, int send_count, MPI_Datatype send_datatype, void* recv_data, int recv_count, MPI_Datatype recv_datatype, MPI_Comm communicator){
    int rank;
    MPI_Comm_rank(communicator, &rank);
    int nprocs;
    MPI_Comm_size(communicator, &nprocs);

    int total_count=send_count*nprocs;
    if (rank == 0){
        // process 0 receive data from others
        int i;

        for (i=0; i<nprocs; i++){
            if (i == rank){
                // Copy send data to recv data of processor 0
                memcpy(recv_data,send_data,sizeof(recv_datatype)*send_count);
            }
            if (i != rank){
                MPI_Recv(recv_data+sizeof(recv_datatype)*(recv_count*i), recv_count, recv_datatype, i, 0, communicator, MPI_STATUS_IGNORE);
            }
        }
    }else{
        // If we are a sender process, send the data from process 0
        MPI_Send(send_data, send_count, send_datatype, 0, 0, communicator);
    }
    if (rank == 0){
        // process 0 send data to others
        int i;
        for (i=0; i<nprocs; i++){
            if (i != rank){
                MPI_Send(recv_data, total_count, recv_datatype, i, 0, communicator);
            }
        }
    }else{
        // If we are a receiver process, receive the data from process 0
        MPI_Recv(recv_data, total_count, recv_datatype, 0, 0, communicator, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char** argv){
    if (argc != 3) {
        fprintf(stderr, "Usage: compare_bcast num_elements num_trials\n");
        exit(1);
    }
    int num_elements = atoi(argv[1]);
    int num_trials = atoi(argv[2]);

    MPI_Init(NULL, NULL);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    double total_my_gather_time, total_mpi_gather_time;
    total_my_gather_time = total_mpi_gather_time = 0.0;
    int i;

    int* send_nums = (int*)malloc(sizeof(int) * num_elements);
    assert(send_nums != NULL);
    int* recv_nums = (int*)malloc(sizeof(int) * num_elements * nprocs);
    assert(recv_nums != NULL);

    //printf("proc %d send ", rank);
    for (i=0; i<num_elements; i++){
        send_nums[i] = (rank+1)*(i+1); // Random initialize the array 
        //printf("%d ", send_nums[i]);
    }
    //printf("\n");
    for (i = 0; i < num_trials; i++) {
        // Time my_allgather
        // Synchronize before starting timing
        MPI_Barrier(MPI_COMM_WORLD);
        total_my_gather_time -= MPI_Wtime();
        my_allgather(send_nums, num_elements, MPI_INT, recv_nums, num_elements, MPI_INT, MPI_COMM_WORLD);
        // Synchronize again before obtaining final time
        MPI_Barrier(MPI_COMM_WORLD);
        total_my_gather_time += MPI_Wtime();
        /*
        printf("my process, proc %d recv ", rank);
        for (int j=0; j<nprocs*num_elements; j++){
            printf("%d ", recv_nums[j]);
        }
        printf("\n");
        */

        // Time MPI_Allgather
        MPI_Barrier(MPI_COMM_WORLD);
        total_mpi_gather_time -= MPI_Wtime();
        MPI_Allgather(send_nums, num_elements, MPI_INT, recv_nums, num_elements, MPI_INT, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        total_mpi_gather_time += MPI_Wtime();
        /*
        printf("mpi process, proc %d recv ", rank);
        for (int j=0; j<nprocs*num_elements; j++){
            printf("%d ", recv_nums[j]);
        }
        printf("\n");
        */
    }

    MPI_Barrier(MPI_COMM_WORLD);
    // Print off timing information
    if (rank == 0) {
        printf("Data size = %d, Trials = %d\n", num_elements * (int)sizeof(int),
            num_trials);
        printf("Avg my_allgather time = %lf seconds\n", total_my_gather_time / num_trials);
        printf("Avg MPI_Allgather time = %lf seconds\n", total_mpi_gather_time / num_trials);
    }

    free(send_nums);
    free(recv_nums);

    MPI_Finalize();
}

