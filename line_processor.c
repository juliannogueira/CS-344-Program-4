#include "line_processor.h"
#include "util.h"

/*
 *
 */
void runLineProcessor(void) {
    struct LineProcessor *lp = malloc(sizeof(struct LineProcessor));
    
    initLineProcessor(lp);

    while (*(lp->isRunning)) {
        getLineToProcess(lp);
        outputLine(lp);
        // sleep(3);
    }

    freeLineProcessor(lp);
}

/*
 * Initialize the line processor struct.
 */
void initLineProcessor(struct LineProcessor *lp) {
    lp->isRunning = malloc(sizeof(int));
    lp->beginIndex = malloc(sizeof(int));
    lp->endIndex = malloc(sizeof(int));
    lp->count = malloc(sizeof(int));
    lp->bufferSize = malloc(sizeof(int));
    lp->outputSize = malloc(sizeof(int));
    *(lp->isRunning) = 1;
    *(lp->beginIndex) = 0;
    *(lp->endIndex) = 0;
    *(lp->count) = 0;
    *(lp->bufferSize) = 65536;
    *(lp->outputSize) = 10;
    lp->inputBuffer = malloc(sizeof(char) * *(lp->bufferSize));
    lp->processBuffer = malloc(sizeof(char) * *(lp->bufferSize));
    lp->outputBuffer = malloc(sizeof(char) * *(lp->bufferSize));
}

/*
 *
 */
void getLineToProcess(struct LineProcessor *lp) {
    int isFinished = 0;
    char ch = '\0';

    printf(": ");
    fflush(stdout);
    fgets(lp->inputBuffer, *(lp->bufferSize), stdin);

    for (int i = 0; i < *(lp->bufferSize); i++) {
        ch = *(lp->inputBuffer + i);

        if (ch == '\n') {
            ch = ' ';
            isFinished = 1;
        }

        if (ch == '9') {
            *(lp->isRunning) = 0;
        }

        *(lp->processBuffer + *(lp->endIndex)) = ch;
        *(lp->endIndex) += 1;
        *(lp->count) += 1;

        if (isFinished) {
            break;
        }
    }
}

/*
 *
 */
void outputLine(struct LineProcessor *lp) {
    while (*lp->count >= *(lp->outputSize)) {
        for (int i = 0; i < *(lp->outputSize); i++) {
            *(lp->outputBuffer + i) = *(lp->processBuffer + *(lp->beginIndex) + i);
        }
        outputBuffer(lp->outputBuffer, *(lp->outputSize));
        *(lp->beginIndex) += *(lp->outputSize);
        *(lp->count) -= *(lp->outputSize);
    }
    printf("\n");
    fflush(stdout);
}

/*
 * Free the line processor struct.
 */
void freeLineProcessor(struct LineProcessor *lp) {
    free(lp->isRunning);
    free(lp->beginIndex);
    free(lp->endIndex);
    free(lp->count);
    free(lp->bufferSize);
    free(lp->outputSize);
    free(lp->inputBuffer);
    free(lp->processBuffer);
    free(lp->outputBuffer);
    free(lp);
}