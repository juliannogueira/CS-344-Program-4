#ifndef LINE_PROCESSOR_H
#define LINE_PROCESSOR_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
 *
 */

struct LineProcessor {
    int *is_running;
    int *process_index;
    int *output_index;
    int *current_index;
    int *input_count;
    int *process_count;
    int *output_count;
    int *buffer_size;
    int *output_size;
    char *input_buffer;
    char *process_buffer;
    char *output_buffer;
};

void run_line_processor(void);

void init_line_processor(struct LineProcessor *lp);

void read_input(struct LineProcessor *lp);

void convert_line_seperator(struct LineProcessor *lp);

void convert_plus_sign(struct LineProcessor *lp);

void print_output(struct LineProcessor *lp); 

void free_line_processor(struct LineProcessor *lp);

#endif