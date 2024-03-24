#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "failable.h"

failable_const_char file_read(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (f == NULL) return failed_const_char(NULL, "Could not open file %s", filepath);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    char *buffer = malloc(size + 1);
    fseek(f, 0, SEEK_SET);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';

    fclose(f);
    return ok_const_char(buffer);
}

static char **get_entries(const char *dirpath, int mask) {
    DIR *d = opendir(dirpath);
    if (d == NULL)
        return NULL;
    
    int count = 0;
    struct dirent *e;
    char **names;

    while ((e = readdir(d)) != NULL) {
        if ((e->d_type & mask) == 0)
            continue;
        count++;
    }

    rewinddir(d);
    names = malloc(sizeof(char *) * (count + 1));
    count = 0;

    while ((e = readdir(d)) != NULL) {
        if ((e->d_type & mask) == 0)
            continue;
        
        names[count] = malloc(strlen(dirpath) + 1 + strlen(e->d_name) + 1);
        strcpy(names[count], dirpath);
        strcat(names[count], "/");
        strcat(names[count], e->d_name);
        count++;
    }
    closedir(d);
    names[count] = NULL;

    return names;
}

static void free_entries_array(char **arr) {
    for (int i = 0; arr[i] != NULL; i++)
        free(arr[i]);
    free(arr);
}

char **get_files(const char *dirpath) {
    return get_entries(dirpath, DT_REG);
}

char **get_dirs(const char *dirpath) {
    return get_entries(dirpath, DT_DIR);
}

void free_files(char **files) {
    free_entries_array(files);
}

void free_dirs(char **dirs) {
    free_entries_array(dirs);
}

char *find_extension(const char *file) {
    char *p = strrchr(file, '.');
    return p == NULL ? NULL : p + 1;
}