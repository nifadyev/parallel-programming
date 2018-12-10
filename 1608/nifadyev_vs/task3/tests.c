#include <gtest/gtest.h>
#include "shell_sort.h"

TEST(ShellSort, can_generate_array)
{
    int size = 100;
    int *array = (int *)malloc(sizeof(int) * size);
    ASSERT_NO_THROW(GenerateArray(array, size));
}

TEST(ShellSort, can_copy_array)
{
    int sourceArray[] = {1, 4, 6, 23, 100, 453};
    int destinationArray[6];

    ASSERT_NO_THROW(CopyArray(sourceArray, destinationArray, 6));
}

TEST(ShellSort, can_correctle_copy_array)
{
    int i;
    int sourceArray[] = {1, 4, 6, 23, 100, 453};
    int destinationArray[6];

    CopyArray(sourceArray, destinationArray, 6);

    for (i = 0; i < 6; i++)
    {
        EXPECT_EQ(sourceArray[i], destinationArray[i]);
    }
}

TEST(ShellSort, can_sort_array)
{
    int array[] = {1, 4, 2, 10, 14, 5, 17, 3, 4, 7};

    ASSERT_NO_THROW(ShellSort(array, 10));
}

TEST(ShellSort, can_correctly_sort_array)
{
    int i;
    int array[] = {1, 4, 2, 10, 14, 5, 17, 3, 4, 7};
    int expectedArray[] = {1, 2, 3, 4, 4, 5, 7, 10, 14, 17};

    ShellSort(array, 10);

    for (i = 0; i < 10; i++)
    {
        EXPECT_EQ(array[i], expectedArray[i]);
    }
}

TEST(ShellSort, cam_merge_arrays)
{
    int firstArray[] = {1, 3, 3, 5, 6, 9};
    int secondArray[] = {0, 1, 3, 4, 5, 8};
    int resultArray[12];

    ASSERT_NO_THROW(Merge(firstArray, 6, secondArray, 6));
}

TEST(ShellSort, can_correctly_merge_arrays)
{
    int i;
    int firstArray[] = {1, 3, 3, 5, 6, 9};
    int secondArray[] = {0, 1, 3, 4, 5, 8};
    int *resultArray = (int *)malloc(sizeof(int) * 12);
    int expectedArray[] = {0, 1, 1, 3, 3, 3, 4, 5, 5, 6, 8, 9};
    resultArray = Merge(firstArray, 6, secondArray, 6);

    for (i = 0; i < 12; i++)
    {
        EXPECT_EQ(resultArray[i], expectedArray[i]);
    }

    free(resultArray);
}

TEST(ShellSort, can_execute_are_results_equal)
{
    int array1[] = {1, 4, 6, 3, 6, 10, 34};
    int array2[] = {2, 5, 1, 6, 9, 12, 23};

    ASSERT_NO_THROW(AreResultsEqual(array1, array2, 7));
}

TEST(ShellSort, are_results_equal_returns_1)
{
    int array1[] = {1, 4, 6, 3, 6, 10, 34};
    int array2[] = {2, 5, 1, 6, 9, 12, 23};

    EXPECT_EQ(1, AreResultsEqual(array1, array2, 7));
}

TEST(ShellSort, are_results_equal_returns_0)
{
    int array1[] = {1, 4, 6, 3, 6, 10, 34};
    int array2[] = {1, 4, 6, 3, 6, 10, 34};

    EXPECT_EQ(0, AreResultsEqual(array1, array2, 7));
}

TEST(ShellSort, can_execute_parallel_shell_sort)
{
    int procNum, procRank;
    int array[] = {1, 4, 6, 4, 34, 67, 11, 4, 7, 10};
    double time;
    int resultingArray[10] = {0};

    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    ASSERT_NO_THROW(ParallelShellSort(array, 10, procNum, procRank,
                                      time, resultingArray));
}

TEST(ShellSort, can_execute_parallel_shell_sort_with_small_array)
{
    int procNum, procRank;
    int i;
    int array[] = {1, 4, 6, 4, 34, 67, 11, 4, 7, 10};
    double parallelTime;
    int resultingArray[10] = {0};
    int expectedArray[] = {1, 4, 4, 4, 6, 7, 10, 11, 34, 67};

    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    ParallelShellSort(array, 10, procNum, procRank, parallelTime, resultingArray);

    if (procRank == 0)
    {
        for (i = 0; i < 10; i++)
        {
            EXPECT_EQ(resultingArray[i], expectedArray[i]);
        }
    }
}

TEST(ShellSort, can_execute_parallel_shell_sort_with_big_array)
{
    int procNum, procRank;
    int i;
    int *array = (int *)malloc(sizeof(int) * 500);
    double parallelTime;
    int *resultingArray = (int *)malloc(sizeof(int) * 500);

    srand(time(NULL));
    for (i = 0; i < 500; i++)
    {
        array[i] = rand() % 100;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    ParallelShellSort(array, 500, procNum, procRank, parallelTime, resultingArray);

    if (procRank == 0)
    {
        for (i = 0; i < 500 - 1; i++)
        {
            EXPECT_TRUE(resultingArray[i] <= resultingArray[i + 1]);
        }
    }

    free(array);
    free(resultingArray);
}

TEST(ShellSort, can_execute_parallel_shell_sort_with_huge_array)
{
    int procNum, procRank;
    int i;
    int *array = (int *)malloc(sizeof(int) * 10000);
    double parallelTime;
    int *resultingArray = (int *)malloc(sizeof(int) * 10000);

    srand(time(NULL));
    for (i = 0; i < 10000; i++)
    {
        array[i] = rand() % 100;
    }

    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    ParallelShellSort(array, 10000, procNum, procRank, parallelTime, resultingArray);

    if (procRank == 0)
    {
        for (i = 0; i < 10000 - 1; i++)
        {
            EXPECT_TRUE(resultingArray[i] <= resultingArray[i + 1]);
        }
    }

    free(array);
    free(resultingArray);
}
