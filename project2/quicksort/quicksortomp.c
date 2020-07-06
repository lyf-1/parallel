#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int *nums;
int N = 1000000;


void init_array(int *A, int length){
    for(int i=0; i<length; i++){
        A[i] = (int)rand() % 100;
    }
}


void print_array(int *A, int length){
    for(int i=0; i<length; i++){
        printf("%d, ", A[i]);
    }
    printf("\n");
}


void cal_position(int num_thread, int thread_id, int num_ele, int *start, int *end){
    int a = num_ele / num_thread;
    int b = num_ele % num_thread;

    *start = a * thread_id;
    if(b > thread_id){
        *start += thread_id;
        *end = *start + a;
    }else{
        *start += b;
        *end = *start + a - 1;
    }
}


void q_sort(int *A, int l, int h){
    if(l>=h) return;

    int key = A[l];
    int i=l, j=h;

    while(i<j){
        while(A[j]>=key && i<j) j--;
        if(i<j) A[i++] = A[j];
        while(A[i]<=key && i<j) i++;
        if(i<j) A[j--] = A[i];
    }
    // print_array(nums, N);
    A[i] = key;
    q_sort(A, l, i-1);
    q_sort(A, i+1, h);
}


void merge_sort(int *A, int s, int m, int e){
    int size = e - s + 1;
    int *tmp = (int *)malloc(sizeof(int) * size);

    int i = s, j = m, idx = 0;
    while(i<m && j<=e){
        if(A[i] < A[j]){
            tmp[idx++] = A[i++];
        }
        else{
            tmp[idx++] = A[j++];
        }
    }

    while(i<m){
        tmp[idx++] = A[i++];
    }
    while(j<=e){
        tmp[idx++] = A[j++];
    }

    for(int k=0; k<size; k++){
        A[s+k] = tmp[k];
    }
}


int main(int argc, char* argv[]){
    double run_time = 0.;
    int threads_num = 4;
    if(argc==2){
        threads_num = atoi(argv[1]);
    }

    printf("1: Threads num = %d\n", threads_num);
    omp_set_num_threads(threads_num);
    nums = (int *)malloc(sizeof(int) * N);
    init_array(nums, N);    
    
    // printf("original array: \n");
    // print_array(nums, N);
    
    // parallel quick sort begin
    run_time -= omp_get_wtime();

    // divide data into each thread. parallel execute q_sort()
    int nth; // threads number
    #pragma omp parallel shared(nums, N, nth)
    {
        int tid = omp_get_thread_num(); // thread id
        
        #pragma omp master
            nth = omp_get_num_threads();  // master thread gets the number of threads

        #pragma omp barrier
        int array_start, array_end;
        cal_position(nth, tid, N, &array_start, &array_end);
        q_sort(nums, array_start, array_end);
        printf("Thread# %d, array start=%d, array end=%d\n", tid, array_start, array_end);
    }

    // merge results
    int cnt = 1;
    while(cnt < nth){
        cnt *= 2;

        #pragma omp parallel for
        for(int i=0; i<nth; i=i+cnt){
            int m = i + cnt/2, y = i + cnt -1;
            if(i+cnt > nth) y = nth - 1;
            if(i != y) {
                int s0, e0, s1, e1, s2, e2;
                cal_position(nth, i, N, &s0, &e0);
                cal_position(nth, m, N, &s1, &e1);
                cal_position(nth, y, N, &s2, &e2);
                merge_sort(nums, s0, s1, e2);
            }

        }
    }
    run_time += omp_get_wtime();

    // printf("Sorted array: \n");
    // print_array(nums, N);
    printf("data array size: %d, thread number: %d, Running time: %f\n", N, threads_num, run_time);
}

/*
N = 10000
qsort-sequential: 0.003769s
openmp: threads_num = 16,  0.009050s

N=1000000
seq: 10.364302s
openmp: 0.133703s nth=16
*/