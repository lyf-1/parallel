#!/bin/bash
chmod 777 ./gather/my_allgather
chmod 777 ./gemm/cannon
chmod 777 ./gemm/conv

# 1 - name
# 2 - num of procs
# 3 - num of elements
# 4 - num of trials
if test $1 = "gather"; then
    mpirun -np $2 ./gather/my_allgather $3 $4

# 3 - matrix dimension
elif test $1 = "matrixmat"; then
    mpiexec -n $2 ./gemm/cannon $3
elif test $1 = "matrixconv"; then
    mpiexec -n $2 ./gemm/conv $3 0
elif test $1 = "matrixpool"; then
    mpiexec -n $2 ./gemm/conv $3 1
elif test $1 = "wc_big"; then
    echo "you need to go to mpi_wordcount/src folder and run 'mpiexec -n <process_num> ./main 0'"
elif test $1 = "wc_small"; then
    echo "you need to go to mpi_wordcount/src folder and run 'mpiexec -n <process_num> ./main 1'"
fi
