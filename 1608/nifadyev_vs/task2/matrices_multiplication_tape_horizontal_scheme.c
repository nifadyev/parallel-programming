#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void InitializeMatrix(int *matrix, const int columns, const int rows)
{
    int i;
    srand(time(NULL));

    for (i = 0; i < columns * rows; i++)
    {
        matrix[i] = rand() % 100 - 50;
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
    int ROWS, COLUMNS;
    int *matrixA, *matrixB, *resultMatrix;
    int procNum, procRank;
    int i;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    MPI_Status status;

    if (argc == 1)
    {
        COLUMNS = ROWS = 10;
    }
    else if (argc == 3)
    {
        COLUMNS = atoi(argv[1]);
        ROWS = atoi(argv[2]);
    }

    matrixA = (int *)malloc(sizeof(int) * COLUMNS * ROWS);
    matrixB = (int *)malloc(sizeof(int) * COLUMNS * ROWS);
    resultMatrix = (int *)malloc(sizeof(int) * COLUMNS * ROWS);

    InitializeMatrix(matrixA, COLUMNS, ROWS);
    InitializeMatrix(matrixB, COLUMNS, ROWS);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    // Linear Realization
    if (procRank == 0)
    {
        startTime = MPI_Wtime();
        MultiplyMatrices(matrixA, matrixB, resultMatrix, COLUMNS, ROWS);
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;

        // for (i = 0; i < COLUMNS * ROWS; i++)
        // {
        //     if (i % COLUMNS == 0)
        //     {
        //         printf("\n");
        //     }
        //     printf("%d ", resultMatrix[i]);
        // }
        // printf("\n");
        printf("Linear time = %.3f\n", linearTime);
    }

    MPI_Finalize();
    free(matrixA);
    free(matrixB);
    free(resultMatrix);
    return 0;
}