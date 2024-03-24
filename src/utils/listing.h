#ifndef _LISTING_H
#define _LISTING_H

typedef struct listing listing;

listing *new_listing(const char *code);
int listing_lines_count(listing *l);
const char *listing_get_line(listing *l, int line_no);
void listing_free(listing *l);


#endif
