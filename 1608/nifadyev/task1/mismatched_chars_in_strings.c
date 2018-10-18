#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Task # 27: Count the number of mismatched characters in two strings
// Представлять матрицы как одномерный массив, std, шаблонов, макс низкоуровневый код, string можно
// TODO: compare the result of linear realization and parallel
int main(int argc, char **argv)
{
    int MAX_SIZE;
    char *str1, *str2;
    //char *temp1, *temp2;
    int linearResult = 0, parallelResult = 0, mismatchCount = 0;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    int procNum, procRank;
    int i, taskSize, taskRemainder;
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
    srand(time(NULL));
    //Fill strings with random characters
    for (i = 0; i < MAX_SIZE; i++)
    {
        str1[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
        str2[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
    }
    str1[MAX_SIZE] = '\0';
    str2[MAX_SIZE] = '\0';
    // printf("%s\n", str1);
    // printf("%s\n", str2);

    //TODO: Add check for MPI_SUCCESS
    // Initialize MPI
    MPI_Init(&argc, &argv);
    // if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    // {
    //     free(str1);
    //     free(str2);
    //     free(temp1);
    //     free(temp2);
    //     return 1;
    // }
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
        for (i = 0; i < MAX_SIZE; i++)
        {
            if (str1[i] != str2[i])
            {
                linearResult++;
            }
        }
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
                printf("sent part of string1 with rem\n");
                MPI_Send(&str2[taskSize * i], taskSize + taskRemainder, MPI_CHAR, i, 2, MPI_COMM_WORLD);
                printf("sent part of string2 with rem\n");
            }
            else
            {
                MPI_Send(&str1[taskSize * i], taskSize, MPI_CHAR, i, 1, MPI_COMM_WORLD);
                printf("sent part of string1\n");
                MPI_Send(&str2[taskSize * i], taskSize, MPI_CHAR, i, 2, MPI_COMM_WORLD);
                printf("sent part of string2\n");
            }
        }
        // Broadcast first and second strings to each processor
        //MPI_Bcast(&taskSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
        //MPI_Bcast(&MAX_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (procRank != 0)
    {
        //TODO: if it works then create a special flag to decrease code volume
        if (procRank == procNum - 1)
        {
            char* temp1 = (char *)malloc(sizeof(char) * (taskSize + taskRemainder));
            char* temp2 = (char *)malloc(sizeof(char) * (taskSize + taskRemainder));
            MPI_Recv(temp1, taskSize + taskRemainder, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            printf("received part of string1 with pos rem on proc %d\n", procRank);
            MPI_Recv(temp2, taskSize + taskRemainder, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
            printf("received part of string2 with pos rem on proc %d\n", procRank);
            for (i = 0; i < taskSize + taskRemainder; i++)
            {
                if (temp1[i] != temp2[i])
                {
                    mismatchCount++;
                }
            }
            // free(temp1);
            // free(temp2);
        }
        else
        {
            char* temp1 = (char *)malloc(sizeof(char) * taskSize);
            char* temp2 = (char *)malloc(sizeof(char) * taskSize);
            MPI_Recv(temp1, taskSize, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            printf("received part of string1 on proc %d\n", procRank);
            MPI_Recv(temp2, taskSize, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
            printf("received part of string2 on proc %d\n", procRank);
            //for (i = taskSize * procRank; i < taskSize * (procRank + 1); i++)
            for (i = 0; i < taskSize; i++)
            {
                if (temp1[i] != temp2[i])
                {
                    mismatchCount++;
                }
            }
            // free(temp1);
            // free(temp2);
        }
    }
    if (procRank == 0)
    {
        for (i = taskSize * procRank; i < taskSize * (procRank + 1); i++)
        {
            if (str1[i] != str2[i])
            {
                mismatchCount++;
            }
        }
    }

    // Collect the result into main processor
    MPI_Reduce(&mismatchCount, &parallelResult, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (procRank == 0 && procNum > 1)
    {
        endTime = MPI_Wtime();
        parallelTime = endTime - startTime;
        printf("MPI time = %.3f\n", endTime - startTime);
        printf("MPI result = %d\n", parallelResult);
        printf("Performance difference: %.1f%\n", ((linearTime - parallelTime) / ((linearTime + parallelTime) / 2)) * 100);
    }

    

    MPI_Finalize();
    //FIXME: if comment free str1 & str2 then itll work with -n 2
    free(str1);
    free(str2);
    // if (procNum > 1)
    // {
    //     free(temp1);
    //     free(temp2);
    // }
    return 0;
}
