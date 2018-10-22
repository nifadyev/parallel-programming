#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Task # 27: Count the number of mismatched characters in two strings

void InitializeStrings(char *string1, char *string2, const int size);
int CountMismatchesInTwoStrings(char *string1, char *string2, const int endIndex);

int main(int argc, char **argv)
{
    const int FIRST_STRING = 1, SECOND_STRING = 2;
    int MAX_SIZE;
    int procNum, procRank = 0;
    int partSize, partRemainder;
    int i, partRemainderFlag = 0;
    int linearResult = 0, parallelResult = 0, mismatchCount = 0;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    char *str1, *str2;
    char *temp1, *temp2;
    MPI_Status status;

    if (argc == 1)
    {
        MAX_SIZE = 10000000;
    }
    else
    {
        MAX_SIZE = atoi(argv[1]);
    }

    str1 = (char *)malloc(sizeof(char) * (MAX_SIZE + 1));
    str2 = (char *)malloc(sizeof(char) * (MAX_SIZE + 1));
    InitializeStrings(str1, str2, MAX_SIZE);

    MPI_Init(&argc, &argv);
    // Get the number of processors
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    // Get rank of current process
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    partSize = MAX_SIZE / procNum;
    partRemainder = MAX_SIZE % procNum;

    /************Linear realization************/
    if (procRank == 0)
    {
        startTime = MPI_Wtime();
        linearResult = CountMismatchesInTwoStrings(str1, str2, MAX_SIZE);
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;

        printf("Linear time = %.3f\n", endTime - startTime);
        printf("Linear result = %d\n", linearResult);
    }

    /************Parallel realization********/
    if (procRank == 0)
    {
        startTime = MPI_Wtime();

        partRemainderFlag = 0;
        for (i = 1; i < procNum; i++)
        {
            if (procNum - 1 == i)
            {
                // Add remainder to last processor's partSize
                partRemainderFlag = 1;
            }

            MPI_Send(&str1[partSize * i], partSize + partRemainder * partRemainderFlag, MPI_CHAR, i, FIRST_STRING, MPI_COMM_WORLD);
            MPI_Send(&str2[partSize * i], partSize + partRemainder * partRemainderFlag, MPI_CHAR, i, SECOND_STRING, MPI_COMM_WORLD);
        }

        mismatchCount = CountMismatchesInTwoStrings(str1, str2, partSize);
    }
    else
    {
        partRemainderFlag = 0;
        if (procRank == procNum - 1)
        {
            partRemainderFlag = 1;
        }

        temp1 = (char *)malloc(sizeof(char) * (partSize + partRemainder * partRemainderFlag));
        temp2 = (char *)malloc(sizeof(char) * (partSize + partRemainder * partRemainderFlag));
        MPI_Recv(temp1, partSize + partRemainder * partRemainderFlag, MPI_CHAR, 0, FIRST_STRING, MPI_COMM_WORLD, &status);
        MPI_Recv(temp2, partSize + partRemainder * partRemainderFlag, MPI_CHAR, 0, SECOND_STRING, MPI_COMM_WORLD, &status);
        mismatchCount = CountMismatchesInTwoStrings(temp1, temp2, partSize + partRemainder * partRemainderFlag);
    }

    // Collect the result into main processor
    MPI_Reduce(&mismatchCount, &parallelResult, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (procRank == 0 && procNum > 1)
    {
        if (linearResult != parallelResult)
        {
            printf("Error! Results are not equal\n");
        }

        endTime = MPI_Wtime();
        parallelTime = endTime - startTime;
        printf("MPI time = %.3f\n", endTime - startTime);
        printf("MPI result = %d\n", parallelResult);
        printf("Performance difference: %.1f\%\n", ((linearTime - parallelTime) / ((linearTime + parallelTime) / 2)) * 100);
    }

    MPI_Finalize();
    free(str1);
    free(str2);

    return 0;
}

void InitializeStrings(char *string1, char *string2, const int size)
{
    int i;
    srand(time(NULL));

    for (i = 0; i < size; i++)
    {
        string1[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
        string2[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
    }
    string1[size] = '\0';
    string2[size] = '\0';
}

int CountMismatchesInTwoStrings(char *string1, char *string2, const int endIndex)
{
    int result = 0;
    int i;

    for (i = 0; i < endIndex; i++)
    {
        if (string1[i] != string2[i])
        {
            result++;
        }
    }

    return result;
}

// CORRECTLY CALC FIRST ELEMENTS oF STRINGS
// #include <mpich/mpi.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>

// void InitializeMatrix(int *matrix, const int matrixSize)
// {
//     int i;

//     for (i = 0; i < matrixSize; i++)
//     {
//         matrix[i] = rand() % 150 - 50;
//     }
// }

// void MultiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix,
//                       const int columns, const int rows)
// {
//     int i, j, k;

//     for (i = 0; i < columns; i++)
//     {
//         for (j = 0; j < rows; j++)
//         {
//             resultMatrix[i * rows + j] = 0;
//             for (k = 0; k < rows; k++)
//             {
//                 resultMatrix[i * rows + j] += matrixA[i * rows + k] * matrixB[k * rows + j];
//             }
//         }
//     }
// }

// int main(int argc, char **argv)
// {
//     int ROWS, COLUMNS, MATRIX_SIZE; //TODO: use MATRIX_SIZE instead of COLUMNS * ROWS
//     int *matrixA, *matrixB, *linearResultingMatrix, *parallelResultingMatrix;
//     int procNum, procRank = 0;
//     int partSize, procPartSize, partSizeB, procPartSizeB; //TODO: RENAME THIS!!!!
//     int *bufferA, *bufferB, *resultBuffer;
//     int i, j, k, nproc, index;
//     int nextProc, prevProc;
//     double startTime = 0, endTime = 0;
//     double linearTime = 0, parallelTime = 0;
//     MPI_Status status;

//     // May broke the program
//     if (procRank == 0)
//     {
//         if (argc == 1)
//         {
//             COLUMNS = ROWS = 10;
//         }
//         else if (argc == 3)
//         {
//             COLUMNS = atoi(argv[1]);
//             ROWS = atoi(argv[2]);
//         }
//         MATRIX_SIZE = COLUMNS * ROWS;
//     }

//     matrixA = (int *)malloc(sizeof(int) * MATRIX_SIZE);
//     matrixB = (int *)malloc(sizeof(int) * MATRIX_SIZE);
//     linearResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);
//     parallelResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);

//     srand(time(NULL));
//     InitializeMatrix(matrixA, MATRIX_SIZE);

//     InitializeMatrix(matrixB, MATRIX_SIZE);


//     MPI_Init(&argc, &argv);
//     MPI_Comm_size(MPI_COMM_WORLD, &procNum);
//     MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

//     if (procRank == 0)
//     {
//         printf("FIRST");
//         for (i = 0; i < MATRIX_SIZE; i++)
//         {
//             if (i % COLUMNS == 0)
//             {
//                 printf("\n");
//             }
//             printf("%d ", matrixA[i]);
//         }
//         printf("\n\n");

//         printf("SECOND");
//         for (i = 0; i < MATRIX_SIZE; i++)
//         {
//             if (i % COLUMNS == 0)
//             {
//                 printf("\n");
//             }
//             printf("%d ", matrixB[i]);
//         }
//         printf("\n\n");
//     }

//     procPartSize = ROWS / procNum;
//     partSize = (ROWS / procNum) * COLUMNS;
//     procPartSizeB = COLUMNS / procNum;
//     partSizeB = (COLUMNS / procNum) * COLUMNS;
//     bufferA = (int *)malloc(sizeof(int) * partSize);
//     bufferB = (int *)malloc(sizeof(int) * partSizeB);
//     resultBuffer = (int *)malloc(sizeof(int) * partSize);

//     // Linear Realization
//     if (procRank == 0)
//     {
//         startTime = MPI_Wtime();
//         MultiplyMatrices(matrixA, matrixB, linearResultingMatrix, COLUMNS, ROWS);
//         endTime = MPI_Wtime();
//         linearTime = endTime - startTime;

//         printf("LINEAR RESULT");
//         for (i = 0; i < MATRIX_SIZE; i++)
//         {
//             if (i % COLUMNS == 0)
//             {
//                 printf("\n");
//             }
//             printf("%d ", linearResultingMatrix[i]);
//         }
//         printf("\n\n");
//         printf("Linear time = %.3f\n\n", linearTime);
//     }

//     // *********************************************************************
//     // Sends data from one process to all other processes in a communicator
//     // MPI_Bcast(&ROWS, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     // MPI_Bcast(&COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     // MPI_Bcast(&MATRIX_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Bcast(matrixB, MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Scatter(matrixA, partSize, MPI_INT, bufferA, partSize, MPI_INT, 0, MPI_COMM_WORLD);
//     //MPI_Scatter(matrixB, partSizeB, MPI_INT, bufferB, partSizeB, MPI_INT, 0, MPI_COMM_WORLD);

//     for (i = 0; i < procPartSize; i++)
//     {
//         for (j = 0; j < procPartSizeB; j++)
//         {
//             //resultMatrix[i * rows + j] = 0;
//             for (k = 0; k < COLUMNS; k++)
//             {
//                 // resultMatrix[i * rows + j] += matrixA[i * rows + k] * matrixB[k * rows + j];
//                 //resultBuffer[i * COLUMNS + j + procPartSize * procRank] += bufferA[i * COLUMNS + k] * bufferB[j * COLUMNS + k];
//                 // bufC[i*dim+j+ProcPartSize*ProcRank] = temp;
//                 resultBuffer[i * ROWS + j + procPartSize * procRank] += bufferA[i * ROWS + k] * matrixB[k * ROWS + j];
//                 printf("FIRST Adding to result buffer %d %d\n", i * ROWS + j + procPartSize * procRank, i * ROWS + j + procPartSize * procRank);
//             }
//         }
//     }

//     nextProc = procRank + 1;
//     if (procRank == procNum - 1)
//     {
//         nextProc = 0;
//     }
//     prevProc = procRank - 1;
//     if (procRank == 0)
//     {
//         prevProc = procNum - 1;
//     }

//     for (nproc = 1; nproc < procNum; nproc++)
//     {
//         // Sends and receives using a single buffer
//         MPI_Sendrecv_replace(bufferB, partSizeB, MPI_INT, nextProc, 0, prevProc, 0, MPI_COMM_WORLD, &status);
//         for (i = 0; i < procPartSize; i++)
//         {
//             for (j = 0; j < partSizeB; j++)
//             {
//                 if (procRank - nproc >= 0)
//                 {
//                     index = procRank - nproc;
//                 }
//                 else
//                 {
//                     index = (procNum - nproc + procRank);
//                 }
//                 for (k = 0; k < COLUMNS; k++)
//                 {
//                     // bufC[i*dim+j+ind*ProcPartSize] = temp;
//                     resultBuffer[i * COLUMNS + j + index * procPartSize] +=
//                         bufferA[i * ROWS + k] * matrixB[k * ROWS + j];
//                     printf("SECOND Adding to result buffer %d (not in root) %d\n", i * COLUMNS * ROWS + j + index * procPartSize, bufferA[i * ROWS + k] * bufferB[k * ROWS + j]);
//                 }
//             }
//         }
//     }

//     // Gathers together values from a group of processes
//     MPI_Gather(resultBuffer, procPartSize * COLUMNS, MPI_INT, parallelResultingMatrix, procPartSize * COLUMNS, MPI_INT, 0, MPI_COMM_WORLD);

//     if (procRank == 0)
//     {
//         for (i = 0; i < MATRIX_SIZE; i++)
//         {
//             if (linearResultingMatrix[i] != parallelResultingMatrix[i])
//             {
//                 printf("Error! Linear and parallel results are not equal\n");
//             }
//         }

//         printf("\nPARALLEL RESULT");
//         for (i = 0; i < MATRIX_SIZE; i++)
//         {
//             if (i % COLUMNS == 0)
//             {
//                 printf("\n");
//             }
//             printf("%d ", parallelResultingMatrix[i]);
//         }
//         printf("\n\n");
//     }

//     MPI_Finalize();
//     free(matrixA);
//     free(matrixB);
//     free(linearResultingMatrix);
//     free(parallelResultingMatrix);
//     return 0;
// }