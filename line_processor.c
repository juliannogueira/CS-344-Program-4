#include "line_processor.h"

/*
 * Init the LineProcessor struct and create threads to perform the processing.
 *
 * Wait for the threads to finish, then free the struct.
 */
void run_line_processor(void) {
    struct LineProcessor *lp = malloc(sizeof(struct LineProcessor));
    
    init_line_processor(lp);
    init_line_processor_threads(lp);

    pthread_create(lp->input_t, NULL, read_input, (void *) lp);
    pthread_create(lp->line_separator_t, NULL, convert_line_seperator, (void *) lp);
    pthread_create(lp->plus_sign_t, NULL, convert_plus_sign, (void *) lp);
    pthread_create(lp->output_t, NULL, print_output, (void *) lp);

    pthread_join(*(lp->input_t), NULL);
    pthread_join(*(lp->line_separator_t), NULL);
    pthread_join(*(lp->plus_sign_t), NULL);
    pthread_join(*(lp->output_t), NULL);

    free_line_processor(lp);
}

/*
 * Initialize the line processor struct.
 */
void init_line_processor(struct LineProcessor *lp) {
    lp->is_running = malloc(sizeof(int));
    lp->process_index = malloc(sizeof(int));
    lp->output_index = malloc(sizeof(int));
    lp->current_index = malloc(sizeof(int));
    lp->input_count = malloc(sizeof(int));
    lp->process_count = malloc(sizeof(int));
    lp->output_count = malloc(sizeof(int));
    lp->buffer_size = malloc(sizeof(int));
    lp->output_size = malloc(sizeof(int));
    *(lp->is_running) = 1;
    *(lp->process_index) = 0;
    *(lp->output_index) = 0;
    *(lp->current_index) = 0;
    *(lp->input_count) = 0;
    *(lp->process_count) = 0;
    *(lp->output_count) = 0;
    *(lp->buffer_size) = 65536;
    *(lp->output_size) = 10;
    lp->input_buffer = malloc(sizeof(char) * *(lp->buffer_size));
    lp->process_buffer = malloc(sizeof(char) * *(lp->buffer_size));
    lp->output_buffer = malloc(sizeof(char) * *(lp->buffer_size));
}

/*
 *
 */
void init_line_processor_threads(struct LineProcessor *lp) {
    lp->input_t_term = malloc(sizeof(int));
    lp->line_separator_t_term = malloc(sizeof(int));
    lp->plus_sign_t_term = malloc(sizeof(int));
    lp->output_t_term = malloc(sizeof(int));
    lp->input_t = malloc(sizeof(pthread_t));
    lp->line_separator_t = malloc(sizeof(pthread_t));
    lp->plus_sign_t = malloc(sizeof(pthread_t));
    lp->output_t = malloc(sizeof(pthread_t));
    lp->input_lock = malloc(sizeof(pthread_mutex_t));
    lp->process_lock = malloc(sizeof(pthread_mutex_t));
    lp->output_lock = malloc(sizeof(pthread_mutex_t));
    lp->has_input = malloc(sizeof(pthread_cond_t));
    lp->has_processed_input = malloc(sizeof(pthread_cond_t));
    lp->has_processed_line_separator = malloc(sizeof(pthread_cond_t));
    lp->has_output = malloc(sizeof(pthread_cond_t));
    *(lp->input_t_term) = 0;
    *(lp->line_separator_t_term) = 0;
    *(lp->plus_sign_t_term) = 0;
    *(lp->output_t_term) = 0;
    pthread_mutex_init(lp->input_lock, NULL);
    pthread_mutex_init(lp->process_lock, NULL);
    pthread_mutex_init(lp->output_lock, NULL);
    pthread_cond_init(lp->has_input, NULL);
    pthread_cond_init(lp->has_processed_input, NULL);
    pthread_cond_init(lp->has_processed_line_separator, NULL);
    pthread_cond_init(lp->has_output, NULL);
}

/*
 * Read input from stdin.
 *
 * While reading a line from stdin, hold the input lock. After the line has been
 * read, signal that the buffer has input, and release the input lock.
 */
void *read_input(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running)) {

        int is_finished = 0;
        int index = 0;
        char ch = '\0';

        pthread_mutex_lock(lp->input_lock);
        // printf("t1 -> Holding input_lock\n");

        while (*(lp->input_count) > 0) {
            // printf("t1 -> Waiting for has_processed_input and input_lock\n");
            pthread_cond_wait(lp->has_processed_input, lp->input_lock);
            // printf("t1 -> Finished waiting for has_processed_input and input_lock\n");
        }

        while (!is_finished) {

            ch = getchar();
            *(lp->input_buffer + index) = ch;

            // printf("ib___%d___%c___%d\n", index, ch, ch);

            if (ch == '\n' || ch == EOF) {
                is_finished = 1;
                if (ch == EOF) {
                    *(lp->is_running) = 0;
                }
            }

            *(lp->input_count) += 1;
            index += 1;
        }

        pthread_cond_signal(lp->has_input);
        // printf("t1 -> Signaled has_input\n");
        pthread_mutex_unlock(lp->input_lock);
        // printf("t1 -> Released input_lock\n");
    }

    *(lp->input_t_term) = 1;

    // printf("t1 -> Terminating\n");
    
    return NULL;
}

/*
 *
 */
void *convert_line_seperator(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running) || *(lp->input_count) > 0 || !*(lp->input_t_term)) {
        char ch = '\0';

        pthread_mutex_lock(lp->input_lock);
        // printf("t2 -> Holding input lock\n");

        while (*(lp->input_count) == 0) {
            // printf("t2 -> Waiting for has_input and input_lock\n");
            pthread_cond_wait(lp->has_input, lp->input_lock);
            // printf("t2 -> Finished waiting for has_input and input_lock\n");
        }

        pthread_mutex_lock(lp->process_lock);
        // printf("t2 -> Holding process lock\n");

        for (int i = 0; i < *(lp->input_count); i++) {
            ch = *(lp->input_buffer + i);

            if (ch == 10) {
                ch = ' ';
            }
            
            if (ch >= 32 && ch <= 126) {
                // printf("___%d___%c", i, ch);
                *(lp->process_buffer + *(lp->process_index)) = ch;
                *(lp->process_index) += 1;
                *(lp->process_count) += 1;
            }
        }

        // *(lp->process_count) = *(lp->input_count);
        *(lp->input_count) = 0;

        pthread_cond_signal(lp->has_processed_line_separator);
        // printf("t2 -> Signaled has_processed_line_separator\n");

        pthread_mutex_unlock(lp->process_lock);
        // printf("t2 -> Released process_lock\n");

        pthread_cond_signal(lp->has_processed_input);
        // printf("t2 -> Signaled has_processed_input\n");

        pthread_mutex_unlock(lp->input_lock);
        // printf("t2 -> Released input_lock\n");
    }

    *(lp->line_separator_t_term) = 1;

    // printf("t2 -> Terminating\n");

    return NULL;
}

/*
 *
 */
void *convert_plus_sign(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running) || *(lp->process_count) > 0 || !*(lp->line_separator_t_term)) {
        pthread_mutex_lock(lp->process_lock);
        // printf("t3 -> Holding process_lock\n");

        while (*(lp->process_count) == 0) {
            // printf("t3 -> Waiting for has_processed_line_separator and process_lock\n");
            pthread_cond_wait(lp->has_processed_line_separator, lp->process_lock);
            // printf("t3 -> Finished waiting for has_processed_line_separator and process_lock\n");
        }

        pthread_mutex_lock(lp->output_lock);
        // printf("t3 -> Holding output_lock\n");

        int is_load_output = 0;
        int is_prev_char = 0;
        char ch = '\0';

        for (int i = 0; i < *(lp->process_count); i++) {
            ch = *(lp->process_buffer + *(lp->process_index) - *(lp->process_count) + i);

            if (ch == '+') {
                if (is_prev_char) {
                    is_prev_char = 0;
                    *(lp->output_buffer + *(lp->output_index) - 1) = '^';
                } else {
                    is_prev_char = 1;
                    is_load_output = 1;
                }
            } else {
                if (is_prev_char) {
                    is_prev_char = 0;
                }
                is_load_output = 1;
            }

            if (is_load_output) {
                *(lp->output_buffer + *(lp->output_index)) = ch;
                *(lp->output_index) += 1;
                *(lp->output_count) += 1;
                is_load_output = 0;
            }
        }

        // The location of this reset may need to be changed.
        *(lp->process_count) = 0;

        pthread_cond_signal(lp->has_output);
        // printf("t3 -> Signaled has_output\n");

        pthread_mutex_unlock(lp->output_lock);
        // printf("t3 -> Released output_lock\n");

        pthread_mutex_unlock(lp->process_lock);
        // printf("t3 -> Released process_lock\n");
    }

    *(lp->plus_sign_t_term) = 1;

    // printf("t3 -> Terminating\n");

    return NULL;
}

/*
 *
 */
void *print_output(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running) || *(lp->output_count) >= *(lp->output_size)\
    || !*(lp->plus_sign_t_term)) {
        pthread_mutex_lock(lp->output_lock);
        // printf("t4 -> holding output_lock\n");

        while (*(lp->output_count) < *(lp->output_size)) {
            // printf("t4 -> Waiting for has_output and output_lock\n");
            pthread_cond_wait(lp->has_output, lp->output_lock);
            // printf("t4 -> Finished waiting for has_output and output_lock\n");
        }

        while (*(lp->output_count) >= *(lp->output_size)) {
            for (int i = 0; i < *(lp->output_size); i++) {
                putchar(*(lp->output_buffer + *(lp->current_index) + i));
                // printf("___%d___%c\n", i, *(lp->output_buffer + *(lp->current_index) + i));
                fflush(stdout);
            }
            *(lp->current_index) += *(lp->output_size);
            *(lp->output_count) -= *(lp->output_size);
            printf("\n");
            fflush(stdout);
        }

        // printf("t4 -> Signaled has_output\n");
        pthread_cond_signal(lp->has_output);
        // printf("t4 -> Released output_lock\n");
        pthread_mutex_unlock(lp->output_lock);
    }

    *(lp->output_t_term) = 1;

    // printf("t4 -> Terminating\n");

    return NULL;
}

/*
 * Free the line processor struct.
 */
void free_line_processor(struct LineProcessor *lp) {
    free(lp->is_running);
    free(lp->process_index);
    free(lp->output_index);
    free(lp->current_index);
    free(lp->input_count);
    free(lp->process_count);
    free(lp->output_count);
    free(lp->buffer_size);
    free(lp->output_size);
    free(lp->input_t_term);
    free(lp->line_separator_t_term);
    free(lp->plus_sign_t_term);
    free(lp->output_t_term);
    free(lp->input_buffer);
    free(lp->process_buffer);
    free(lp->output_buffer);
    free(lp->input_t);
    free(lp->line_separator_t);
    free(lp->plus_sign_t);
    free(lp->output_t);
    free(lp->input_lock);
    free(lp->process_lock);
    free(lp->output_lock);
    free(lp->has_input);
    free(lp->has_processed_input);
    free(lp->has_processed_line_separator);
    free(lp->has_output);
    free(lp);
}