/* SWIG Configuration File for Ocaml. -*-c-*-
   Modified from mzscheme.i
   This file is parsed by SWIG before reading any other interface
   file. */

/* Insert common stuff */
%insert(runtime) "swigrun.swg"
%insert(runtime) "common.swg"

/* Include headers */
%insert(runtime) "ocamldec.swg"

/* Type registration */
%insert(init) "typeregister.swg"

%insert(mlitail) %{
  val swig_val : c_enum_type -> c_obj -> Swig.c_obj
%}

%insert(mltail) %{
  let rec swig_val t v = 
    match v with
        C_enum e -> enum_to_int t v
      | C_list l -> Swig.C_list (List.map (swig_val t) l)
      | C_array a -> Swig.C_array (Array.map (swig_val t) a)
      | _ -> Obj.magic v
%}

/*#ifndef SWIG_NOINCLUDE*/
%insert(runtime) "ocaml.swg"
/*#endif*/

%insert(classtemplate) "class.swg"

/* Definitions */
#define SWIG_malloc(size) swig_malloc(size, FUNC_NAME)
#define SWIG_free(mem) free(mem)

/* Read in standard typemaps. */
%include "swig.swg"
%include "typemaps.i"
%include "typecheck.i"
%include "exception.i"
%include "preamble.swg"
