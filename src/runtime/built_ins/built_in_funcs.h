#ifndef _BUILT_IN_FUNCS_H
#define _BUILT_IN_FUNCS_H

#include "../../containers/_module.h"

void initialize_built_in_funcs_table();
dict *get_built_in_funcs_table();

dict *get_built_in_str_methods_dictionary();
dict *get_built_in_list_methods_dictionary();
dict *get_built_in_dict_methods_dictionary();


#endif
