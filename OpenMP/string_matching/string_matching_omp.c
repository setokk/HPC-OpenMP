#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main (int argc, char *argv[]) {
	FILE *pFile;
	long file_size, match_size, pattern_size, total_matches;
	char * buffer;
	char * filename, *pattern;
	size_t result;
	int *match;
    int n_threads_list[N_THREADS_LEN] = {2, 4, 6, 8, 10, 12, 16, 18, 20, 22};

    if (argc != 3) {
		printf ("Usage : %s <file_name> <string>\n", argv[0]);
		return 1;
    }
	filename = argv[1];
	pattern = argv[2];

	pFile = fopen ( filename , "rb" );
	if (pFile==NULL) {printf ("File error\n"); return 2;}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	file_size = ftell (pFile);
	rewind (pFile);

    #ifdef DEBUG
	printf("file size is %ld\n", file_size);
    #endif

	// allocate memory to contain the file:
	buffer = (char*) malloc (sizeof(char)*file_size);
	if (buffer == NULL) {printf ("Memory error\n"); return 3;}

	// copy the file into the buffer:
	result = fread (buffer,1,file_size,pFile);
	if (result != file_size) {printf ("Reading error\n"); return 4;}

	pattern_size = strlen(pattern);
	match_size = file_size - pattern_size + 1;

	match = (int *) malloc (sizeof(int)*match_size);
	if (match == NULL) {printf ("Malloc error\n"); return 5;}

    #ifdef PARALLEL
    printf("[" ANSI_COLOR_BLUE "INFO" ANSI_COLOR_RESET "] " "Runnning on PARALLEL mode.\n");
    printf("Disable by removing -DPARALLEL directive when compiling.\n\n");
    #else
    printf("[" ANSI_COLOR_BLUE "INFO" ANSI_COLOR_RESET "] " "Runnning on SEQUENCIAL mode.\n");
    printf("Enable PARALLEL mode by adding -DPARALLEL directive when compiling.\n\n");
    #endif

	total_matches = 0;
    for (int k=0; k<N_THREADS_LEN; ++k) {
        double start = omp_get_wtime();

        int num_threads = n_threads_list[k];

        #ifdef PARALLEL
        #pragma omp parallel for num_threads(num_threads)
        #endif
        for (long j = 0; j < match_size; ++j) {
            match[j]=0;
        }

        /* Brute Force string matching */
        #ifdef PARALLEL
        #pragma omp parallel for num_threads(num_threads) \
            reduction(+:total_matches)
        #endif
        for (long j = 0; j < match_size; ++j) {
            long i;
            for (i = 0; i < pattern_size && pattern[i] == buffer[i + j]; ++i);
                if (i >= pattern_size) {
                    match[j] = 1;
                    total_matches++;
                }
        }

        printf("\n[" ANSI_COLOR_YELLOW "NUM THREADS=%d" ANSI_COLOR_RESET "] -> time taken: %lf seconds\n",
               num_threads, (omp_get_wtime()-start));

        #ifdef DEBUG
        printf("\nTotal matches = %ld\n", total_matches);
        #endif

        // Reset
        total_matches = 0;

        #ifdef PRINT
        for (long j = 0; j < match_size; j++) {
            printf("%d", match[j]);
        }
        #endif
    }

	fclose (pFile);
	free (buffer);
	free (match);

	return 0;
}
