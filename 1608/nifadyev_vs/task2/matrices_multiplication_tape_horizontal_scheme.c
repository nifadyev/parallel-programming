#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void InitializeMatrix(int *matrix, const int matrixSize);
void MultiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix,
                      const int columns, const int rows);
void CheckResults(int *linearResultingMatrix, int *parallelResultingMatrix, const int matrixSize);
void PrintMatrix(int *matrix, const int size, const int columns);

// TODO: Not working if rows and columns are different
// FIXME: change somewhere ROWS to COLUMNS or on the contrary
// FIXME: Not working if ROWS or COLUMNS % procNum != 0
// TODO: change location of startTime and endTime for par alg

// TODO: breaks if ROWS < COLUMNS
// FIXME: works correctly if ROWS >= COLUMNS

int main(int argc, char **argv)
{
    int A_ROWS = 0, A_COLUMNS = 0, B_ROWS = 0, B_COLUMNS = 0;
    int A_SIZE = 0, B_SIZE = 0;
    int *matrixA = NULL, *matrixB = NULL, *linearResultingMatrix = NULL, *parallelResultingMatrix = NULL;
    int procNum = 1, procRank = 0;

    int elementsPerProc, rowsPerProc, remainingRows;
    int *rowsPerProcs = NULL, *shifts = NULL;
    int *bufferA = NULL, *resultBuffer = NULL;
    int i, j, k;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    //FIXME: Last strings of par resulting matrix are completely wrong

    if (procRank == 0)
    {
        if (argc == 5)
        {
            // A_COLUMNS & B_ROWS should be equal
            A_ROWS = atoi(argv[1]);
            A_COLUMNS = atoi(argv[2]);
            B_ROWS = atoi(argv[3]);
            B_COLUMNS = atoi(argv[4]);
        }
        else
        {
            A_ROWS = A_COLUMNS = 1000;
            B_ROWS = B_COLUMNS = 1000;
        }
        A_SIZE = A_ROWS * A_COLUMNS;
        B_SIZE = B_ROWS * B_COLUMNS;
        srand(time(NULL));
        matrixA = (int *)malloc(sizeof(int) * A_SIZE);
        InitializeMatrix(matrixA, A_SIZE);
        // printf("\nA matrix");
        // PrintMatrix(matrixA, A_SIZE, A_COLUMNS);
        matrixB = (int *)malloc(sizeof(int) * B_SIZE);
        InitializeMatrix(matrixB, B_SIZE);
        // printf("\nB matrix");
        // PrintMatrix(matrixB, B_SIZE, B_COLUMNS);
        // linearResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);
        //parallelResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    }


    startTime = MPI_Wtime();
    //FIXME: do i really need to do this???
    MPI_Bcast(&A_ROWS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&A_COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&A_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&B_ROWS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&B_COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&B_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (procRank != 0)
    {
        matrixB = (int *)malloc(sizeof(int) * B_SIZE);
    }
    MPI_Bcast(matrixB, B_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    rowsPerProcs = (int *)malloc(sizeof(int) * procNum);
    shifts = (int *)malloc(sizeof(int) * procNum);
    rowsPerProc = A_ROWS / procNum;
    //elementsPerProc = rowsPerProc * COLUMNS;
    for (i = 0; i < procNum; i++)
    {
        rowsPerProcs[i] = rowsPerProc * A_COLUMNS;
        shifts[i] = i * rowsPerProc * A_COLUMNS;
    }
    remainingRows = A_ROWS - (procNum - 1) * rowsPerProc;
    rowsPerProcs[procNum - 1] = remainingRows * A_COLUMNS;

    // Parallel Realization

    if (procRank == procNum - 1)
    {
        rowsPerProc = remainingRows;
    }
    bufferA = (int *)malloc(sizeof(int) * rowsPerProc * A_COLUMNS);

    //MPI_Scatter(matrixA, rowsPerProc, MPI_INT, bufferA, rowsPerProc, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Scatterv(matrixA, rowsPerProcs, shifts, MPI_INT, bufferA, rowsPerProcs[procRank], MPI_INT, 0, MPI_COMM_WORLD);

    resultBuffer = (int *)malloc(sizeof(int) * rowsPerProc * B_COLUMNS);
    //MultiplyMatrices(bufferA, matrixB, resultBuffer, /* ROWS */ COLUMNS, rowsPerProc /* rowsPerProc, ROWS */);

    for (i = 0; i < rowsPerProc; i++)
    {
        for (j = 0; j < B_COLUMNS; j++)
        {
            resultBuffer[i * B_COLUMNS + j] = 0;
            for (k = 0; k < A_COLUMNS; k++)
            {
                resultBuffer[i * B_COLUMNS + j] += bufferA[i * A_COLUMNS + k] * matrixB[k * B_COLUMNS + j];
            }
        }
    }
    if (procRank == 0)
    {
        parallelResultingMatrix = (int *)malloc(sizeof(int) * A_ROWS * B_COLUMNS);
        // for(i = 0; i < A_ROWS * B_COLUMNS; i++)
        // {
        //     parallelResultingMatrix[i] = 0;
        // }
    }
    rowsPerProc = A_ROWS / procNum;
    //elementsPerProc = rowsPerProc * /* ROWS */ COLUMNS;
    for (i = 0; i < procNum; i++) //maybe < procNum -1
    {
        rowsPerProcs[i] = rowsPerProc * B_COLUMNS;
        shifts[i] = i * rowsPerProc * B_COLUMNS;
    }
    //remainingRows = /* ROWS % procNum */ ROWS - (procNum - 1) * rowsPerProc;
    rowsPerProcs[procNum - 1] = remainingRows * B_COLUMNS;

    //MPI_Gather(resultBuffer, rowsPerProc * COLUMNS, MPI_INT, parallelResultingMatrix, rowsPerProc * COLUMNS, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(resultBuffer, rowsPerProcs[procRank], MPI_INT, parallelResultingMatrix, rowsPerProcs, shifts, MPI_INT, 0, MPI_COMM_WORLD);

    // Gathers together values from a group of processes

    if (procRank == 0)
    {
        endTime = MPI_Wtime();
        //CheckResults(linearResultingMatrix, parallelResultingMatrix, MATRIX_SIZE);
        // Linear Realization
        // printf("\nPARALLEL RESULT");
        // for (i = 0; i < MATRIX_SIZE; i++)
        // {
        //     if (i % COLUMNS == 0)
        //     {
        //         printf("\n");
        //     }
        //     printf("%d ", parallelResultingMatrix[i]);
        // }
        // printf("\n\n");
        // printf("\nPARALLEL RESULT");
        // PrintMatrix(parallelResultingMatrix, A_ROWS * B_COLUMNS, B_COLUMNS);
        parallelTime = endTime - startTime;
        printf("Parallel time = %.3f\n", parallelTime);
        // if (procRank == 0)
        // {
        linearResultingMatrix = (int *)malloc(sizeof(int) * A_ROWS * B_COLUMNS);
        startTime = MPI_Wtime();
        //MultiplyMatrices(matrixA, matrixB, linearResultingMatrix, COLUMNS, ROWS);
        for (i = 0; i < A_ROWS; i++)
        {
            for (j = 0; j < B_COLUMNS; j++)
            {
                linearResultingMatrix[i * B_COLUMNS + j] = 0;
                for (k = 0; k < A_COLUMNS; k++)
                {
                    linearResultingMatrix[i * B_COLUMNS + j] += matrixA[i * A_COLUMNS + k] * matrixB[k * B_COLUMNS + j];
                }
            }
        }
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;
        // printf("LINEAR RESULT");
        // PrintMatrix(linearResultingMatrix, A_ROWS * B_COLUMNS, B_COLUMNS);

        // printf("LINEAR RESULT");
        // for (i = 0; i < MATRIX_SIZE; i++)
        // {
        //     if (i % COLUMNS == 0)
        //     {
        //         printf("\n");
        //     }
        //     printf("%d ", linearResultingMatrix[i]);
        // }
        // printf("\n\n");
        printf("Linear time = %.3f\n\n", linearTime);
        //}
        for (i = 0; i < A_ROWS * B_COLUMNS; i++)
        {
            if (linearResultingMatrix[i] != parallelResultingMatrix[i])
            {
                printf("Error! Linear and parallel results are not equal\n");
            }
        }
    }

    if (procRank == 0)
    {
        free(matrixA);
        free(linearResultingMatrix);
        free(parallelResultingMatrix);
    }
    MPI_Finalize();
    free(matrixB);

    free(bufferA);
    free(resultBuffer);
    free(rowsPerProcs);
    free(shifts);
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
                      const int columns, const int rows)
{
    int i, j, k;

    for (i = 0; i < /* columns */ rows; i++)
    {
        for (j = 0; j < /* rows */ columns; j++)
        {
            resultMatrix[i * /* rows */ columns + j] = 0;
            for (k = 0; k < /* rows */ columns; k++)
            {
                resultMatrix[i * /* rows */ columns + j] += matrixA[i * /* rows */ columns + k] * matrixB[k * /* rows */ columns + j];
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
