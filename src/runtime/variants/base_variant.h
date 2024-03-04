#ifndef _BASE_VARIANT_H
#define _BASE_VARIANT_H


// all "variant" structures (including the variant_type) 
// must have these as the first items, to allow uniform treatment
#define BASE_VARIANT_FIRST_ATTRIBUTES        \
            struct variant_type *_type;  \
            int _references_count


// for statically allocated variants, we don't need to count references
// so use this define for the `_references_count` attribute
#define VARIANT_STATICALLY_ALLOCATED   (-2)


// all variants can be cast to this pointer
// essentially, they can all pretend to be this structure
// check the "type" attribute, to find which "subclass" they are
struct variant { // all variants children of this
    BASE_VARIANT_FIRST_ATTRIBUTES;
};


#endif
