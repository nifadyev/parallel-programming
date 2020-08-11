#include "shell_sort.h"

void GenerateArray(int *array, const int length)
{
    int i;

    srand(time(NULL));
    for (i = 0; i < length; i++)
    {
        array[i] = rand() % 100;
    }
}

void PrintArray(int *array, const int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void CopyArray(int *sourceArray, int *destinationArray, const int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        destinationArray[i] = sourceArray[i];
    }
}

void ShellSort(int *array, const int length)
{
    int gap; // Distance between compared array elements
    int i, j, k, temp;

    for (gap = length / 2; gap > 0; gap /= 2)
    {
        for (i = gap; i < length; i++)
        {
            j = i - gap;
            while (j >= 0)
            {
                k = j + gap;
                if (array[j] > array[k])
                {
                    temp = array[j];
                    array[j] = array[k];
                    array[k] = temp;
                    j -= gap;
                }
                else
                {
                    j--;
                }
            }
        }
    }
}

int *Merge(int *firstArray, const int firstArrayLength, int *secondArray, const int secondArrayLength)
{
    int i, k = 0, j = 0;
    int *resultingArray = (int *)malloc(sizeof(int) * (firstArrayLength + secondArrayLength));

    for (i = 0; i < firstArrayLength + secondArrayLength; i++)
    {
        if (j > firstArrayLength - 1)
        {
            resultingArray[i] = secondArray[k];
            k++;
        }
        else if (k > secondArrayLength - 1)
        {
            resultingArray[i] = firstArray[j];
            j++;
        }
        else if (firstArray[j] <= secondArray[k])
        {
            resultingArray[i] = firstArray[j];
            j++;
        }
        else
        {
            resultingArray[i] = secondArray[k];
            k++;
        }
    }

    return resultingArray;
}

int AreResultsEqual(int *linearResultingArray, int *parallelResultingArray, const int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        if (linearResultingArray[i] != parallelResultingArray[i])
        {
            return 1;
        }
    }

    return 0;
}

void ParallelShellSort(int *array, int length, const int procNum,
                       const int procRank, double &time, int *resultingArray)
{
    int i;
    int step; // Helps to sync send and receive operations between processes
    int receivedSubArrayLength;
    int elementsPerProc;
    int *subArrayPerProc;
    int *receivedSubArray;
    int *subArrayShifts;  // Arrays of start indexes for each subArrayPerProc
    int *subArrayLengths; // Array of lengths for each subArrayPerProc
    double startTime, endTime;
    MPI_Status status;

    subArrayShifts = (int *)malloc(sizeof(int) * procNum);
    subArrayLengths = (int *)malloc(sizeof(int) * procNum);

    if (procRank == 0)
    {
        elementsPerProc = length / procNum;

        for (i = 0; i < procNum; i++)
        {
            subArrayShifts[i] = i * elementsPerProc;
            subArrayLengths[i] = elementsPerProc;
        }
        // Last process handles subArray with remainder (if there is one)
        subArrayLengths[procNum - 1] = elementsPerProc + length % procNum;
    }

    startTime = MPI_Wtime();
    MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&elementsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(subArrayShifts, procNum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(subArrayLengths, procNum, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == procNum - 1)
    {
        elementsPerProc += length % procNum;
    }

    subArrayPerProc = (int *)malloc(sizeof(int) * elementsPerProc);

    MPI_Scatterv(array, subArrayLengths, subArrayShifts,
                 MPI_INT, subArrayPerProc, subArrayLengths[procRank],
                 MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    free(subArrayShifts);
    free(subArrayLengths);

    ShellSort(subArrayPerProc, elementsPerProc);

    for (step = 1; step < procNum; step *= 2)
    {
        // Even processes (included root) receive subArrays from not even processes
        // And store merged subArray
        if (procRank % (2 * step) == 0)
        {
            if (procRank + step < procNum)
            {
                MPI_Recv(&receivedSubArrayLength, 1, MPI_INT, procRank + step, 0, MPI_COMM_WORLD, &status);
                receivedSubArray = (int *)malloc(receivedSubArrayLength * sizeof(int));
                MPI_Recv(receivedSubArray, receivedSubArrayLength, MPI_INT, procRank + step, 1, MPI_COMM_WORLD, &status);
                subArrayPerProc = Merge(subArrayPerProc, elementsPerProc, receivedSubArray, receivedSubArrayLength);
                free(receivedSubArray);
                elementsPerProc += receivedSubArrayLength;
            }
        }
        else
        {
            MPI_Send(&elementsPerProc, 1, MPI_INT, ((procRank - step) < 0) ? 0 : (procRank - step), 0, MPI_COMM_WORLD);
            MPI_Send(subArrayPerProc, elementsPerProc, MPI_INT, ((procRank - step) < 0) ? 0 : (procRank - step), 1, MPI_COMM_WORLD);
        }
    }

    if (procRank == 0)
    {
        CopyArray(subArrayPerProc, resultingArray, length);
        endTime = MPI_Wtime();
        time = endTime - startTime;
    }

    free(subArrayPerProc);
}
