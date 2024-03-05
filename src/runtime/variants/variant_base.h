#ifndef _BASE_VARIANT_H
#define _BASE_VARIANT_H

// forward declarations
typedef struct variant variant;
typedef struct variant_type variant_type;


// all "variant" structures (including the variant_type) 
// must have these as the first items, to allow uniform treatment
#define BASE_VARIANT_FIRST_ATTRIBUTES        \
            struct variant_type *_type;  \
            int _references_count


// for statically allocated variants, we don't need to count references
// so use this define for the `_references_count` attribute
#define VARIANT_STATICALLY_ALLOCATED   (-222) // really random number


// all variants can be cast to this pointer
// essentially, they can all pretend to be this structure
// check the "type" attribute, to find which "subclass" they are
// all variants are children of (encapsulate) this

struct variant { 
    BASE_VARIANT_FIRST_ATTRIBUTES;
};


#endif
