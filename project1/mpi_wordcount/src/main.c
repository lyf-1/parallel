#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include <mpi_utils.h>
#include <file_utils.h>

#include <file_information.h>
#include <file_information_container.h>
#include <file_information_container_mpi.h>

#include <counter.h>
#include <counter_container.h>
#include <counter_container_mpi.h>

#include <file_reader.h>

int main(int argc, char * argv[]) {

// MPI INIZIALIZE

	int my_rank;
	int master_rank;
	int num_processes;
	double timeStart, timeEnd;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
	
	master_rank = 0;
	timeStart = MPI_Wtime();

	if (argc!=2){
		if(my_rank == 0){
			printf("Usage: mpiexec -n <process num> ./<execute file> <0 or 1 (0 is big file, 1 is small file)> \n");
		}
		MPI_Finalize();
		exit(EXIT_SUCCESS);
	}
	int flag = atoi(argv[1]);
	
	// Log File
	// char * log_file_name;
	// FILE * log_file;

	// if (my_rank == master_rank){
		// if (flag == 0){
		// 	asprintf(&log_file_name, "../logs/big_rst.txt");			
		// }
		// else
		// {
		// 	asprintf(&log_file_name, "../logs/small_rst.txt");
		// }
	// 	log_file = openFile(log_file_name, "w");
	// 	fprintf(log_file, "MPI Parallel Words Count\n");
	// 	fprintf(log_file, "Number of Porcesses %d \n", num_processes);
	// 	fprintf(log_file, "Rank %d (MASTER)\n\n", my_rank);
	// 	printf("MPI Parallel Words Count\n");
	// }
	
	// Get information about all files to read
	FileInformationContainer filesContainer;
	int filesContainerPackSize = 0;

	if(my_rank == master_rank) {
		if (flag == 0){
			filesContainer = FileInformationContainer_buildByMasterFile("../data/master_big.txt");
		}
		else
		{
			filesContainer = FileInformationContainer_buildByMasterFile("../data/master_small.txt");
		}
		
		filesContainerPackSize = FileInformationContainer_calculateSendPackSize(&filesContainer);
		// printf("size=%d\n", filesContainerPackSize);
	}
	else {
		filesContainer = FileInformationContainer_constructor();
	}
	
	MPI_Bcast(&filesContainerPackSize, 1, MPI_INT, master_rank, MPI_COMM_WORLD);

	void * filesContainerBuffer = NULL;
	
	if(my_rank == master_rank) {
		filesContainerBuffer = FileInformationContainer_makeSendPackBuffer(&filesContainer, filesContainerPackSize);
	}
	else {
		filesContainerBuffer = malloc(filesContainerPackSize);
	}

	MPI_Bcast(filesContainerBuffer, filesContainerPackSize, MPI_PACKED, master_rank, MPI_COMM_WORLD);

	if(my_rank != master_rank) {
		FileInformationContainer_unpack(&filesContainer, filesContainerBuffer, filesContainerPackSize);
	}

	// Calculate split size that each process will have to read
	double split_size = (int) (filesContainer.total_size / num_processes);

	// if(my_rank == master_rank) {
	// 	fprintf(log_file, "Number of files: %d\n", filesContainer.num_files);
	// 	fprintf(log_file, "Total size of files: %.2f bytes\n", filesContainer.total_size);
	// 	fprintf(log_file, "Single split size: %f bytes\n\n", split_size);
	// }
	
	// Words Count Process

	CounterContainer entriesContainer = CounterContainer_constructor();

	startReader(my_rank, split_size, &filesContainer, &entriesContainer);

	// Calculate Send Pack Size

	int sendPackSize = 0;

    if(my_rank != master_rank) {
		sendPackSize = CounterContainer_calculateSendPackSize(&entriesContainer);
	}

    // Gather Send Pack Size

    int packsSizes[num_processes];

	MPI_Gather(&sendPackSize, 1, MPI_INT, packsSizes, 1, MPI_INT, master_rank, MPI_COMM_WORLD);

	// Allocate Recv Pack Buffer and Calculate Displacements

	int     recvBufferSize;
	void *  recvBuffer = NULL;
	int 	displacements[num_processes];

	if(my_rank == master_rank) {
		recvBufferSize = CounterContainer_calculateRecvPacksBufferSize(num_processes, master_rank, packsSizes);
		recvBuffer = malloc(recvBufferSize);
		CounterContainer_calculateDisplacements(num_processes, master_rank, packsSizes, displacements);
	}

	// Make Send Pack Buffer

	void * packedBuffer;

	if(my_rank != master_rank) {
		packedBuffer = CounterContainer_makeSendPackBuffer(&entriesContainer, sendPackSize);
	}

	// Gather Send Pack

    MPI_Gatherv(packedBuffer, sendPackSize, MPI_PACKED, 
        recvBuffer, packsSizes, displacements, MPI_PACKED, 
        master_rank, MPI_COMM_WORLD);

    // Merge all packs

    if(my_rank == master_rank) {
        CounterContainer_merge(&entriesContainer, recvBuffer, recvBufferSize, num_processes-1);

		timeEnd = MPI_Wtime();

		// fprintf(log_file, "\nGlobal Histogram\n\n");
		// CounterContainer_printToFile(&entriesContainer, log_file);

		printf("Global Histogram\n\n");
       	CounterContainer_print(&entriesContainer);

		if (flag == 0){
			printf("One big file:\n");			
		}
		else
		{
			printf("100 small file:\n");
		}

		printf("\nExcution Time: %f seconds\n", timeEnd-timeStart);
		// fprintf(log_file, "\nExecution Time: %f seconds\n", timeEnd-timeStart);
		// fclose(log_file);
    }

    

// MPI CLOSE
	
	MPI_Finalize(); 	
	exit(EXIT_SUCCESS);

}