#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>


float *pccvector;
int **X, *y;

/*
	Helper function for solving pearson correlation coefficient vector.
	Solves for the vector values by computing for each term.
	 and stores it inside the pccvector array.
*/
void * pearson_cor_threaded_helper(void *position)
{
	int i, j, temp1, temp2, temp3, temp4, temp5, *temp;
	temp = (int *) position;
    temp1=0, temp2=0, temp3=0, temp4=0, temp5=0;

    //printf("startcol{%d}: endcol:{%d} rows:{%d}\n", temp[0], temp[1], temp[2]);
    for(i=temp[0]; i<temp[1]; i++){
		for(j=0; j<temp[2]; j++){
			temp1 += X[i][j];       
            temp2 += pow(X[i][j], 2);
            temp3 += y[j];
            temp4 += pow(y[j], 2);
            temp5 += X[i][j]*y[j];
		}

		//printf("\n%d, %d, %d, %d, %d", temp1,temp2,temp3,temp4,temp5);
		pccvector[i] = (((temp[1])*temp5) - (temp1*temp3)) / pow(((((temp[1])*temp2) - pow(temp1,2))*(((temp[1])*temp4) - pow(temp3,2))), 0.5);
	}

    printf("\n");

}

/*
	Main function for pearson correlation coefficient vector.
	Manages threading and execution of the helper function.
*/
void * pearson_cor_threaded(int m, int n, int t)
{
    int flag=0, i, j, subroutine_count=n/t, **arguments, temp;
    float *v;

    // //PRINTING 2D ARRAY VALUES
    // printf("\n\nRANDOMIZED %d by %d MATRIX\n", m,n);
    // for(i=0; i<n; i++){
    //     for(j=0; j<m; j++){
    //         printf("%d\t", X[i][j]);
    //     }
    //     printf("\n");
    // }
    

    // //PRINTING VECTOR VALUES
    // printf("\n\nRANDOMIZED 1 by %d VECTOR\n", m);
    // for(i=0; i<n; i++){
    //     printf("%d\t", y[i]);
    // }
    // printf("\n");

    // PCC VECTOR
    v = (float*)malloc(sizeof(float)*n);

    // CPU INITIALIZATION
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    // THREAD INITIALIZATION
    pthread_t tid[t];
    arguments = (int **)malloc(sizeof(int*)*t);

    temp = 0;
    for(i=0; i<t; i++){
        arguments[i] = (int*)malloc(sizeof(int)*3);
        arguments[i][0] = temp;
        arguments[i][1] = temp+subroutine_count;
        arguments[i][2] = n;
        temp += subroutine_count;
    }

    int proc_count = get_nprocs();

    // LOOP FOR CREATING THREADS
    for(i=0; i<t; i++){

        printf("thread# %d executing... \n", i+1);
        temp = i%t;
        CPU_SET(temp, &cpuset);
    	pthread_create(&tid[i], NULL, pearson_cor_threaded_helper, (void *) arguments[i]);
        pthread_setaffinity_np(tid[i], sizeof(cpu_set_t), &cpuset);

    }

    // JOINS THREADS
    for(i=0; i<t; i++){
    	pthread_join(tid[i], NULL);
    }   
}

// FREES MEMORY ALLOCATION
void deallocate(int size)
{
    for(int i=0; i<size; i++){
        free(X[i]);
    }
    free(X);
    free(y);
}


int main()
{
    FILE *fptr;
    int i, j, size, num_of_threads, temp;
    int *v;
    
    printf("Enter matrix size: ");
    scanf("%d", &size);
    printf("Enter number of threads: ");
    // scanf("%d", &num_of_threads);
    
    //allocates memory for matrix and vectors
    X = (int**)malloc(sizeof(int*)*size);
    y = (int*)malloc(sizeof(int)*size);
    pccvector = (float*)malloc(sizeof(float)*size);
    
    for(i=0; i<size; i++){
      X[i] = (int*)malloc(sizeof(int*)*size);  
    }
    
    //assigns random values to matrix and vector
    for(i=0; i<size; i++){
        y[i] = 1+rand()%1000000;
        for(j=0; j<size; j++){
            X[i][j] = 1+rand()%1000000;
        }
    }

    // TRANSPOSES THE MATRIX
	for(i=0; i<size; i++){
		for(j=0; j<i; j++){
			temp = X[i][j];
			X[i][j] = X[j][i];
			X[j][i] = temp;
		}
	}
    
    // STARTS CLOCK
    struct timeval start, end;
    gettimeofday(&start, 0);
    
    pearson_cor_threaded(size, size, num_of_threads);

    gettimeofday(&end, 0);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

    //COMPUTES TIME TAKEN TO RUN
    printf("Execution time: %.4f\n", elapsed);
    deallocate(size);
}

/*
    References:
        Measuring execution time (wall time) :
            https://levelup.gitconnected.com/8-ways-to-measure-execution-time-in-c-c-48634458d0f9
        C/C++: Set Affinity to process thread - Example Code:
            https://bytefreaks.net/programming-2/c/cc-set-affinity-to-process-thread-example-code
        22.3.5 Limiting execution to certain CPUs:
            https://www.gnu.org/software/libc/manual/html_node/CPU-Affinity.html
*/