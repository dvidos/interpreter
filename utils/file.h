

// this should be good to get lots of information in memory

const char *file_read_text(const char *path);
list *      file_read_lines(const char *path);
void        file_write_text(const char *path, const char *text);
void        file_write_lines(const char *path, list *lines);

// we could try to make a very crude database, to work on ata intensive things
// for example optical recognition...
// with a fixed record size maybe...

dict *json_to_variants_dict(const char *json_text);
const char *variants_dict_to_json(dict *json_object);

dict *yaml_to_variants_dict(const char *yaml_text);
const char *variants_dict_to_yaml(dict *yaml_object);
