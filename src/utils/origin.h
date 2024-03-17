#ifndef _ORIGIN_H
#define _ORIGIN_H


typedef struct origin {
    const char *filename;
    int line_no;
    int column_no;
} origin;


origin *new_origin(const char *filename, int line_no, int column_no);



#endif
