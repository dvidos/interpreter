#ifndef _FILE_H
#define _FILE_H

#include "failable.h"

failable_const_char file_read(const char *filepath);

char **get_files(const char *dirpath);
char **get_dirs(const char *dirpath);
void free_files(char **files);
void free_dirs(char **dirs);

char *find_extension(const char *file);

#endif
