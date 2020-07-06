# parallel

parallel-proj-requirment.txt: requirements for parallel course final projects.



## project 1: parallel programming with MPI



###### project1/allgather

my_allgather.c: achieve MPI_Allgather with MPI_Send and MPI_Recv. compare the performance of your implementation and the original MPI implementation.



###### project1/gemm

cannon.c: cannon algorithm for  matrix multiplication. 

conv.c: convolution and sum-pooling operation.



###### project1/wordcount

worldcount algorithm. -- copy from [this](https://github.com/giuseppeangri/mpi-parallel-words-count)



###### project1/run.sh, tips_for_run.txt：

how to run the code.



## project 2: parallel programming with OPENMP

###### project2/montecarlo

circle.c: Monte Carlo algorithm to estimate $\pi$.



###### project2/quicksort

quicksortomp.c: divide data, execute quick sort on each thread. And merge sort to sum all results of threads in parallel.



###### project2/pagerank

pagerankomp.c: page rank algorithm with coo sparse matrix to store adjacent matrix.



###### project2/run.sh, tips_for_run.txt：

how to run the code.





## project3: hadoop

###### project3/tips_for_run.txt：

how to run the code.