#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initializeProcessInput(int input[], int inputSize, int processRank, int world_size, int *arrayToAssignResult);
int main(int argc, char** argv) {
    int numbersToSort[] = {1, 9, 10, 5, 4, 7, 100, 1000, 34, 56, 342, 68, 433, 938457};

    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank != 0) {
        int *processInputSegment = malloc(((world_size / world_rank) + 1) * sizeof(int));
        initializeProcessInput(numbersToSort, sizeof(numbersToSort)/sizeof(numbersToSort[0]), world_rank, world_size, processInputSegment);
        printf("%d \n", processInputSegment[0]);
//        printf("%d \n", processInputSegment[1]);
//        printf("%d \n", processInputSegment[2]);
    }

    MPI_Finalize();
}

void initializeProcessInput(int input[], int inputSize, int processRank, int world_size, int *arrayToAssignResult) {
    float nonMasterProcessWorldSize = world_size - 1;
    int processInputFragmentEndIndex = (int) ((processRank/nonMasterProcessWorldSize) * inputSize);
    int processInputFragmentStartIndex = processRank - 1 == 0 ? 0 : (int) (((processRank - 1) / nonMasterProcessWorldSize) * inputSize);

    int *processInputSegment = malloc(((world_size / processRank) + 1) * sizeof(int));

    int processInputSegmentCurrentIndex = 0;
    for(int i=processInputFragmentStartIndex; i< processInputFragmentEndIndex; i++) {
        processInputSegment[processInputSegmentCurrentIndex] = input[i];
        processInputSegmentCurrentIndex++;
    }
    memcpy(arrayToAssignResult, processInputSegment, (int)sizeof(processInputSegment) * sizeof(int));
    free(processInputSegment);
}


