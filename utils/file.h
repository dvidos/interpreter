
// typedef struct file file;

// file *new_file(const char *path);
// const char *file_read_all(file *f);
// failable_list file_read_lines(file *f);
// failable file_write_all(file *f, const char contents);
// failable file_write_lines(file *f, list *lines);
// failable file_delete(file *f);


failable_constcharptr file_read(const char *filepath);

// we could try to make a very crude database, to work on ata intensive things
// for example optical recognition...
// with a fixed record size maybe...

// dict *json_to_variants_dict(const char *json_text);
// const char *variants_dict_to_json(dict *json_object);

// dict *yaml_to_variants_dict(const char *yaml_text);
// const char *variants_dict_to_yaml(dict *yaml_object);
