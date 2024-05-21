#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 300
#define T 10

void initializeTemperature(double h[2][N][N], int world_rank);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int i, j, current = 0, nextCurrent = 1, iterations;

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    double h[2][N][N]; // Temperature matrix for each time step
    double local_h[2][N][N / world_size]; // Local portion of temperature matrix for each process

    // Initialize arrays with zero and set boundary conditions
    initializeTemperature(h, world_rank);

    // Calculate local range for each process
    int local_start = world_rank * (N / world_size);
    int local_end = (world_rank + 1) * (N / world_size);

    // Perform iterations
    for (iterations = 0; iterations < T; iterations++) {
        // Non-blocking communication for boundary exchange
        MPI_Request send_requests[2], recv_requests[2];

        if (world_rank > 0) {
            MPI_Irecv(&h[current][local_start - 1][0], N, MPI_DOUBLE, world_rank - 1, 0, MPI_COMM_WORLD, &recv_requests[0]);
            MPI_Isend(&h[current][local_start][0], N, MPI_DOUBLE, world_rank - 1, 0, MPI_COMM_WORLD, &send_requests[0]);
        }
        if (world_rank < world_size - 1) {
            MPI_Irecv(&h[current][local_end][0], N, MPI_DOUBLE, world_rank + 1, 0, MPI_COMM_WORLD, &recv_requests[1]);
            MPI_Isend(&h[current][local_end - 1][0], N, MPI_DOUBLE, world_rank + 1, 0, MPI_COMM_WORLD, &send_requests[1]);
        }

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

        // Wait for all non-blocking communications to complete
        if (world_rank > 0) {
            MPI_Wait(&recv_requests[0], MPI_STATUS_IGNORE);
            MPI_Wait(&send_requests[0], MPI_STATUS_IGNORE);
        }
        if (world_rank < world_size - 1) {
            MPI_Wait(&recv_requests[1], MPI_STATUS_IGNORE);
            MPI_Wait(&send_requests[1], MPI_STATUS_IGNORE);
        }

        // Update current and nextCurrent indices
        current = nextCurrent;
        nextCurrent = 1 - current;
    }

    // Gather results from all processes to h[0]
    MPI_Gather(&h[current][local_start][0], N * (N / world_size), MPI_DOUBLE,
               &h[current][local_start][0], N * (N / world_size), MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    // Print final temperatures from process 0
    if (world_rank == 0) {
        printf("Final Temperatures (After Parallel Execution) [N=%d, T=%d]:\n", N, T);
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

void initializeTemperature(double h[2][N][N], int world_rank) {
    // Initialize boundary conditions
        if (world_rank == 0) {
            int i, j;
            // Init
            for (i = 0; i < N; i++) {
                for (j = 0; j < N; j++) {
                    h[0][i][j] = 0.0;
                    h[1][i][j] = 0.0;
                }
            }

            for (i = 0; i < N; i++) {
                h[0][i][0] = 20.0;          // Left wall
                h[0][i][N - 1] = 20.0;      // Right wall
                h[0][0][i] = 20.0;          // Top wall
                h[0][N - 1][i] = 20.0;      // Bottom wall
            }

            // Initialize fireplace in the middle of the bottom wall
            for (i = (N / 3); i < (2 * N / 3); i++) {
                h[0][N - 1][i] = 100.0;    // Fireplace temperature
            }
    }
}
