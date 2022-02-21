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
    int *input_t_term;
    int *line_separator_t_term;
    int *plus_sign_t_term;
    int *output_t_term;
    char *input_buffer;
    char *process_buffer;
    char *output_buffer;
    pthread_t *input_t;
    pthread_t *line_separator_t;
    pthread_t *plus_sign_t;
    pthread_t *output_t;
    pthread_mutex_t *input_lock;
    pthread_mutex_t *process_lock;
    pthread_mutex_t *output_lock;
    pthread_cond_t *has_input;
    pthread_cond_t *has_processed_input;
    pthread_cond_t *has_processed_line_separator;
    pthread_cond_t *has_processed_plus_sign;
    pthread_cond_t *has_output;
};

void run_line_processor(void);

void init_line_processor(struct LineProcessor *lp);

void init_line_processor_threads(struct LineProcessor *lp);

void *read_input(void *arg);

void *convert_line_seperator(void *arg);

void *convert_plus_sign(void *arg);

void *print_output(void *arg); 

void free_line_processor(struct LineProcessor *lp);

#endif