#include "shell_sort.h"

// Task # 14: Shell sort with simple merge

int main(int argc, char **argv)
{
    int procRank, procNum;
    int length;
    int *linearResultingArray, *parallelResultingArray;
    double startTime = 0.0, endTime = 0.0;
    double linearTime = 0.0, parallelTime = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0)
    {
        length = (argc == 2) ? atoi(argv[1]) : 10000;

        linearResultingArray = (int *)malloc(sizeof(int) * length);
        GenerateArray(linearResultingArray, length);

        parallelResultingArray = (int *)malloc(sizeof(int) * length);
    }

    ParallelShellSort(linearResultingArray, length, procNum, procRank,
                        parallelTime, parallelResultingArray);

    if (procRank == 0)
    {
        startTime = MPI_Wtime();
        ShellSort(linearResultingArray, length);
        endTime = MPI_Wtime();

        linearTime = endTime - startTime;

        if (length < 30)
        {
            printf("Initial array: ");
            PrintArray(linearResultingArray, length);

            printf("Sorted array (parallel algorithm): ");
            PrintArray(parallelResultingArray, length);

            printf("Sorted array (linear algorithm):   ");
            PrintArray(linearResultingArray, length);
        }

        printf("Linear time:   %.4f\n", linearTime);
        printf("Parallel time: %.4f\n", parallelTime);

        if (AreResultsEqual(linearResultingArray, parallelResultingArray, length))
        {
            printf("Results are not equal\n");
        }

        free(linearResultingArray);
        free(parallelResultingArray);
    }

    MPI_Finalize();
    return 0;
}
