#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initializeProcessInput(int input[], int inputSize, int processRank, int world_size, int *arrayToAssignResult);

int getPivot(int numbersToSort[], int numbersToSortLength);

void assignInputBiggerThanPivot(int input[], int inputLength, int pivot, int*arrayToAssignResult);
void assignInputSmallerThanPivot(int input[], int inputLength, int pivot, int*arrayToAssignResult);

int main(int argc, char **argv) {
    int numbersToSort[] = {1, 9, 10, 5, 4, 7, 100, 1000, 34, 56, 342, 68, 433, 938457};
    int numbersToSortLength = 14;

    MPI_Init(NULL, NULL);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank != 0) {
        int *processInputSegment = malloc(((world_size / world_rank) + 1) * sizeof(int));
        initializeProcessInput(numbersToSort, sizeof(numbersToSort) / sizeof(numbersToSort[0]), world_rank, world_size,
                               processInputSegment);

        int pivot;
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int *inputsSmallerThanPivot = malloc(((world_size / world_rank) + 1) * sizeof(int));
        int *inputsBiggerThanPivot = malloc(((world_size / world_rank) + 1) * sizeof(int));
        assignInputSmallerThanPivot(processInputSegment, numbersToSortLength, pivot, inputsSmallerThanPivot);
        assignInputBiggerThanPivot(processInputSegment, numbersToSortLength, pivot, inputsBiggerThanPivot);

    } else {
        int pivot = getPivot(numbersToSort, numbersToSortLength);
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}

void assignInputSmallerThanPivot(int input[], int inputSize, int pivot, int *arrayToAssignResult) {
    int *inputSmallerThanPivot = malloc(inputSize * sizeof(int));
    int inputSmallerThanPivotIndex = 0;
    for (int i=0; i < inputSize; i++) {
        if(input[i] < pivot) {
            inputSmallerThanPivot[inputSmallerThanPivotIndex] = input[i];
            inputSmallerThanPivotIndex++;
        }
    }
    memcpy(arrayToAssignResult, inputSmallerThanPivot, (int) sizeof(inputSmallerThanPivot) * sizeof(int));
}

void assignInputBiggerThanPivot(int input[], int inputSize, int pivot, int*arrayToAssignResult) {
    int *inputBiggerThanPivot = malloc(inputSize * sizeof(int));
    int inputBiggerThanPivotIndex = 0;
    for (int i=0; i <= inputSize; i++) {
        if(input[i] < pivot) {
            inputBiggerThanPivot[inputBiggerThanPivotIndex] = input[i];
            inputBiggerThanPivotIndex++;
        }
    }
    memcpy(arrayToAssignResult, inputBiggerThanPivot, (int) sizeof(inputBiggerThanPivot) * sizeof(int));
}

int getPivot(int numbersToSort[], int numbersToSortLength) {
    return numbersToSort[arc4random() % numbersToSortLength];
};

void initializeProcessInput(int input[], int inputSize, int processRank, int world_size, int *arrayToAssignResult) {
    float nonMasterProcessWorldSize = world_size - 1;
    int processInputFragmentEndIndex = (int) ((processRank / nonMasterProcessWorldSize) * inputSize);
    int processInputFragmentStartIndex =
            processRank - 1 == 0 ? 0 : (int) (((processRank - 1) / nonMasterProcessWorldSize) * inputSize);

    int *processInputSegment = malloc(((world_size / processRank) + 1) * sizeof(int));

    int processInputSegmentCurrentIndex = 0;
    for (int i = processInputFragmentStartIndex; i < processInputFragmentEndIndex; i++) {
        processInputSegment[processInputSegmentCurrentIndex] = input[i];
        processInputSegmentCurrentIndex++;
    }
    memcpy(arrayToAssignResult, processInputSegment, (int) sizeof(processInputSegment) * sizeof(int));
    free(processInputSegment);
}


