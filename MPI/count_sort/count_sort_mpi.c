#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stddef.h>

#define UPPER 1000
#define LOWER 0

typedef struct {
    int currIndex;
    int *indices;
    int *xIndices;
} SortedIndices;

SortedIndices* new_SortedIndices(int local_n);
void create_mpi_sorted_indices(MPI_Datatype *mpi_sorted_indices, int local_n);

int main(int argc, char *argv[])
{
   int i, j, my_num, my_place, world_rank, world_size, *y, *x;

   if (argc != 2) {
		printf ("Usage : %s <array_size>\n", argv[0]);
		return 1;
   }

   int n = strtol(argv[1], NULL, 10);
   y = ( int * ) malloc ( n * sizeof ( int ) );

   // MPI init
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &world_size);
   x = ( int * ) malloc ( n * sizeof ( int ) );
   for (i=0; i<n; i++)
		x[i] = n - i;

   // Define struct in MPI
   int local_n = n / world_size;
   MPI_Datatype mpi_sorted_indices;
   create_mpi_sorted_indices(&mpi_sorted_indices);

   SortedIndices* sortedIndices = new_SortedIndices(local_n);
   for (j=rank * local_n; j<(rank + 1) * local_n; j++) {

     my_num = x[j];
     my_place = 0;
     for (i=0; i<n; i++)
		if ((my_num > x[i]) || ((my_num == x[i]) && (j < i)))
			my_place++;
     sortedIndices->indices[sortedIndices->currIndex] = my_place;
     sortedIndices->xIndices[sortedIndices->currIndex++] = j;
   }

   MPI_Send(sortedIndices, 1, mpi_sorted_indices, 1, 0, MPI_COMM_WORLD);

   if (world_rank == 0) {
        SortedIndices result;
        for (int k=1; k<world_size; k++) {
            MPI_Recv(&result, 1, mpi_sorted_indices, k, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int l=0; l<local_n; l++) {
                y[result->indices[l]] = x[result->xIndices[l]];
            }
        }

    for (i=0; i<n; i++)
		printf("%d\n", y[i]);
   }

    MPI_Type_free(&mpi_custom_struct);
    MPI_Finalize();

   return 0;
}

SortedIndices* new_SortedIndices(int local_n) {
    SortedIndices* sortedIndices = (SortedIndices *) malloc(sizeof(SortedIndices));
    sortedIndices->currIndex = 0;
    sortedIndices->indices = (int*) malloc(local_n * sizeof(int));
    sortedIndices->xIndices = (int*) malloc(local_n * sizeof(int));

    return sortedIndices;
}

void create_mpi_sorted_indices(MPI_Datatype *mpi_sorted_indices, int local_n) {
    // Number of elements in the struct
    int count = 3;

    // Array of block lengths
    int blocklengths[3] = {1, local_n, local_n};

    // Array of displacements
    MPI_Aint displacements[3];
    displacements[0] = offsetof(SortedIndices, currIndex);
    displacements[1] = offsetof(SortedIndices, indices);
    displacements[3] = offsetof(SortedIndices, xIndices);

    // Array of types
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};

    // Create the MPI struct type
    MPI_Type_create_struct(count, blocklengths, displacements, types, mpi_sorted_indices);
    MPI_Type_commit(mpi_sorted_indices);
}
