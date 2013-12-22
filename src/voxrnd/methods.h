/**
   @file methods.h
   @brief (Vain) attempt to implement classes and methods

   Implements runtime method dispatching based on statically defined
   dispatch tables (which means that tables, methods and classes
   cannot be defined at runtime.
**/
#ifndef __METHODS__
#define __METHODS__

/**
   \brief Generic class structure
**/
typedef struct
{
    int obj_type; /**< \brief All classes must have this field */
} class_t;

/**
   \brief Generates a setter name for a slot
**/
#define SETTER_NAME(slot) vox_set_##slot
/**
   \brief Generates getter name for a slot
**/
#define GETTER_NAME(slot) vox_get_##slot

/**
   \brief Generates a name of an implementation of setter
**/
#define SETTER_IMPL_NAME(class, slot) class##_set_##slot
/**
   \brief Generates a name of an implementation of getter
**/
#define GETTER_IMPL_NAME(class, slot) class##_get_##slot

/**
   \brief Generates a simple setter for a slot with specified type
**/
#define DEF_SIMPLE_SETTER_IMPL(class, slot, type) type SETTER_IMPL_NAME(class, slot) (class_t *obj, type val) \
    {class *obj2 = (class*)obj; obj2->slot = val; return val;}
/**
   \brief Generates a simple getter for a slot with specified type
**/
#define DEF_SIMPLE_GETTER_IMPL(class, slot, type) type GETTER_IMPL_NAME(class, slot) (const class_t *obj) \
    {class *obj2 = (class*)obj; return obj2->slot;}

#define SETTER_PROTO(slot, type) type SETTER_NAME(slot) (class_t*, type);
#define GETTER_PROTO(slot, type) type GETTER_NAME(slot) (const class_t*);

#define SETTER_DISPATCH(slot, type) type (*SETTER_NAME(slot)) (class_t*, type);
#define GETTER_DISPATCH(slot, type) type (*GETTER_NAME(slot)) (const class_t*);

#define DEF_SETTER(table, slot, type) type SETTER_NAME(slot) (class_t *obj, type val) \
    {return table[obj->obj_type].SETTER_NAME(slot) (obj, val);}
#define DEF_GETTER(table, slot, type) type GETTER_NAME(slot) (const class_t *obj) \
    {return table[obj->obj_type].GETTER_NAME(slot) (obj);}

#endif
