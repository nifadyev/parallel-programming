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
    // int *array;
    int *linearResultingArray, *parallelResultingArray;
    int *subArrayPerProc;
    int *mergedArray;
    int *buffer;
    int *other;
    int *shifts, *elemsPerProc;
    int m; // tempElementsPerProc
    int near;
    double startTime, endTime;
    int elementsPerProc, remaindedElements;
    int step;
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
    shifts = (int *)malloc(sizeof(int) * procNum);
    elemsPerProc = (int *)malloc(sizeof(int) * procNum);
    // parallelResultingArray = (int *)malloc(sizeof(int) * length);

    if (procRank == 0)
    {
        if (length < 30)
        {
            PrintArray(linearResultingArray, length);
        }

        parallelResultingArray = (int *)malloc(sizeof(int) * length);
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
        // remaindedElements = length % procNum;
        // if (remaindedElements != 0)
        // {
        //     elementsPerProc++;
        // }
        // elementsPerProc += (length % procNum != 0) ? 1 : 0;

        for (i = 0; i < procNum; i++)
        {
            shifts[i] = i * elementsPerProc;
            elemsPerProc[i] = elementsPerProc;
        }
        // shifts[procNum - 1] = elementsPerProc + length % procNum;
        elemsPerProc[procNum - 1] = elementsPerProc + length % procNum;

        free(linearResultingArray);
    }

    MPI_Bcast(shifts, procNum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(elemsPerProc, procNum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&elementsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (procRank == procNum - 1)
    {
        elementsPerProc += length % procNum;
    }
    // MPI_Bcast(&remaindedElements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    subArrayPerProc = (int *)malloc(sizeof(int) * elementsPerProc);

    // MPI_Scatter(parallelResultingArray, elementsPerProc, MPI_INT,
    //             subArrayPerProc, elementsPerProc, MPI_INT, 0,
    //             MPI_COMM_WORLD);
    MPI_Scatterv(parallelResultingArray, elemsPerProc, shifts,
                MPI_INT, subArrayPerProc, elemsPerProc[procRank],
                MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    ShellSort(subArrayPerProc, elementsPerProc);

    step = 1;
    while (step < procNum)
    {
        if (procRank % (2 * step) == 0)
        {
            if (procRank + step < procNum)
            {
                MPI_Recv(&m, 1, MPI_INT, procRank + step, 0, MPI_COMM_WORLD, &status);
                other = (int *)malloc(m * sizeof(int));
                MPI_Recv(other, m, MPI_INT, procRank + step, 1, MPI_COMM_WORLD, &status);
                // printf("%d received: ", step);
                // PrintArray(other, m);
                // PrintArray(subArrayPerProc, elementsPerProc);
                subArrayPerProc = Merge(subArrayPerProc, elementsPerProc, other, m);
                // PrintArray(subArrayPerProc, elementsPerProc);
                elementsPerProc += m;
            }
        }
        else
        {
            near = procRank - step;
            MPI_Send(&elementsPerProc, 1, MPI_INT, near, 0, MPI_COMM_WORLD);
            MPI_Send(subArrayPerProc, elementsPerProc, MPI_INT, near, 1, MPI_COMM_WORLD);
            // printf("%d sent: ", step);
            // PrintArray(subArrayPerProc, elementsPerProc);
            break;
        }
        step *= 2;
    }

    if (procRank == 0)
    {
        printf("subArray: ");
        PrintArray(subArrayPerProc, elementsPerProc);
        // printf("Array: ");
        // PrintArray(parallelResultingArray, length);

        // AreResultsEqual(linearResultingArray, parallelResultingArray, length);

        free(parallelResultingArray);
    }

    // free(subArrayPerProc);
    // free(other);

    MPI_Finalize();
    return 0;
}
