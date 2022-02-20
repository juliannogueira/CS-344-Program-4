#ifndef LINE_PROCESSOR_H
#define LINE_PROCESSOR_H

/*
 *
 */

struct LineProcessor {
    int *isRunning;
    // int *beginIndex;
    // int *currentIndex;
    int *processIndex;
    int *outputIndex;
    int *currentIndex;
    int *inputCount;
    int *processCount;
    int *outputCount;
    int *bufferSize;
    int *outputSize;
    char *inputBuffer;
    char *processBuffer;
    char *outputBuffer;
};

void runLineProcessor(void);

void initLineProcessor(struct LineProcessor *lp);

void readInput(struct LineProcessor *lp);

void convertLineSeperator(struct LineProcessor *lp);

void convertPlusSign(struct LineProcessor *lp);

void printOutput(struct LineProcessor *lp); 

void freeLineProcessor(struct LineProcessor *lp);

#endif