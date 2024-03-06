#include "hash.h"

unsigned int simple_hash(void *data, int size) {
    unsigned result = 5381;
    unsigned char *ptr = data;
    while (size-- > 0) {
        // result = (result * 33) + char
        result = ((result << 5) + result) + *ptr++;
    }
    return result;
}

