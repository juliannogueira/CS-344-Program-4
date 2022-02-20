#include "line_processor.h"
#include "util.h"

/*
 *
 */
void runLineProcessor(void) {
    struct LineProcessor *lp = malloc(sizeof(struct LineProcessor));
    
    initLineProcessor(lp);

    while (*(lp->isRunning)) {
        readInput(lp);
        convertLineSeperator(lp);
        convertPlusSign(lp);
        printOutput(lp);
    }

    freeLineProcessor(lp);
}

/*
 * Initialize the line processor struct.
 */
void initLineProcessor(struct LineProcessor *lp) {
    lp->isRunning = malloc(sizeof(int));
    lp->processIndex = malloc(sizeof(int));
    lp->outputIndex = malloc(sizeof(int));
    lp->currentIndex = malloc(sizeof(int));
    lp->inputCount = malloc(sizeof(int));
    lp->processCount = malloc(sizeof(int));
    lp->outputCount = malloc(sizeof(int));
    lp->bufferSize = malloc(sizeof(int));
    lp->outputSize = malloc(sizeof(int));
    *(lp->isRunning) = 1;
    *(lp->processIndex) = 0;
    *(lp->outputIndex) = 0;
    *(lp->currentIndex) = 0;
    *(lp->inputCount) = 0;
    *(lp->processCount) = 0;
    *(lp->outputCount) = 0;
    *(lp->bufferSize) = 65536;
    *(lp->outputSize) = 10;
    lp->inputBuffer = malloc(sizeof(char) * *(lp->bufferSize));
    lp->processBuffer = malloc(sizeof(char) * *(lp->bufferSize));
    lp->outputBuffer = malloc(sizeof(char) * *(lp->bufferSize));
}

/*
 *
 */
void readInput(struct LineProcessor *lp) {
    int isFinished = 0;
    int index = 0;
    char ch = '\0';

    while (!isFinished) {
        ch = getchar();
        *(lp->inputBuffer + index) = ch;

        if (ch == '\n' || ch == EOF) {
            isFinished = 1;
            if (ch == EOF) {
                *(lp->isRunning) = 0;
            }
        }

        *(lp->inputCount) += 1;
        index += 1;
    }
}

/*
 *
 */
void convertLineSeperator(struct LineProcessor *lp) {
    char ch = '\0';

    for (int i = 0; i < *(lp->inputCount); i++) {
        ch = *(lp->inputBuffer + i);

        if (ch == '\n') {
            ch = ' ';
        }
        
        if (ch != '\r' && ch != EOF) {
            *(lp->processBuffer + *(lp->processIndex)) = ch;
            *(lp->processIndex) += 1;
        }
    }

    *(lp->processCount) = *(lp->inputCount);
    *(lp->inputCount) = 0;
}

/*
 *
 */
void convertPlusSign(struct LineProcessor *lp) {
    int isLoadOutput = 0;
    int isPrevChar = 0;
    char ch = '\0';

    for (int i = 0; i < *(lp->processCount); i++) {
        ch = *(lp->processBuffer + *(lp->processIndex) - *(lp->processCount) + i);

        if (ch == '+') {
            if (isPrevChar) {
                isPrevChar = 0;
                *(lp->outputBuffer + *(lp->outputIndex) - 1) = '^';
            } else {
                isPrevChar = 1;
                isLoadOutput = 1;
            }
        } else {
            if (isPrevChar) {
                isPrevChar = 0;
            }
            isLoadOutput = 1;
        }

        if (isLoadOutput) {
            *(lp->outputBuffer + *(lp->outputIndex)) = ch;
            *(lp->outputIndex) += 1;
            *(lp->outputCount) += 1;
            isLoadOutput = 0;
        }
    }

    // The location of this reset may need to be changed.
    *(lp->processCount) = 0;
}

/*
 *
 */
void printOutput(struct LineProcessor *lp) {
    while (*(lp->outputCount) >= *(lp->outputSize)) {
        for (int i = 0; i < *(lp->outputSize); i++) {
            putchar(*(lp->outputBuffer + *(lp->currentIndex) + i));
            fflush(stdout);
        }
        *(lp->currentIndex) += *(lp->outputSize);
        *(lp->outputCount) -= *(lp->outputSize);
        printf("\n");
        fflush(stdout);
    }
}

/*
 * Free the line processor struct.
 */
void freeLineProcessor(struct LineProcessor *lp) {
    free(lp->isRunning);
    free(lp->processIndex);
    free(lp->outputIndex);
    free(lp->currentIndex);
    free(lp->inputCount);
    free(lp->processCount);
    free(lp->outputCount);
    free(lp->bufferSize);
    free(lp->outputSize);
    free(lp->inputBuffer);
    free(lp->processBuffer);
    free(lp->outputBuffer);
    free(lp);
}