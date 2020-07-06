#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>


int main(int argc, char* argv[]){
    int max = 1000000, count = 0;
    double x, y, area, run_time = 0.;
    int threads_num = 4;

    if(argc>=2){
        threads_num = atoi(argv[1]);
    }

    run_time -= omp_get_wtime();
    srand((unsigned int)time(NULL));
    omp_set_num_threads(threads_num);
    
    #pragma omp parallel for private(x, y) reduction(+:count)
        for(int i=0; i<max; i++){
            // printf("%d\n", i);
            x = (double)rand() / (RAND_MAX);
            y = (double)rand() / (RAND_MAX);
            if((x*x + y*y) <= 1.) count ++;
        }
        
    area = 4 * ((double)count / max);
    run_time += omp_get_wtime();

    printf("Threads number is %d\n", threads_num);
    printf("Radius = 1, circle Area = %f\n", area);
    printf("Running time: %f\n", run_time);
    
    return 0;
}