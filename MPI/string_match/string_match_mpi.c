#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    FILE *pFile;
    long file_size, pattern_size, local_size, match_size, total_matches;
    char *buffer, *local_buffer, *filename, *pattern;
    size_t result;
    int i, j, local_matches;

    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: %s <file_name> <string>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    filename = argv[1];
    pattern = argv[2];

    if (rank == 0) {
        pFile = fopen(filename, "rb");
        if (pFile == NULL) {
            printf("File error\n");
            MPI_Finalize();
            return 2;
        }

        fseek(pFile, 0, SEEK_END);
        file_size = ftell(pFile);
        rewind(pFile);
        printf("File size is %ld\n", file_size);

        buffer = (char*) malloc(sizeof(char) * file_size);
        if (buffer == NULL) {
            printf("Memory error\n");
            fclose(pFile);
            MPI_Finalize();
            return 3;
        }

        result = fread(buffer, 1, file_size, pFile);
        if (result != file_size) {
            printf("Reading error\n");
            free(buffer);
            fclose(pFile);
            MPI_Finalize();
            return 4;
        }
        fclose(pFile);
    }

    MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    pattern_size = strlen(pattern);
    match_size = file_size - pattern_size + 1;
    local_size = match_size / size;
    if (rank < match_size % size) {
        local_size++;
    }

    local_buffer = (char*) malloc(sizeof(char) * (local_size + pattern_size - 1));
    if (rank == 0) {
        for (int proc = 0; proc < size; proc++) {
            long start = proc * (match_size / size);
            if (proc < match_size % size) {
                start += proc;
            } else {
                start += match_size % size;
            }
            long end = start + (match_size / size);
            if (proc < match_size % size) {
                end++;
            }

            if (proc == 0) {
                memcpy(local_buffer, buffer + start, local_size + pattern_size - 1);
            } else {
                MPI_Send(buffer + start, end - start + pattern_size - 1, MPI_CHAR, proc, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        MPI_Recv(local_buffer, local_size + pattern_size - 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    local_matches = 0;
    for (j = 0; j < local_size; ++j) {
        for (i = 0; i < pattern_size && pattern[i] == local_buffer[i + j]; ++i);
        if (i >= pattern_size) {
            local_matches++;
        }
    }

    MPI_Reduce(&local_matches, &total_matches, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total matches = %ld\n", total_matches);
        free(buffer);
    }
    free(local_buffer);

    MPI_Finalize();
    return 0;
}
