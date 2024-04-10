#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

// ANSI color codes for text colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PAD 8
#define N_THREADS_LEN 10

void main (int argc, char *argv[]) {
    int N;
    float *x, *b, **a;
    char any;
    int n_threads_list[N_THREADS_LEN] = {2, 4, 6, 8, 10, 12, 16, 18, 20, 22};

	if (argc != 2) {
		printf ("Usage : %s <matrix size>\n", argv[0]);
        exit(1);
	}
	N = strtol(argv[1], NULL, 10);

	/* Allocate space for matrices */
	a = (float **) malloc ( N * sizeof ( float *) );
	for (int i = 0; i < N; i++)
		a[i] = ( float * ) malloc ( (i+1) * sizeof ( float ) );
	b = ( float * ) malloc ( N * sizeof ( float ) );
	x = ( float * ) malloc ( N * sizeof ( float ) );

	/* Create floats between 0 and 1. Diagonal elents between 2 and 3. */
	srand ( time ( NULL));
	for (int i = 0; i < N; i++) {
		x[i] = 0.0;
		b[i] = (float)rand()/(RAND_MAX*2.0-1.0);
		a[i][i] = 2.0+(float)rand()/(RAND_MAX*2.0-1.0);
		for (int j = 0; j < i; j++)
			a[i][j] = (float)rand()/(RAND_MAX*2.0-1.0);;
	}

    #ifdef PARALLEL
    printf("[" ANSI_COLOR_BLUE "INFO" ANSI_COLOR_RESET "] " "Runnning on PARALLEL mode.\n");
    printf("Disable by removing -DPARALLEL directive when compiling.\n\n");
    #else
    printf("[" ANSI_COLOR_BLUE "INFO" ANSI_COLOR_RESET "] " "Runnning on SEQUENCIAL mode.\n");
    printf("Enable PARALLEL mode by adding -DPARALLEL directive when compiling.\n\n");
    #endif

	for (int k=0; k<N_THREADS_LEN; ++k) {
        double start = omp_get_wtime();

        int num_threads = n_threads_list[k];

        /* Calulation */
        #ifdef PARALLEL
        #pragma omp parallel for num_threads(num_threads)
        #endif
        for (int i = 0; i < N; i++) {
            double sum = 0.0;
            for (int j = 0; j < i; j++) {
                sum += (x[j] * a[i][j]);
                //printf ("%d %d %f %f %f \t \n", i, j, x[j], a[i][j], sum);
            }
            x[i] = (b[i] - sum) / a[i][i];
            //printf ("%d %f %f %f %f \n", i, b[i], sum, a[i][i], x[i]);
        }

        printf("\n[" ANSI_COLOR_YELLOW "NUM THREADS=%d" ANSI_COLOR_RESET "] -> time taken: %lf seconds\n",
               num_threads, (omp_get_wtime()-start));

        /* Print result */
        #ifdef DEBUG
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < i; j++)
                printf ("%f \t", a[i][j]);
            printf ("%f \t%f\n", x[i], b[i]);
        }
        #endif

        /* Check result */
        #ifdef VALIDATE
        int success = 1;
        for (int i = 0; i < N; i++) {
            double sum = 0.0;
            for (int j = 0; j < i; j++)
                sum += (x[j]*a[i][j]);
            if (fabsf(sum - b[i]) > 0.00001) {
                //printf("%f != %f\n", sum, b[i]);
                success = 0;
            }
        }
        if (success)
            printf(ANSI_COLOR_RED "Validation Failed...\n" ANSI_COLOR_RESET);
        else
            printf(ANSI_COLOR_GREEN "Validation Succeeded...\n" ANSI_COLOR_RESET);
        #endif
    }
}
