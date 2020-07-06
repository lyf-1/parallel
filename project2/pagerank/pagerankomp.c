#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>


void double_array_init(double *A, int length, double ele){
    for(int i=0; i<length; i++)
        A[i] = ele;
}


void print_array(double *A, int length){
    for(int i=0; i<length; i++)
        printf("%f ", A[i]);
    printf("\n");
}


int cmp(const void * a, const void * b){
    return((*(double*)a - *(double*)b>0)?-1:1);
}


int main(int argc, char* argv[]){
    int rand_edges = 10, iteration=100, n_nodes, n_edges, sum=0;
    int *rowptr, *colidx, *edge_cnt;
    double damp1 = 0.85, damp2=1-damp1;
    double *val, *new_pr, *old_pr;
    double start, start1, finish;
    int threads_num;

    if(argc==3){
        n_nodes = atoi(argv[1]);
        threads_num = atoi(argv[2]);
    }
    else{
        n_nodes = 1024000;
        threads_num = 4;
        printf("Usage: ./pagerank <num_nodes> <num_threads>\n");
        printf("Default: num_nodes=%d, num_threads=%d\n", n_nodes, threads_num);
    }
    omp_set_num_threads(threads_num);
    start = omp_get_wtime();

    // random generate edges num
    edge_cnt = (int *)malloc(sizeof(int) * n_nodes);
    for(int i=0; i<n_nodes; i++){
        edge_cnt[i] = rand() % 10 + 1;;    // the edge count of each node ranges from 1 to 10
        sum += edge_cnt[i];
    }

    val = (double *)malloc(sizeof(double) * sum);
    colidx = (int *)malloc(sizeof(int) * sum);
    rowptr = (int *)malloc(sizeof(int) * (n_nodes+1));
    new_pr = (double *)malloc(sizeof(double) * n_nodes);
    old_pr = (double *)malloc(sizeof(double) * n_nodes);
    double_array_init(old_pr, n_nodes, 1.);
   
    // random init graph, use coo_row to store sparse adj matrix
    for(int i=0; i<n_nodes; i++){
        rowptr[i] = i==0 ? 0 : rowptr[i-1]+edge_cnt[i-1];
        for(int j=0; j<edge_cnt[i]; j++){
            int idx = rowptr[i] + j;
            int tmp = rand() % n_nodes;
            colidx[idx] = tmp;  
            val[idx] = 1./edge_cnt[i];
        }    
    }
    rowptr[n_nodes] = sum;

    start1 = omp_get_wtime();


    // iteration pagerank
    for(int it=0; it<iteration; it++){
        
        double_array_init(new_pr, n_nodes, 0);
        #pragma omp parallel for
        for(int i=0; i<n_nodes; i++) {
            for(int j=rowptr[i]; j<rowptr[i+1]; j++){
                new_pr[colidx[j]] += val[j] * old_pr[i];
            }
        }

        double rank_sum = 0;
        #pragma omp parallel for reduction(+:rank_sum)
        for(int i=0; i<n_nodes; i+=1){
            rank_sum += old_pr[i];
        }

        #pragma omp parallel for
        for(int i = 0; i<n_nodes; i++) {
            old_pr[i] = new_pr[i]*damp1 + damp2*rank_sum/n_nodes;
        }
    }
    finish = omp_get_wtime();

    qsort(old_pr, n_nodes, sizeof(double), cmp);
    
    printf("num nodes: %d, iteration: %d, num threads: %d\n", n_nodes, iteration, threads_num);
    printf("init pagerank: all elements are 1.\n");
    int show_num = n_nodes<50 ? n_nodes:50;
    printf("Top-%d  pagerank value:\n", show_num);
    print_array(old_pr, show_num);    
    printf("Init graph running time:%f\n", start1-start);
    printf("Pagerank running time:%f\n", finish-start1);
    printf("Total running time: %f\n", finish-start);
}


/*
Rst:
Pagerank:

Init graph running time: 0.285890
Pagerank running time: 20.730838
Total running time: 21.016728


omp:
4
Init graph running time:0.287740
Pagerank running time:8.687361
Total running time: 8.975101

8
Init graph running time:0.253703
Pagerank running time:4.691297
Total running time: 4.945000

16
Init graph running time:0.318334
Pagerank running time:3.427925
Total running time: 3.746259

32
Init graph running time:0.281477
Pagerank running time:3.635472
Total running time: 3.916949


*/