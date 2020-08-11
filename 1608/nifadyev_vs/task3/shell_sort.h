#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpich/mpi.h>

void GenerateArray(int *array, const int length);
void PrintArray(int *array, const int length);
void CopyArray(int *sourceArray, int *destinationArray, const int length);
int AreResultsEqual(int *linearResultingArray, int *parallelResultingArray,
                    const int length);

void ShellSort(int *array, const int length);
int *Merge(int *firstArray, const int firstArrayLength, int *secondArray,
           const int secondArrayLength);
void ParallelShellSort(int *array, int length, const int procNum,
                       const int procRank, double &time, int *resultingArray);
