#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define UPPER 1000
#define LOWER 0
#define ROOT 0


int main(int argc, char *argv[])
{
    int rank, size;
    int *x, *y, *local_x, *local_y;
    int i, j, my_num, my_place, local_size, n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        if (rank == ROOT)
        {
            printf("Usage : %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    n = strtol(argv[1], NULL, 10);
    local_size = n / size;
    x = (int *)malloc(n * sizeof(int));
    y = (int *)malloc(n * sizeof(int));
    local_x = (int *)malloc(local_size * sizeof(int));
    local_y = (int *)malloc(local_size * sizeof(int));
[
    if (rank == ROOT)
    {
        for (i = 0; i < n; i++)
            x[i] = n - i;
        // x[i] = (rand() % (UPPER - LOWER + 1)) + LOWER;
    }

    // Scatter to all processes
    MPI_Scatter(x, local_size, MPI_INT, local_x, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    for (j = 0; j < local_size; j++)
    {
        my_num = local_x[j];
        my_place = 0;
        for (i = 0; i < local_size; i++)
            if ((my_num > local_x[i]) || ((my_num == local_x[i]) && (j < i)))
                my_place++;
        local_y[my_place] = my_num;
    }

    // Gather sorted subarrays
    MPI_Gather(local_y, local_size, MPI_INT, y, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Print if array is properly sorted
    if (rank == ROOT) {
        for (i = 0; i < n; i++) {
            if (i%3==0) printf("\t\ty[%d]=%d\n",   i, y[i]);
            else        printf("\t\ty[%d]=%d -> ", i, y[i]);
        }
    }

    // Clean memory
    free(x);
    free(y);
    free(local_x);
    free(local_y);

    MPI_Finalize();

    return 0;
}
