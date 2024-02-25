#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../discrete/person.h"
#include "../discrete/student.h"



void act_on_person(person *person) {
    
    // it could be a person, or a student
    assert(is_or_encloses_struct(person, person_class));
    person->_vt->dream(person);

    str_builder *sb = new_str_builder();
    instance_describe(person, sb);
    printf("person is '%s'\n", str_builder_charptr(sb));
    str_builder_free(sb);
}

void act_on_student(student *student) {

    assert(is_or_encloses_struct(student, student_class));
    student->_vt->take_test(student, 90);
    student->_vt->dream(student);
    student->_class->_class_info_magic_number;

    str_builder *sb = new_str_builder();
    instance_describe(student, sb);
    printf("student is '%s'\n", str_builder_charptr(sb));
    str_builder_free(sb);
}

void act_on_unknown(void *instance) {
    str_builder *sb = new_str_builder();
    instance_describe(instance, sb);
    unsigned hash = instance_hash(instance);
    printf("acting on instance of class '%s', description is '%s', hash is 0x%x\n", 
        instance_struct_name(instance),
        str_builder_charptr(sb),
        hash
    );
    str_builder_free(sb);
}

void run_objects1_tests() {
    person *bob = new_person("bob the builder");
    act_on_person(bob);

    student *maria = new_student("maria");
    act_on_person((person *)maria);
    act_on_student(maria);

    act_on_unknown(bob);
    act_on_unknown(maria);
}
