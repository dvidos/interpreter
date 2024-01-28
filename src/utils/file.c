#include <stdio.h>
#include <stdlib.h>
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

// we could try to make a very crude database, to work on ata intensive things
// for example optical recognition...
// with a fixed record size maybe...

// dict *json_to_variants_dict(const char *json_text);
// const char *variants_dict_to_json(dict *json_object);

// dict *yaml_to_variants_dict(const char *yaml_text);
// const char *variants_dict_to_yaml(dict *yaml_object);
