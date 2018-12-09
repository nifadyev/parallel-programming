// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// #include <mpich/mpi.h>
#include "shell_sort.h"


int main(int argc, char **argv)
{
    int procRank, procNum;
    int i, j;
    int length;
    int *array;
    int *linearResultingArray, *parallelResultingArray;
    double startTime, endTime;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0)
    {
        length = (argc == 2) ? atoi(argv[1]) : 20;

        linearResultingArray = (int *)malloc(sizeof(int) * length);
        GenerateArray(linearResultingArray, length);
        // CopyArray(linearResultingArray, parallelResultingArray, length);

        if (length < 30)
        {
            PrintArray(linearResultingArray, length);
        }

        startTime = MPI_Wtime();
        ShellSort(linearResultingArray, length);
        endTime = MPI_Wtime();

        if (length < 30)
        {
            PrintArray(linearResultingArray, length);
        }
        printf("Linear time: %.4f\n", endTime - startTime);
        free(linearResultingArray);
    }

    return 0;
}
