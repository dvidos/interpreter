#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "listing.h"

struct listing {
    int lines_count;
    const char *code_tokenized;
    const char **line_ptr_array;
};

listing *new_listing(const char *code) {
    listing *l = malloc(sizeof(listing));

    // make a first pass, count lines
    int n = 0;
    char *p = (char *)code;
    while (*p) {
        if (*p == '\n') n++;
        p++;
    }
    
    // create the lines array
    l->lines_count = n;
    l->code_tokenized = malloc(strlen(code) + 1);
    strcpy((char *)l->code_tokenized, code);
    l->line_ptr_array = malloc(sizeof(char *) * (n + 1));
    l->line_ptr_array[0] = NULL;

    // make second pass, setting pointers and null terminators
    bool start_of_line = true;
    n = 1;
    p = (char *)l->code_tokenized;
    while (*p) {
        if (start_of_line) {
            l->line_ptr_array[n++] = p;
            start_of_line = false;
        }
        if (*p == '\n') {
            *p = '\0';
            start_of_line = true;
        }
        p++;
    }

    return l;
}

int listing_lines_count(listing *l) {
    return l->lines_count;
}

const char *listing_get_line(listing *l, int line_no) {
    // line_no is one based, here
    if (line_no <= 0 || line_no > l->lines_count)
        return NULL;
    
    return l->line_ptr_array[line_no];
}

void listing_free(listing *l) {
    free((void *)l->code_tokenized);
    free((void *)l->line_ptr_array);
    free(l);
}
