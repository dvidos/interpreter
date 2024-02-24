typedef struct dict_object {
    BASE_OBJECT_FIRST_ATTRIBUTES;
    void **entries_arr;
    int capacity;
    int length;
} dict_object;

