#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 128
#define base 0

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0)
            printf("Usage : %s <file_name>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    char *filename = argv[1];
    FILE *pFile = fopen(filename, "rb");
    if (pFile == NULL) {
        if (rank == 0)
            printf("File error\n");
        MPI_Finalize();
        return 2;
    }

    fseek(pFile, 0, SEEK_END);
    long file_size = ftell(pFile);
    rewind(pFile);
    if (rank == 0)
        printf("file size is %ld\n", file_size);

    long local_file_size = file_size / size;
    char *local_buffer = (char *)malloc(sizeof(char) * local_file_size);
    if (local_buffer == NULL) {
        if (rank == 0)
            printf("Memory error\n");
        MPI_Finalize();
        return 3;
    }

    fseek(pFile, rank * local_file_size, SEEK_SET);
    fread(local_buffer, 1, local_file_size, pFile);

    fclose(pFile);

    int *freq = (int *)calloc(N, sizeof(int));

    for (int i = 0; i < local_file_size; i++) {
        freq[local_buffer[i] - base]++;
    }

    int *global_freq = (int *)calloc(N, sizeof(int));
    MPI_Reduce(freq, global_freq, N, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int j = 0; j < N; j++) {
            printf("%d = %d\n", j + base, global_freq[j]);
        }
    }

    free(local_buffer);
    free(freq);
    free(global_freq);

    MPI_Finalize();
    return 0;
}
