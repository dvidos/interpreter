typedef struct dict_object {
    FIRST_OBJECT_ATTRIBUTES;
    void **entries_arr;
    int capacity;
    int length;
} dict_object;

