#ifndef LINE_PROCESSOR_H
#define LINE_PROCESSOR_H

/*
 *
 */

struct LineProcessor {
    int *isRunning;
    int *beginIndex;
    int *endIndex;
    int *count;
    int *bufferSize;
    int *outputSize;
    char *inputBuffer;
    char *processBuffer;
    char *outputBuffer;
};

void runLineProcessor(void);

void initLineProcessor(struct LineProcessor *lp);

void getLineToProcess(struct LineProcessor *lp);

void outputLine(struct LineProcessor *lp); 

void freeLineProcessor(struct LineProcessor *lp);

#endif