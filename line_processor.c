#include "line_processor.h"

/*
 *
 */
void run_line_processor(void) {
    struct LineProcessor *lp = malloc(sizeof(struct LineProcessor));
    
    init_line_processor(lp);

    while (*(lp->is_running)) {
        read_input(lp);
        convert_line_seperator(lp);
        convert_plus_sign(lp);
        print_output(lp);
    }

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
void read_input(struct LineProcessor *lp) {
    int is_finished = 0;
    int index = 0;
    char ch = '\0';

    while (!is_finished) {
        ch = getchar();
        *(lp->input_buffer + index) = ch;

        if (ch == '\n' || ch == EOF) {
            is_finished = 1;
            if (ch == EOF) {
                *(lp->is_running) = 0;
            }
        }

        *(lp->input_count) += 1;
        index += 1;
    }
}

/*
 *
 */
void convert_line_seperator(struct LineProcessor *lp) {
    char ch = '\0';

    for (int i = 0; i < *(lp->input_count); i++) {
        ch = *(lp->input_buffer + i);

        if (ch == '\n') {
            ch = ' ';
        }
        
        if (ch != '\r' && ch != EOF) {
            *(lp->process_buffer + *(lp->process_index)) = ch;
            *(lp->process_index) += 1;
        }
    }

    *(lp->process_count) = *(lp->input_count);
    *(lp->input_count) = 0;
}

/*
 *
 */
void convert_plus_sign(struct LineProcessor *lp) {
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
}

/*
 *
 */
void print_output(struct LineProcessor *lp) {
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
    free(lp->input_buffer);
    free(lp->process_buffer);
    free(lp->output_buffer);
    free(lp);
}