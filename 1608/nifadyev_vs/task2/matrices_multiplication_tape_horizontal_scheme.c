#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void InitializeMatrix(int *matrix, const int matrixSize);
void MultiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix,
                      const int columns, const int rows);
void CheckResults(int *linearResultingMatrix, int *parallelResultingMatrix, const int matrixSize);

// TODO: Not working if rows and columns are different
// FIXME: change somewhere ROWS to COLUMNS or on the contrary
// FIXME: Not working if ROWS or COLUMNS % procNum != 0
// TODO: change location of startTime and endTime for par alg

// TODO: breaks if ROWS < COLUMNS
// FIXME: works correctly if ROWS >= COLUMNS

int main(int argc, char **argv)
{
    int ROWS, COLUMNS, MATRIX_SIZE;
    int *matrixA, *matrixB, *linearResultingMatrix, *parallelResultingMatrix;
    int procNum, procRank = 0;
    int elementsPerProc, rowsPerProc, remainingRows;
    int *rowsPerProcs, *shifts;
    int *bufferA, *resultBuffer;
    int i, j, k;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0)
    {
        if (argc == 1)
        {
            COLUMNS = ROWS = 10;
        }
        else if (argc == 3)
        {
            // COLUMNS = atoi(argv[1]);
            // ROWS = atoi(argv[2]);
            ROWS = atoi(argv[1]);
            COLUMNS = atoi(argv[2]);
        }
        MATRIX_SIZE = COLUMNS * ROWS;
    }

    MPI_Bcast(&COLUMNS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ROWS, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&MATRIX_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);

    matrixA = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    matrixB = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    linearResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);
    parallelResultingMatrix = (int *)malloc(sizeof(int) * MATRIX_SIZE);

    srand(time(NULL));
    InitializeMatrix(matrixA, MATRIX_SIZE);
    InitializeMatrix(matrixB, MATRIX_SIZE);

    rowsPerProcs = (int *)malloc(sizeof(int) * procNum);
    shifts = (int *)malloc(sizeof(int) * procNum);
    rowsPerProc = ROWS / procNum;
    elementsPerProc = rowsPerProc * COLUMNS;
    for (i = 0; i < procNum; i++) //maybe < procNum -1
    {
        rowsPerProcs[i] = elementsPerProc;
        shifts[i] = i * elementsPerProc;
    }
    remainingRows = /* ROWS % procNum */ ROWS - (procNum - 1) * rowsPerProc;
    rowsPerProcs[procNum - 1] = remainingRows * COLUMNS;

    // bufferA = (int *)malloc(sizeof(int) * elementsPerProc);
    // resultBuffer = (int *)malloc(sizeof(int) * elementsPerProc);

    // Linear Realization
    if (procRank == 0)
    {
        startTime = MPI_Wtime();
        MultiplyMatrices(matrixA, matrixB, linearResultingMatrix, COLUMNS, ROWS);
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;

        printf("Linear time = %.3f\n\n", linearTime);
    }

    // Parallel Realization
    startTime = MPI_Wtime();
    MPI_Bcast(matrixB, MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == procNum - 1)
    {
        rowsPerProc = remainingRows;
    }
    bufferA = (int *)malloc(sizeof(int) * rowsPerProc * /* ROWS */COLUMNS);
    resultBuffer = (int *)malloc(sizeof(int) * rowsPerProc * /* ROWS */COLUMNS);
    MPI_Scatterv(matrixA, rowsPerProcs, shifts, MPI_INT, bufferA, rowsPerProcs[procRank], MPI_INT, 0, MPI_COMM_WORLD);
    MultiplyMatrices(bufferA, matrixB, resultBuffer, /* ROWS */COLUMNS, rowsPerProc/* rowsPerProc, ROWS */);

    rowsPerProc = ROWS / procNum;
    elementsPerProc = rowsPerProc * /* ROWS */COLUMNS;
    for (i = 0; i < procNum; i++) //maybe < procNum -1
    {
        rowsPerProcs[i] = elementsPerProc;
        shifts[i] = i * elementsPerProc;
    }
    //remainingRows = /* ROWS % procNum */ ROWS - (procNum - 1) * rowsPerProc;
    rowsPerProcs[procNum - 1] = remainingRows * /* ROWS */COLUMNS;

    MPI_Gatherv(resultBuffer, rowsPerProcs[procRank], MPI_INT, parallelResultingMatrix, rowsPerProcs, shifts, MPI_INT, 0, MPI_COMM_WORLD);

    // if (procRank == procNum - 1 && remainingRows != 0)
    // {
    //     free(bufferA);
    //     free(resultBuffer);
    //     elementsPerProc = (rowsPerProc + remainingRows) * COLUMNS;
    //     bufferA = (int *)malloc(sizeof(int) * elementsPerProc);
    //     resultBuffer = (int *)malloc(sizeof(int) * elementsPerProc);

    // MPI_Scatter(matrixA, elementsPerProc, MPI_INT, bufferA, elementsPerProc, MPI_INT, 0, MPI_COMM_WORLD);

    // MultiplyMatrices(bufferA, matrixB, resultBuffer, rowsPerProc + remainingRows, ROWS);
    // MPI_Gather(resultBuffer, elementsPerProc, MPI_INT, parallelResultingMatrix, elementsPerProc, MPI_INT, 0, MPI_COMM_WORLD);

    // }
    // else
    // {

    //     MPI_Scatter(matrixA, elementsPerProc, MPI_INT, bufferA, elementsPerProc, MPI_INT, 0, MPI_COMM_WORLD);

    //     MultiplyMatrices(bufferA, matrixB, resultBuffer, rowsPerProc, ROWS);
    //     MPI_Gather(resultBuffer, /* sizeof */ elementsPerProc, MPI_INT, parallelResultingMatrix, elementsPerProc, MPI_INT, 0, MPI_COMM_WORLD);
    // }

    // Gathers together values from a group of processes

    if (procRank == 0)
    {
        endTime = MPI_Wtime();
        //CheckResults(linearResultingMatrix, parallelResultingMatrix, MATRIX_SIZE);

        for (i = 0; i < MATRIX_SIZE; i++)
        {
            if (linearResultingMatrix[i] != parallelResultingMatrix[i])
            {
                printf("Error! Linear and parallel results are not equal\n");
            }
        }
        parallelTime = endTime - startTime;
        printf("Parallel time = %.3f\n", parallelTime);
    }

    MPI_Finalize();
    free(matrixA);
    free(matrixB);
    free(linearResultingMatrix);
    free(parallelResultingMatrix);
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

    for (i = 0; i < /* columns */rows; i++)
    {
        for (j = 0; j < /* rows */columns; j++)
        {
            for (k = 0; k < /* rows */columns; k++)
            {
                resultMatrix[i * /* rows */columns + j] += matrixA[i * /* rows */columns + k] * matrixB[k * /* rows */columns + j];
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
