#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

// ANSI color codes for text colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define N 128
#define base 0
#define PAD 8
#define N_THREADS_LEN 10

typedef struct {
	int *freq;
} LocalFreq;

LocalFreq* create_local_freq();


int main (int argc, char *argv[]) {
	FILE *pFile;
	long file_size;
	char *buffer;
	char *filename;
	size_t result;
	int freq[N];
	int n_threads_list[N_THREADS_LEN] = {2, 4, 6, 8, 10, 12, 16, 18, 20, 22};

    if (argc != 2) {
		printf ("Usage : %s <file_name>\n", argv[0]);
        return 1;
    }
	filename = argv[1];
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

    #ifdef PARALLEL
    printf("[" ANSI_COLOR_BLUE "INFO" ANSI_COLOR_RESET "] " "Runnning on PARALLEL mode.\n");
    printf("Disable by removing -DPARALLEL directive when compiling.\n\n");
    #else
    printf("[" ANSI_COLOR_BLUE "INFO" ANSI_COLOR_RESET "] " "Runnning on SEQUENCIAL mode.\n");
    printf("Enable PARALLEL mode by adding -DPARALLEL directive when compiling.\n\n");
    #endif

	for (int k=0; k<N_THREADS_LEN; k++){
		double start = omp_get_wtime();

		int num_threads = n_threads_list[k];
		#pragma omp parallel for num_threads(num_threads)
		for (int i=0; i<N; i++){
			freq[i]=0;
		}

		LocalFreq* local_freq[num_threads][PAD];
		for (int id=0; id<num_threads; id++){
			local_freq[id][0] = create_local_freq(freq);
		}

		#pragma omp parallel num_threads(num_threads)
		{
			int id = omp_get_thread_num();
			#pragma omp for
			for (long i=0; i<file_size; i++){
				local_freq[id][0]->freq[buffer[i] - base]++;
			}
		}

		// Aggregate local frequencies to the final array
		for (int id=0; id<num_threads; id++){
			for (int j=0; j<N; j++){
				freq[j] += local_freq[id][0]->freq[j];
			}
			// Cleanup
			free(local_freq[id][0]->freq);
			free(local_freq[id][0]);
		}

		printf("\n[" ANSI_COLOR_YELLOW "NUM THREADS=%d" ANSI_COLOR_RESET "] -> time taken: %lf seconds\n",
               num_threads, (omp_get_wtime()-start));
	}

	#ifdef DEBUG
	for (int j=0; j<N; j++){
		printf("%d = %d\n", j+base, freq[j]);
	}
	#endif

	fclose (pFile);
	free (buffer);

	return 0;
}

LocalFreq* create_local_freq(int freq[]) {
	LocalFreq *local_freq = (LocalFreq *) malloc(sizeof(LocalFreq));
	local_freq->freq = (int*) malloc(N * sizeof(int));
    if (local_freq->freq != NULL) {
        memcpy(local_freq->freq, freq, N * sizeof(int));
    }
	return local_freq;
}
