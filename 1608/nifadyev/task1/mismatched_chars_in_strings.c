#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Task # 27: Count the number of mismatched characters in two strings

int main(int argc, char **argv)
{
    int MAX_SIZE;
    char *str1;
    char *str2;
    int linearResult = 0, parallelResult = 0, mismatchCount = 0;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    int procNum, procRank;
    int i;

    if (argc == 1)
    {
        MAX_SIZE = 100000000;
    }
    else
    {
        MAX_SIZE = atoi(argv[1]);
    }

    str1 = (char *)malloc(sizeof(char) * (MAX_SIZE + 1));
    str2 = (char *)malloc(sizeof(char) * (MAX_SIZE + 1));
    srand(time(NULL));
    //Fill strings with random characters
    for (i = 0; i < MAX_SIZE; i++)
    {
        str1[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
        str2[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
    }
    str1[MAX_SIZE] = '\0';
    str2[MAX_SIZE] = '\0';

    // Initialize MPI
    MPI_Init(&argc, &argv);
    // Get the number of processors
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    // Get rank of current process
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    /************Linear realization************/
    if (procRank == 0)
    {
        startTime = MPI_Wtime();
        for (i = 0; i < MAX_SIZE; i++)
        {
            if (str1[i] != str2[i])
            {
                linearResult++;
            }
        }
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;

        printf("Linear time = %.3f\n", endTime - startTime);
        printf("Linear result = %d\n", linearResult);
    }

    /************Parallel realization********/
    if (procRank == 0)
    {
        startTime = MPI_Wtime();
    }

    // Broadcast first and second strings to each processor
    MPI_Bcast(str1, MAX_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(str2, MAX_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);

    for (i = procRank; i <= MAX_SIZE; i += procNum)
    {
        if (str1[i] != str2[i])
        {
            mismatchCount++;
        }
    }

    // Collect the result into main processor
    MPI_Reduce(&mismatchCount, &parallelResult, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (procRank == 0)
    {
        endTime = MPI_Wtime();
        parallelTime = endTime - startTime;
        printf("MPI time = %.3f\n", endTime - startTime);
        printf("MPI result = %d\n", parallelResult);
        printf("Performance difference: %.1f%\n", ((linearTime - parallelTime) / ((linearTime + parallelTime) / 2)) * 100);
    }

    MPI_Finalize();
    free(str1);
    free(str2);
    return 0;
}
