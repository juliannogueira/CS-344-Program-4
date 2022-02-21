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
    lp->is_end_sequence_char = malloc(sizeof(int));
    lp->is_end_sequence = malloc(sizeof(int));
    lp->process_index = malloc(sizeof(int));
    lp->output_index = malloc(sizeof(int));
    lp->current_index = malloc(sizeof(int));
    lp->input_count = malloc(sizeof(int));
    lp->process_count = malloc(sizeof(int));
    lp->output_count = malloc(sizeof(int));
    lp->input_buffer_size = malloc(sizeof(int));
    lp->buffer_size = malloc(sizeof(int));
    lp->output_size = malloc(sizeof(int));
    *(lp->is_running) = 1;
    *(lp->is_end_sequence_char) = 0;
    *(lp->is_end_sequence) = 0;
    *(lp->process_index) = 0;
    *(lp->output_index) = 0;
    *(lp->current_index) = 0;
    *(lp->input_count) = 0;
    *(lp->process_count) = 0;
    *(lp->output_count) = 0;
    *(lp->input_buffer_size) = 1024;
    *(lp->buffer_size) = 65536;
    *(lp->output_size) = 80;
    lp->input_buffer = malloc(sizeof(char) * *(lp->input_buffer_size));
    lp->process_buffer = malloc(sizeof(char) * *(lp->buffer_size));
    lp->output_buffer = malloc(sizeof(char) * *(lp->buffer_size));
}

/*
 * Initialize the thread attributes in the line processor struct.
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
 * Check if the end sequence was encountered.
 * 
 * The end sequence is STOP, followed by a newline character.
 * 
 * If the end sequence is encountered, set the input count to zero, as it will
 * prevent output from the current line to be printed, that is the end sequence
 * will not be printed.
 * 
 * Update the end sequence attribute to signal all threads to finish their
 * work, then break out of their loops.
 */
void check_is_end_sequence(struct LineProcessor *lp, int index, char ch) {
    if (index == 0 && ch == 'S') {
        *(lp->is_end_sequence_char) = 1;
    } else if (*(lp->is_end_sequence_char)) {
        if (index == 1 && ch == 'T') {
            return;
        } else if (index == 2 && ch == 'O') {
            return;
        } else if (index == 3 && ch == 'P') {
            return;
        } else if (index == 4 && (ch == '\r' || ch =='\n')) {
            *(lp->is_end_sequence_char) = 0;
            *(lp->is_end_sequence) = 1;
            *(lp->input_count) = 0;
            *(lp->is_running) = 0;
        } else {
            *(lp->is_end_sequence_char) = 0;
        }
    }
}

/*
 * Read input from stdin into the input buffer.
 *
 * While reading a line from stdin, hold the input lock. Increment the input
 * count for each read character. After the line has been read, signal that the
 * buffer has input, and release the input lock.
 * 
 * Check if the input count is greater than zero. If so, wait for the next
 * thread to finish consuming the input. The next thread will set the input
 * count back to zero, and the loop will break.
 */
void *read_input(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running)) {
        int is_finished = 0;
        int index = 0;
        char ch = '\0';

        pthread_mutex_lock(lp->input_lock);

        while (*(lp->input_count) > 0) {
            pthread_cond_wait(lp->has_processed_input, lp->input_lock);
        }

        while (!is_finished) {
            ch = getchar();

            *(lp->input_buffer + index) = ch;
            *(lp->input_count) += 1;

            check_is_end_sequence(lp, index, ch);

            if (ch == '\n' || *(lp->is_end_sequence)) {
                is_finished = 1;
            }

            index += 1;
        }

        pthread_cond_signal(lp->has_input);
        pthread_mutex_unlock(lp->input_lock);
    }

    *(lp->input_t_term) = 1;
    
    return NULL;
}

/*
 * Read characters from the input buffer into the process buffer.
 *
 * While reading the input, hold the input lock and the process lock.
 * 
 * If a newline character is encountered, convert the newline to a space. If the
 * character is printable save it to the process buffer and increment the
 * process index and process count.
 * 
 * After all characters are read from the input buffer, reset the input count to
 * zero. Then, signal that line separator processing is finished, and release 
 * the process lock. This will allow the next thread to process the partially
 * cleansed data. Also signal that input was processed, and release the input
 * lock. This, along with resetting the input count, will allow the input thread
 * to get additional input. Finally, wait until there is more input or until the
 * end sequence is reached.
 */
void *convert_line_seperator(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running) || *(lp->input_count) > 0 || !*(lp->input_t_term)) {
        char ch = '\0';

        pthread_mutex_lock(lp->input_lock);

        while (*(lp->input_count) == 0 && !*(lp->is_end_sequence)) {
            pthread_cond_wait(lp->has_input, lp->input_lock);
        }

        pthread_mutex_lock(lp->process_lock);

        for (int i = 0; i < *(lp->input_count); i++) {
            ch = *(lp->input_buffer + i);

            if (ch == '\n') {
                ch = ' ';
            }
            
            if (ch >= 32 && ch <= 126) {
                *(lp->process_buffer + *(lp->process_index)) = ch;
                *(lp->process_index) += 1;
                *(lp->process_count) += 1;
            }
        }

        *(lp->input_count) = 0;

        pthread_cond_signal(lp->has_processed_line_separator);
        pthread_mutex_unlock(lp->process_lock);

        pthread_cond_signal(lp->has_processed_input);
        pthread_mutex_unlock(lp->input_lock);
    }

    *(lp->line_separator_t_term) = 1;

    return NULL;
}

/*
 * Read characters from the process buffer into the output buffer.
 *
 * While reading the process buffer, hold the process lock and the output lock.
 * 
 * If a double plus sign is encountered, convert the first plus sign to a caret
 * symbol, then skip the second plus sign. Insert characters into the output
 * buffer, then increment the output index and output count.
 * 
 * Once the processing is complete, the data is fully cleansed, and is ready for
 * output.
 * 
 * Reset the process count. Then, signal that output is ready, and release the
 * output lock. This allows the output thread to consume the data. Also release
 * the process lock to allow the previous thread to gain access to the process
 * buffer. Finally, wait for more data to process or until the end sequence is
 * encountered.
 */
void *convert_plus_sign(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running) || *(lp->process_count) > 0 || !*(lp->line_separator_t_term)) {
        pthread_mutex_lock(lp->process_lock);

        while (*(lp->process_count) == 0 && !*(lp->is_end_sequence)) {
            pthread_cond_wait(lp->has_processed_line_separator, lp->process_lock);
        }

        pthread_mutex_lock(lp->output_lock);

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

        *(lp->process_count) = 0;

        pthread_cond_signal(lp->has_output);
        pthread_mutex_unlock(lp->output_lock);

        pthread_mutex_unlock(lp->process_lock);
    }

    *(lp->plus_sign_t_term) = 1;

    return NULL;
}

/*
 * Write characters from the output buffer to stdout.
 *
 * While reading from the output buffer, hold the output lock.
 * 
 * If the output count is less than the specified line size, wait until more
 * output is available or until the end sequence is encountered.
 * 
 * Otherwise, if the output count is greater than or equal to the specified line
 * size, continue outputting lines and decrementing the count until the count is
 * less than the specified line size.
 * 
 * Then, signal that output has finished, and release the output lock. This lets
 * the previous thread access the output buffer to load it with more data.
 * Finally, wait for more output or until the end sequence is encountered.
 */
void *print_output(void *arg) {
    struct LineProcessor *lp = (struct LineProcessor *) arg;

    while (*(lp->is_running) || *(lp->output_count) >= *(lp->output_size) || !*(lp->plus_sign_t_term)) {
        pthread_mutex_lock(lp->output_lock);

        while ((*(lp->output_count) < *(lp->output_size)) && !*(lp->is_end_sequence)) {
            pthread_cond_wait(lp->has_output, lp->output_lock);
        }

        while (*(lp->output_count) >= *(lp->output_size)) {
            for (int i = 0; i < *(lp->output_size); i++) {
                putchar(*(lp->output_buffer + *(lp->current_index) + i));
                fflush(stdout);
            }

            *(lp->current_index) += *(lp->output_size);
            *(lp->output_count) -= *(lp->output_size);

            printf("\n");
            fflush(stdout);
        }

        pthread_cond_signal(lp->has_output);
        pthread_mutex_unlock(lp->output_lock);
    }

    *(lp->output_t_term) = 1;

    return NULL;
}

/*
 * Free the line processor struct.
 */
void free_line_processor(struct LineProcessor *lp) {
    free(lp->is_running);
    free(lp->is_end_sequence_char);
    free(lp->is_end_sequence);
    free(lp->process_index);
    free(lp->output_index);
    free(lp->current_index);
    free(lp->input_count);
    free(lp->process_count);
    free(lp->output_count);
    free(lp->input_buffer_size);
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