#include <math.h>
#include "shell_sort.h"

//TODO: move parallel code into separate function
//TODO: to consider remainder need to use SCATTERV
//TODO: if possible get rid of pow and log10
//TODO: move parallel resulting array from subArrayy to parallelResultingArray
//FIXME: not working if procNum % 2 == 1

int main(int argc, char **argv)
{
    int procRank, procNum;
    int i, j;
    int length;
    int *array;
    int *linearResultingArray, *parallelResultingArray;
    int *subArrayPerProc;
    int *mergedArray;
    int *buffer;
    double startTime, endTime;
    int elementsPerProc;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0)
    {
        length = (argc == 2) ? atoi(argv[1]) : 20;

        linearResultingArray = (int *)malloc(sizeof(int) * length);
        GenerateArray(linearResultingArray, length);
    }

    MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD);
    parallelResultingArray = (int *)malloc(sizeof(int) * length);

    if (procRank == 0)
    {
        if (length < 30)
        {
            PrintArray(linearResultingArray, length);
        }

        // parallelResultingArray = (int *)malloc(sizeof(int) * length);
        CopyArray(linearResultingArray, parallelResultingArray, length);

        startTime = MPI_Wtime();
        ShellSort(linearResultingArray, length);
        endTime = MPI_Wtime();

        if (length < 30)
        {
            PrintArray(linearResultingArray, length);
        }
        printf("Linear time: %.4f\n", endTime - startTime);

        elementsPerProc = length / procNum;

        free(linearResultingArray);
    }

    MPI_Bcast(&elementsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
    subArrayPerProc = (int *)malloc(sizeof(int) * elementsPerProc);

    MPI_Scatter(parallelResultingArray, elementsPerProc, MPI_INT,
                subArrayPerProc, elementsPerProc, MPI_INT, 0,
                MPI_COMM_WORLD);

    ShellSort(subArrayPerProc, elementsPerProc);

    // int iteration = log10(procNum) / log10(2);
    int iteration = procNum / 2;
    if (procRank == 0)
    {
        printf("iteration: %d\n", iteration);
    }

    for (i = 0; i < iteration; i++)
    {
        //TODO: what does it do?
        if ((procRank % ((int)pow(2, i + 1))) == 0)
        {
            printf("i: %d, pow: %d\n", i, (int)pow(2, i + 1));
            mergedArray = (int *)malloc(sizeof(int) * elementsPerProc * 2);
            buffer = (int *)malloc(sizeof(int) * elementsPerProc);

            //FIXME: if procNum % 2 == 1, source procRank == procNum, thats incorrect
            MPI_Recv(buffer, elementsPerProc, MPI_INT, procRank + (int)pow(2, i), i, MPI_COMM_WORLD, &status);

            Merge(subArrayPerProc, elementsPerProc, buffer, elementsPerProc, mergedArray);

            free(subArrayPerProc);

            subArrayPerProc = (int *)malloc(sizeof(int) * elementsPerProc * 2);
            // subArrayPerProc = realloc(subArrayPerProc, sizeof(int) * elementsPerProc * 2);
            CopyArray(mergedArray, subArrayPerProc, elementsPerProc * 2);

            elementsPerProc *= 2;

            free(mergedArray);
            free(buffer);
        }
        else if (procRank % (int)pow(2, i) == 0)
        {
            printf("i: %d, pow: %d\n", i, (int)pow(2, i));
            MPI_Send(subArrayPerProc, elementsPerProc, MPI_INT, procRank - (int)pow(2, i), i, MPI_COMM_WORLD);
        }
    }

    if (procRank == 0)
    {
        printf("subArray: ");
        PrintArray(subArrayPerProc, elementsPerProc);
        printf("\nArray: ");
        PrintArray(parallelResultingArray, length);

        // AreResultsEqual(linearResultingArray, parallelResultingArray, length);

        free(parallelResultingArray);
    }

    free(subArrayPerProc);

    MPI_Finalize();
    return 0;
}
