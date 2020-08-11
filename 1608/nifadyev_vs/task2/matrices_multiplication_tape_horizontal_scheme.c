#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Task #17: Matrix multiplication using tape horizontal scheme (splits only first matrix into strings)

void InitializeMatrix(int *matrix, const int matrixSize);
void MultiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix,
                      const int l, const int m, const int n);
void CheckResults(int *linearResultingMatrix, int *parallelResultingMatrix, const int matrixSize);
void PrintMatrix(int *matrix, const int size, const int columns);

int main(int argc, char **argv)
{
    int i, j, k;
    int A_ROWS = 0, A_COLUMNS = 0, B_ROWS = 0, B_COLUMNS = 0;
    int A_SIZE = 0, B_SIZE = 0;
    int procNum = 1, procRank = 0;
    int rowsPerProc = 0, remainingRows = 0;
    int *matrixA = NULL, *matrixB = NULL;
    int *linearResultingMatrix = NULL, *parallelResultingMatrix = NULL;
    int *elementsPerProcScatter = NULL, *shiftsScatter = NULL;
    int *elementsPerProcGather = NULL, *shiftsGather = NULL;
    int *bufferA = NULL, *resultBuffer = NULL;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0)
    {
        if (argc == 5)
        {
            A_ROWS = atoi(argv[1]);
            A_COLUMNS = atoi(argv[2]);
            B_ROWS = atoi(argv[3]);
            B_COLUMNS = atoi(argv[4]);

            // Due to math algorithm of matrices multiplication
            if (A_COLUMNS != B_ROWS)
            {
                printf("Error! Number of columns in first matrix and number ");
                printf("of rows in second must be equal\n");
                MPI_Finalize();
                return 1;
            }
        }
        else
        {
            A_ROWS = A_COLUMNS = 1000;
            B_ROWS = B_COLUMNS = 1000;
        }
        A_SIZE = A_ROWS * A_COLUMNS;
        B_SIZE = B_ROWS * B_COLUMNS;

        rowsPerProc = A_ROWS / procNum;
        remainingRows = A_ROWS - (procNum - 1) * rowsPerProc;

        matrixA = (int *)malloc(sizeof(int) * A_SIZE);
    }

    //***********************Parallel Realization***********************
    startTime = MPI_Wtime();
    MPI_Bcast(&A_ROWS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&A_COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&B_COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&A_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&B_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&remainingRows, 1, MPI_INT, 0, MPI_COMM_WORLD);

    matrixB = (int *)malloc(sizeof(int) * B_SIZE);
    elementsPerProcScatter = (int *)malloc(sizeof(int) * procNum);
    elementsPerProcGather = (int *)malloc(sizeof(int) * procNum);
    shiftsScatter = (int *)malloc(sizeof(int) * procNum);
    shiftsGather = (int *)malloc(sizeof(int) * procNum);

    if (procRank == 0)
    {
        srand(time(NULL));
        InitializeMatrix(matrixA, A_SIZE);
        InitializeMatrix(matrixB, B_SIZE);

        for (i = 0; i < procNum; i++)
        {
            elementsPerProcScatter[i] = rowsPerProc * A_COLUMNS;
            elementsPerProcGather[i] = rowsPerProc * B_COLUMNS;
            shiftsScatter[i] = i * rowsPerProc * A_COLUMNS;
            shiftsGather[i] = i * rowsPerProc * B_COLUMNS;
        }
        elementsPerProcScatter[procNum - 1] = remainingRows * A_COLUMNS;
        elementsPerProcGather[procNum - 1] = remainingRows * B_COLUMNS;

        parallelResultingMatrix = (int *)malloc(sizeof(int) * A_ROWS * B_COLUMNS);
    }

    MPI_Bcast(matrixB, B_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(elementsPerProcScatter, procNum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(elementsPerProcGather, procNum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(shiftsScatter, procNum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(shiftsGather, procNum, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == procNum - 1)
    {
        rowsPerProc = remainingRows;
    }

    bufferA = (int *)malloc(sizeof(int) * rowsPerProc * A_COLUMNS);
    resultBuffer = (int *)malloc(sizeof(int) * rowsPerProc * B_COLUMNS);

    // Scatters a buffer in parts to all processes in a communicator
    MPI_Scatterv(matrixA, elementsPerProcScatter, shiftsScatter, MPI_INT, bufferA,
                 elementsPerProcScatter[procRank], MPI_INT, 0, MPI_COMM_WORLD);

    // Main calculations
    MultiplyMatrices(bufferA, matrixB, resultBuffer, rowsPerProc, A_COLUMNS, B_COLUMNS);

    // Gathers together values from a group of processes
    MPI_Gatherv(resultBuffer, elementsPerProcGather[procRank], MPI_INT,
                parallelResultingMatrix, elementsPerProcGather, shiftsGather, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == 0)
    {
        endTime = MPI_Wtime();
        parallelTime = endTime - startTime;
        printf("Parallel time = %.3f\n", parallelTime);
        //****************************************************************

        //***********************Linear Realization***********************
        linearResultingMatrix = (int *)malloc(sizeof(int) * A_ROWS * B_COLUMNS);

        startTime = MPI_Wtime();
        MultiplyMatrices(matrixA, matrixB, linearResultingMatrix, A_ROWS, A_COLUMNS, B_COLUMNS);
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;
        printf("Linear time = %.3f\n\n", linearTime);
        //****************************************************************

        CheckResults(linearResultingMatrix, parallelResultingMatrix, A_ROWS * B_COLUMNS);
        printf("Performance difference: %.1f%\n", (linearTime / parallelTime) * 100);

        free(matrixA);
        free(linearResultingMatrix);
        free(parallelResultingMatrix);
    }

    MPI_Finalize();

    free(matrixB);
    free(bufferA);
    free(resultBuffer);
    free(elementsPerProcScatter);
    free(shiftsScatter);
    free(elementsPerProcGather);
    free(shiftsGather);

    return 0;
}



void InitializeMatrix(int *matrix, const int matrixSize)
{
    int i;

    for (i = 0; i < matrixSize; i++)
    {
        matrix[i] = rand() % 150 - 50;
    }
}

void MultiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix,
                      const int l, const int m, const int n)
{
    int i, j, k;

    for (i = 0; i < l; i++)
    {
        for (j = 0; j < n; j++)
        {
            resultMatrix[i * n + j] = 0;
            for (k = 0; k < m; k++)
            {
                resultMatrix[i * n + j] += matrixA[i * m + k] * matrixB[k * n + j];
            }
        }
    }
}

void CheckResults(int *linearResultingMatrix, int *parallelResultingMatrix, const int matrixSize)
{
    int i;

    for (i = 0; i < matrixSize; i++)
    {
        if (linearResultingMatrix[i] != parallelResultingMatrix[i])
        {
            printf("Error! Linear and parallel results are not equal\n");
        }
    }
}

void PrintMatrix(int *matrix, const int size, const int columns)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (i % columns == 0)
        {
            printf("\n");
        }
        printf("%d ", matrix[i]);
    }
    printf("\n\n");
}
