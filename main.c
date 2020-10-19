#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int getPivot(int numbersToSort[], int numbersToSortLength);

void sequentialQuicksort(int *arrayPointer, int arrayLength);

void printIntegerArray(int *arrayPointer, int arrayLength);

struct ProcessInput getProcessInputSmallerThanPivot(struct ProcessInput processInput, int pivot);

struct ProcessInput getProcessInputBiggerThanPivot(struct ProcessInput processInput, int pivot);

void handleEvenChildProcess (struct ProcessInput inputsBiggerThanPivot, struct ProcessInput inputsSamllerThanPivot, int world_rank, int numbersToSortLength);

void handleOddChildProcess (struct ProcessInput inputsSmallerThanPivot, struct ProcessInput inputsBiggerThanPivot, int world_rank, int numbersToSortLength);

struct ProcessInput getProcessInputFromArrayPointer(int *pointerToArray, int arrayLength);

struct ProcessInput mergeProcessInputs(struct ProcessInput processInput1, struct ProcessInput processInput2);

        struct ProcessInput {
    int numbersToSort[20];
    int numbersCount;
};

struct ProcessInput getProcessInput(int worldProcessCount, int processRank, int numbersToSort[], int numbersCount);

int main(int argc, char **argv) {
    int numbersToSort[] = {2000, 9, 1, 10, 100, 1000, 34, 56, 342, 5, 4, 7, 68, 433, 23, 90, 91, 76, 2, 2001, 4002};
    int numbersToSortLength = 14;

    MPI_Init(NULL, NULL);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank != 0) {
        struct ProcessInput processInput = getProcessInput(world_size, world_rank, numbersToSort, sizeof(numbersToSort) / sizeof(numbersToSort[0]));

        int pivot;
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);

        struct ProcessInput inputsSmallerThanPivot = getProcessInputSmallerThanPivot(processInput, pivot);
        struct ProcessInput inputsBiggerThanPivot = getProcessInputBiggerThanPivot(processInput, pivot);

        if (world_rank % 2 == 0) { //process 2
            handleEvenChildProcess(inputsBiggerThanPivot, inputsSmallerThanPivot, world_rank, numbersToSortLength);
        } else { //process 1
            handleOddChildProcess(inputsSmallerThanPivot, inputsBiggerThanPivot, world_rank, numbersToSortLength);
        }
    } else {
        int pivot = getPivot(numbersToSort, numbersToSortLength);
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}

void handleEvenChildProcess (struct ProcessInput inputsBiggerThanPivot, struct ProcessInput inputsSmallerThanPivot, int world_rank, int numbersToSortLength) {
    MPI_Send(inputsBiggerThanPivot.numbersToSort, inputsBiggerThanPivot.numbersCount, MPI_INT, 1, 0, MPI_COMM_WORLD);
    MPI_Send(&inputsBiggerThanPivot.numbersCount, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

    int *numbersSmallerThanPivotFromOtherProcess = malloc(world_rank * sizeof(int));
    int numbersSmallerThanPivotFromOtherProcessCount;
    MPI_Recv(numbersSmallerThanPivotFromOtherProcess, numbersToSortLength, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&numbersSmallerThanPivotFromOtherProcessCount, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    struct ProcessInput inputsSmallerThanPivotFromOtherProcesses = getProcessInputFromArrayPointer(numbersSmallerThanPivotFromOtherProcess, numbersSmallerThanPivotFromOtherProcessCount);
    struct ProcessInput mergedInputs = mergeProcessInputs(inputsSmallerThanPivotFromOtherProcesses, inputsSmallerThanPivot);
    for(int i =0; i< mergedInputs.numbersCount; i++) {
        printf("%d \n", mergedInputs.numbersToSort[i]);
    }

}

void handleOddChildProcess (struct ProcessInput inputsSmallerThanPivot, struct ProcessInput inputsBiggerThanPivot, int world_rank, int numbersToSortLength) {
    int *numbersBiggerThanPivotFromOtherProcess = malloc(world_rank * sizeof(int));
    int numbersbiggerThanPivotFromOtherProcessCount;
    MPI_Recv(numbersBiggerThanPivotFromOtherProcess, numbersToSortLength, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&numbersbiggerThanPivotFromOtherProcessCount, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Send(inputsSmallerThanPivot.numbersToSort, inputsSmallerThanPivot.numbersCount, MPI_INT, 2, 0, MPI_COMM_WORLD);
    MPI_Send(&inputsSmallerThanPivot.numbersCount, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);

    struct ProcessInput inputsBiggerThanPivotFromOtherProcesses = getProcessInputFromArrayPointer(numbersBiggerThanPivotFromOtherProcess, numbersbiggerThanPivotFromOtherProcessCount);
    struct ProcessInput mergedInputs = mergeProcessInputs(inputsBiggerThanPivotFromOtherProcesses, inputsBiggerThanPivot);

    for(int i =0; i< mergedInputs.numbersCount; i++) {
        printf("%d \n", mergedInputs.numbersToSort[i]);
    }
}

struct ProcessInput mergeProcessInputs(struct ProcessInput processInput1, struct ProcessInput processInput2) {
    struct ProcessInput processInput;
    processInput.numbersCount = processInput1.numbersCount + processInput2.numbersCount;
    for (int i=0; i < processInput1.numbersCount; i++) {
        processInput.numbersToSort[i] = processInput1.numbersToSort[i];
    }
    int processInput2Index = 0;
    for (int i = processInput1.numbersCount; i < processInput.numbersCount; i++) {
        processInput.numbersToSort[i] = processInput2.numbersToSort[processInput2Index];
        processInput2Index++;
    }
    return processInput;
}

struct ProcessInput getProcessInputFromArrayPointer(int *pointerToArray, int arrayLength) {
    struct ProcessInput processInput;
    processInput.numbersCount = arrayLength;

    for(int i =0; i < arrayLength; i++){
        processInput.numbersToSort[i] = pointerToArray[i];
    }
    return processInput;
}

void sequentialQuicksort(int *arrayPointer, int arrayLength) {
    if (arrayLength < 2) {
        return;
    }
    int pivot = arrayPointer[arrayLength / 2];
    int leftIndex, rightIndex;
    for (leftIndex = 0, rightIndex = arrayLength - 1;; leftIndex++, rightIndex--) {
        while (arrayPointer[leftIndex] < pivot) {
            leftIndex++;
        }
        while (arrayPointer[rightIndex] > pivot) {
            rightIndex--;
        }
        if (leftIndex >= rightIndex) {
            break;
        }
        int temporaryArrayPointerValue = arrayPointer[leftIndex];
        arrayPointer[leftIndex] = arrayPointer[rightIndex];
        arrayPointer[rightIndex] = temporaryArrayPointerValue;
    }

    sequentialQuicksort(arrayPointer, leftIndex);
    sequentialQuicksort(arrayPointer + leftIndex, arrayLength - leftIndex);
}

void printIntegerArray(int *arrayPointer, int arrayLength) {
    for (int i = 0; i < arrayLength; i++) {
        printf("%d|", arrayPointer[i]);
    }
    printf("\n");
}

int getPivot(int numbersToSort[], int numbersToSortLength) {
    return numbersToSort[arc4random() % numbersToSortLength];
};


struct ProcessInput getProcessInput(int worldProcessCount, int processRank, int numbersToSort[], int numbersCount) {
    int index;
    struct ProcessInput processInput;
    processInput.numbersCount = numbersCount / (worldProcessCount - 1);
    index = (processRank - 1) * numbersCount / processRank;
    memcpy(processInput.numbersToSort, &numbersToSort[index], processInput.numbersCount * sizeof(int));

    return processInput;
}

struct ProcessInput getProcessInputSmallerThanPivot(struct ProcessInput processInput, int pivot) {
    int index = 0;
    struct ProcessInput processInputSmallerThanPivot;
    for (int i = 0; i < processInput.numbersCount; i++) {
        if (processInput.numbersToSort[i] < pivot) {
            processInputSmallerThanPivot.numbersToSort[index] = processInput.numbersToSort[i];
            index++;
        }
    }
    processInputSmallerThanPivot.numbersCount = index;
    return processInputSmallerThanPivot;
}

struct ProcessInput getProcessInputBiggerThanPivot(struct ProcessInput processInput, int pivot) {
    int index = 0;
    struct ProcessInput processInputBiggerThanPivot;
    for (int i = 0; i < processInput.numbersCount; i++) {
        if (processInput.numbersToSort[i] >= pivot) {
            processInputBiggerThanPivot.numbersToSort[index] = processInput.numbersToSort[i];
            index++;
        }
    }
    processInputBiggerThanPivot.numbersCount = index;
    return processInputBiggerThanPivot;
}
