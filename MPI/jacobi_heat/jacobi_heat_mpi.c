#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void getInput(int*, int*);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    double start, end, seqTime, parTime;
    int i, j, N, current, nextCurrent, T, iterations;
    current = 0;
    nextCurrent = 1;

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    getInput(&N, &T);

    double h[2][N][N]; // Temperature matrix for each time step
    double local_h[2][N][N / world_size]; // Local portion of temperature matrix for each process

    // Initialize arrays
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            h[0][i][j] = 0.0;
            h[1][i][j] = 0.0;
        }
    }

    // Initialize walls and fireplace (similar to original code)
    // ...

    // Calculate local range for each process
    int local_start = world_rank * (N / world_size);
    int local_end = (world_rank + 1) * (N / world_size);

    // Start timer for sequential execution
    start = MPI_Wtime();

    // Perform iterations
    for (iterations = 0; iterations < T; iterations++) {
        // Exchange boundary values between neighboring processes
        // ...

        // Compute updated temperatures locally within each process
        for (i = local_start + 1; i < local_end - 1; i++) {
            for (j = 1; j < N - 1; j++) {
                h[nextCurrent][i][j] = 0.25 * (
                                            h[current][i - 1][j] +
                                            h[current][i + 1][j] +
                                            h[current][i][j - 1] +
                                            h[current][i][j + 1]
                                        );
            }
        }

        // Synchronize processes
        MPI_Barrier(MPI_COMM_WORLD);

        // Update current and nextCurrent indices
        current = nextCurrent;
        nextCurrent = 1 - current;
    }

    // End timer for sequential execution
    end = MPI_Wtime();
    seqTime = end - start;

    // Gather results from all processes to h[0]
    MPI_Allgather(local_h[nextCurrent], N * (N / world_size), MPI_DOUBLE,
                  h[nextCurrent], N * (N / world_size), MPI_DOUBLE,
                  MPI_COMM_WORLD);

    // Print final temperatures from process 0
    if (world_rank == 0) {
        printf("Final Temperatures (After Parallel Execution):\n");
        for (i = 0; i < N; i += N / 10) {
            for (j = 0; j < N; j += N / 10) {
                printf("%-.2f\t", h[current][i][j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();

    return 0;
}

void getInput(int* N, int* T) {
    if (world_rank == 0) {
        N = 300;
        T = 100;
    }

    MPI_Bcast(N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(T, 1, MPI_INT, 0, MPI_COMM_WORLD);
}
