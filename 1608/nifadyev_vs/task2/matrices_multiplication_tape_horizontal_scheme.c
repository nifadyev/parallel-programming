#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void InitializeMatrix(int *matrix, const int matrixSize)
{
    int i;

    for (i = 0; i < matrixSize; i++)
    {
        matrix[i] = rand() % 150 - 50;
    }
}

void MultiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix,
                      const int columns, const int rows)
{
    int i, j, k;

    for (i = 0; i < columns; i++)
    {
        for (j = 0; j < rows; j++)
        {
            resultMatrix[i * rows + j] = 0;
            for (k = 0; k < rows; k++)
            {
                resultMatrix[i * rows + j] += matrixA[i * rows + k] * matrixB[k * rows + j];
            }
        }
    }
}

int main(int argc, char **argv)
{
    int ROWS, COLUMNS, MATRIX_SIZE; //TODO: use MATRIX_SIZE instead of COLUMNS * ROWS
    int *matrixA, *matrixB, *linearResultingMatrix, *parallelResultingMatrix;
    int procNum, procRank = 0;
    int partSize, procPartSize, partSizeB, procPartSizeB; //TODO: RENAME THIS!!!!
    int *bufferA, *bufferB, *resultBuffer;
    int i, j, k, nproc, index;
    int nextProc, prevProc;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    MPI_Status status;

    // May broke the program
    if (procRank == 0)
    {
        if (argc == 1)
        {
            COLUMNS = ROWS = 10;
        }
        else if (argc == 3)
        {
            COLUMNS = atoi(argv[1]);
            ROWS = atoi(argv[2]);
        }
        MATRIX_SIZE = COLUMNS * ROWS;
    }

    matrixA = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    matrixB = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    linearResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    parallelResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);

    srand(time(NULL));
    InitializeMatrix(matrixA, MATRIX_SIZE);

    InitializeMatrix(matrixB, MATRIX_SIZE);


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0)
    {
        printf("FIRST");
        for (i = 0; i < MATRIX_SIZE; i++)
        {
            if (i % COLUMNS == 0)
            {
                printf("\n");
            }
            printf("%d ", matrixA[i]);
        }
        printf("\n\n");

        printf("SECOND");
        for (i = 0; i < MATRIX_SIZE; i++)
        {
            if (i % COLUMNS == 0)
            {
                printf("\n");
            }
            printf("%d ", matrixB[i]);
        }
        printf("\n\n");
    }

    procPartSize = ROWS / procNum;
    partSize = (ROWS / procNum) * COLUMNS;
    procPartSizeB = COLUMNS / procNum;
    partSizeB = (COLUMNS / procNum) * COLUMNS;
    bufferA = (int *)malloc(sizeof(int) * partSize);
    bufferB = (int *)malloc(sizeof(int) * partSizeB);
    resultBuffer = (int *)malloc(sizeof(int) * partSize);

    // Linear Realization
    if (procRank == 0)
    {
        startTime = MPI_Wtime();
        MultiplyMatrices(matrixA, matrixB, linearResultingMatrix, COLUMNS, ROWS);
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;

        printf("LINEAR RESULT");
        for (i = 0; i < MATRIX_SIZE; i++)
        {
            if (i % COLUMNS == 0)
            {
                printf("\n");
            }
            printf("%d ", linearResultingMatrix[i]);
        }
        printf("\n\n");
        printf("Linear time = %.3f\n\n", linearTime);
    }

    // *********************************************************************
    // Sends data from one process to all other processes in a communicator
    // MPI_Bcast(&ROWS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&MATRIX_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(matrixB, MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(matrixA, partSize, MPI_INT, bufferA, partSize, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Scatter(matrixB, partSizeB, MPI_INT, bufferB, partSizeB, MPI_INT, 0, MPI_COMM_WORLD);

    for (i = 0; i < procPartSize; i++)
    {
        for (j = 0; j < COLUMNS; j++)
        {
            //resultMatrix[i * rows + j] = 0;
            for (k = 0; k < COLUMNS; k++)
            {
                // resultMatrix[i * rows + j] += matrixA[i * rows + k] * matrixB[k * rows + j];
                //resultBuffer[i * COLUMNS + j + procPartSize * procRank] += bufferA[i * COLUMNS + k] * bufferB[j * COLUMNS + k];
                // bufC[i*dim+j+ProcPartSize*ProcRank] = temp;
                resultBuffer[i * ROWS + j/*  + procPartSize * procRank */] += bufferA[i * ROWS + k] * matrixB[k * ROWS + j];
                printf("FIRST Proc %d Adding to result buffer %d %d\n",procRank, i * ROWS + j + procPartSize * procRank, bufferA[i * ROWS + k] * matrixB[k * ROWS + j]);
            }
        }
    }

    nextProc = procRank + 1;
    if (procRank == procNum - 1)
    {
        nextProc = 0;
    }
    prevProc = procRank - 1;
    if (procRank == 0)
    {
        prevProc = procNum - 1;
    }

    // for (nproc = 1; nproc < procNum; nproc++)
    // {
    //     // Sends and receives using a single buffer
    //     //MPI_Sendrecv_replace(bufferB, partSizeB, MPI_INT, nextProc, 0, prevProc, 0, MPI_COMM_WORLD, &status);
    //     for (i = 0; i < procPartSize; i++)
    //     {
    //         for (j = 0; j < partSize; j++)
    //         {
    //             if (procRank - nproc >= 0)
    //             {
    //                 index = procRank - nproc;
    //             }
    //             else
    //             {
    //                 index = (procNum - nproc + procRank);
    //             }
    //             for (k = 0; k < COLUMNS; k++)
    //             {
    //                 // bufC[i*dim+j+ind*ProcPartSize] = temp;
    //                 resultBuffer[i * COLUMNS + j + index * procPartSize] +=
    //                     bufferA[i * ROWS + k] * matrixB[k * ROWS + j];
    //                 printf("SECOND Adding to result buffer %d (not in root) %d\n", i * COLUMNS * ROWS + j + index * procPartSize, bufferA[i * ROWS + k] * matrixB[k * ROWS + j]);
    //             }
    //         }
    //     }
    // }

    // Gathers together values from a group of processes
    MPI_Gather(resultBuffer, procPartSize * COLUMNS, MPI_INT, parallelResultingMatrix, procPartSize * COLUMNS, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == 0)
    {
        for (i = 0; i < MATRIX_SIZE; i++)
        {
            if (linearResultingMatrix[i] != parallelResultingMatrix[i])
            {
                printf("Error! Linear and parallel results are not equal\n");
            }
        }

        printf("\nPARALLEL RESULT");
        for (i = 0; i < MATRIX_SIZE; i++)
        {
            if (i % COLUMNS == 0)
            {
                printf("\n");
            }
            printf("%d ", parallelResultingMatrix[i]);
        }
        printf("\n\n");
    }

    MPI_Finalize();
    free(matrixA);
    free(matrixB);
    free(linearResultingMatrix);
    free(parallelResultingMatrix);
    return 0;
}