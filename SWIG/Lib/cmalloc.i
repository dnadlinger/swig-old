/* -----------------------------------------------------------------------------
 * cmalloc.i
 *
 * Author(s):  David Beazley (beazley@cs.uchicago.edu)
 *
 * This library file contains macros that can be used to create objects using
 * the C malloc function.
 *
 * $Header$
 * ----------------------------------------------------------------------------- */

%{
#include <stdlib.h>
%}

/* %malloc(TYPE [, NAME = TYPE])
   %calloc(TYPE [, NAME = TYPE])
   %realloc(TYPE [, NAME = TYPE])
   %free(TYPE [, NAME = TYPE])
   %allocators(TYPE [,NAME = TYPE])

   Creates functions for allocating/reallocating memory.

   TYPE *malloc_NAME(int nbytes = sizeof(TYPE);
   TYPE *calloc_NAME(int nobj=1, int size=sizeof(TYPE));
   TYPE *realloc_NAME(TYPE *ptr, int nbytes);
   void free_NAME(TYPE *ptr);

*/

%define %malloc(TYPE,...)
#if #__VA_ARGS__ != ""
%name(malloc_##__VA_ARGS__)
#else
%name(malloc_##TYPE)
#endif
TYPE *malloc(int nbytes
#if #TYPE != "void"
= sizeof(TYPE)
#endif
);
%enddef

%define %calloc(TYPE,...)
#if #__VA_ARGS__ != ""
%name(calloc_##__VA_ARGS__)
#else
%name(calloc_##TYPE)
#endif
TYPE *calloc(int nobj = 1, int sz = 
#if #TYPE != "void"
sizeof(TYPE)
#else
1
#endif
);
%enddef

%define %realloc(TYPE,...)
%insert("header") {
#if #__VA_ARGS__ != ""
TYPE *realloc_##__VA_ARGS__(TYPE *ptr, int nitems)
#else
TYPE *realloc_##TYPE(TYPE *ptr, int nitems)
#endif
{
#if #TYPE != "void"
return (TYPE *) realloc(ptr, nitems*sizeof(TYPE));
#else
return (TYPE *) realloc(ptr, nitems);
#endif
}
}
#if #__VA_ARGS__ != ""
TYPE *realloc_##__VA_ARGS__(TYPE *ptr, int nitems);
#else
TYPE *realloc_##TYPE(TYPE *ptr, int nitems);
#endif
%enddef

%define %free(TYPE,...)
#if #__VA_ARGS__ != ""
%name(free_##__VA_ARGS__) void free(TYPE *ptr);
#else
%name(free_##TYPE)        void free(TYPE *ptr);
#endif
%enddef

%define %sizeof(TYPE,...)
#if #__VA_ARGS__ != ""
%constant int sizeof_##__VA_ARGS__ = sizeof(TYPE);
#else
%constant int sizeof_##TYPE = sizeof(TYPE);
#endif
%enddef

%define %allocators(TYPE,...)
%malloc(TYPE,__VA_ARGS__)
%calloc(TYPE,__VA_ARGS__)
%realloc(TYPE,__VA_ARGS__)
%free(TYPE,__VA_ARGS__)
#if #TYPE != "void"
%sizeof(TYPE,__VA_ARGS__)
#endif
%enddef





