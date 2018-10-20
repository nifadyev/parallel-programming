#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Task # 27: Count the number of mismatched characters in two strings
// Представлять матрицы как одномерный массив, std, шаблонов, макс низкоуровневый код, string можно
// TODO: compare the result of linear realization and parallel

void InitializeStrings(char *string1, char *string2, const int size);
int CountMismatchesInTwoStrings(char *string1, char *string2, const int endIndex);


int main(int argc, char **argv)
{
    int MAX_SIZE;
    char *str1, *str2;
    char *temp1, *temp2;
    int linearResult = 0, parallelResult = 0, mismatchCount = 0;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    int procNum, procRank = 0;
    int i, taskSize, taskRemainder; // TODO: change to partSize
    MPI_Status status;

    //TODO: CHANGE into procRank
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
    InitializeStrings(str1, str2, MAX_SIZE);

    //TODO: Add check for MPI_SUCCESS
    // Initialize MPI
    //MPI_Init(&argc, &argv);
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        free(str1);
        free(str2);
        return 1;
    }
    // Get the number of processors
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    // Get rank of current process
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    taskSize = MAX_SIZE / procNum;
    taskRemainder = MAX_SIZE % procNum;

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

        for (i = 1; i < procNum; i++)
        {
            if (procNum - 1 == i)
            {
                MPI_Send(&str1[taskSize * i], taskSize + taskRemainder, MPI_CHAR, i, 1, MPI_COMM_WORLD);
                //printf("sent part of string1 with rem\n");
                MPI_Send(&str2[taskSize * i], taskSize + taskRemainder, MPI_CHAR, i, 2, MPI_COMM_WORLD);
                //printf("sent part of string2 with rem\n");
            }
            else
            {
                MPI_Send(&str1[taskSize * i], taskSize, MPI_CHAR, i, 1, MPI_COMM_WORLD);
                //printf("sent part of string1\n");
                MPI_Send(&str2[taskSize * i], taskSize, MPI_CHAR, i, 2, MPI_COMM_WORLD);
                //printf("sent part of string2\n");
            }
        }
    }

    if (procRank != 0)
    {
        //TODO: if it works then create a special flag to decrease code volume
        if (procRank == procNum - 1)
        {
            /* char * */ temp1 = (char *)malloc(sizeof(char) * (taskSize + taskRemainder));
            /* char * */ temp2 = (char *)malloc(sizeof(char) * (taskSize + taskRemainder));
            MPI_Recv(temp1, taskSize + taskRemainder, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            //printf("received part of string1 with pos rem on proc %d\n", procRank);
            MPI_Recv(temp2, taskSize + taskRemainder, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
            //printf("received part of string2 with pos rem on proc %d\n", procRank);
            mismatchCount = CountMismatchesInTwoStrings(temp1, temp2, taskSize + taskRemainder);
        }
        else
        {
            /* char * */ temp1 = (char *)malloc(sizeof(char) * taskSize);
            /* char * */ temp2 = (char *)malloc(sizeof(char) * taskSize);
            MPI_Recv(temp1, taskSize, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            //printf("received part of string1 on proc %d\n", procRank);
            MPI_Recv(temp2, taskSize, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
            //printf("received part of string2 on proc %d\n", procRank);
            mismatchCount = CountMismatchesInTwoStrings(temp1, temp2, taskSize);
        }
    }
    if (procRank == 0)
    {
        mismatchCount = CountMismatchesInTwoStrings(str1, str2, taskSize);
    }

    // Collect the result into main processor
    MPI_Reduce(&mismatchCount, &parallelResult, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (procRank == 0 && procNum > 1)
    {
        endTime = MPI_Wtime();
        parallelTime = endTime - startTime;
        if (linearResult != parallelResult)
        {
            printf("Error! Results are not equal\n");
        }
        printf("MPI time = %.3f\n", endTime - startTime);
        printf("MPI result = %d\n", parallelResult);
        printf("Performance difference: %.1f%\n", ((linearTime - parallelTime) / ((linearTime + parallelTime) / 2)) * 100);
    }

    MPI_Finalize();
    free(str1);
    free(str2);

    return 0;
}

void InitializeStrings(char *string1, char *string2, const int size)
{
    int i;
    //string1 = (char *)malloc(sizeof(char) * (size + 1));
    //string2 = (char *)malloc(sizeof(char) * (size + 1));
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
