#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpich/mpi.h>

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

// void ShellSort(int *array, const int length)
// {
//     // int gap = length / 2;  //
//     int gap, i, j; //
//     int temp;

//     for (gap = length / 2; gap > 0; gap /= 2)
//     {
//         // Do a gapped insertion sort for this gap size.
//         // The first gap elements a[0..gap-1] are already in gapped order
//         // keep adding one more element until the entire array is
//         // gap sorted
//         for (i = gap; i < length; i++)
//         {
//             // add a[i] to the elements that have been gap sorted
//             // save a[i] in temp and make a hole at position i
//             temp = array[i];
//             // shift earlier gap-sorted elements up until the correct
//             // location for a[i] is found
//             for (j = i; j >= gap && array[j - gap] > temp; j -= gap)
//             {
//                 array[j] = array[j - gap];
//             }
//             //  put temp (the original a[i]) in its correct location
//             array[j] = temp;
//         }
//     }
// }

void ShellSort(int *array, const int length)
{
    int gap = length / 2;
    int i, j, k, temp;
    while(gap > 0)
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
        gap /= 2;
    }
}

void Merge(int *firstArray, const int firstArrayLength, int * secondArray, const int secondArrayLength, int *resultArray)
{
    int i, k = 0, j = 0;
    for (i = 0; i < firstArrayLength + secondArrayLength; i++)
    {
        if (j > firstArrayLength - 1)
        {
            resultArray[i] = secondArray[k];
            k++;
        }
        else if (k > secondArrayLength - 1)
        {
            resultArray[i] = firstArray[j];
            j++;
        }
        else if (firstArray[j] <= secondArray[k])
        {
            resultArray[i] = firstArray[j];
            j++;
        }
        else
        {
            resultArray[i] = secondArray[k];
            k++;
        }
    }
}

