//
// typemaps for Ruby
//
// $Header$
//
// Copyright (C) 2000  Network Applied Communication Laboratory, Inc.
// Copyright (C) 2000  Information-technology Promotion Agency, Japan
//
// Masaki Fukushima
//

/*
The SWIG typemap library provides a language independent mechanism for
supporting output arguments, input values, and other C function
calling mechanisms.  The primary use of the library is to provide a
better interface to certain C function--especially those involving
pointers.
*/

// ------------------------------------------------------------------------
// Pointer handling
//
// These mappings provide support for input/output arguments and common
// uses for C/C++ pointers.
// ------------------------------------------------------------------------

// INPUT typemaps.
// These remap a C pointer to be an "INPUT" value which is passed by value
// instead of reference.

/*
The following methods can be applied to turn a pointer into a simple
"input" value.  That is, instead of passing a pointer to an object,
you would use a real value instead.

         int            *INPUT
         short          *INPUT
         long           *INPUT
         unsigned int   *INPUT
         unsigned short *INPUT
         unsigned long  *INPUT
         unsigned char  *INPUT
	 bool           *INPUT
         float          *INPUT
         double         *INPUT
         
To use these, suppose you had a C function like this :

        double fadd(double *a, double *b) {
               return *a+*b;
        }

You could wrap it with SWIG as follows :
        
        %include typemaps.i
        double fadd(double *INPUT, double *INPUT);

or you can use the %apply directive :

        %include typemaps.i
        %apply double *INPUT { double *a, double *b };
        double fadd(double *a, double *b);

*/

%define INPUT_TYPEMAP(type, converter)
%typemap(in) type *INPUT(type temp), type &INPUT(type temp)
{
    temp = (type) converter($input);
    $1 = &temp;
}
%typemap(typecheck) type *INPUT = type;
%typemap(typecheck) type &INPUT = type;
%enddef

INPUT_TYPEMAP(float, NUM2DBL);
INPUT_TYPEMAP(double, NUM2DBL);
INPUT_TYPEMAP(int, NUM2INT);
INPUT_TYPEMAP(short, NUM2SHRT);
INPUT_TYPEMAP(long, NUM2LONG);
INPUT_TYPEMAP(unsigned int, NUM2UINT);
INPUT_TYPEMAP(unsigned short, NUM2USHRT);
INPUT_TYPEMAP(unsigned long, NUM2ULONG);
INPUT_TYPEMAP(unsigned char, NUM2UINT);
INPUT_TYPEMAP(signed char, NUM2INT);
INPUT_TYPEMAP(bool, RTEST);

#undef INPUT_TYPEMAP

// OUTPUT typemaps.   These typemaps are used for parameters that
// are output only.   The output value is appended to the result as
// a array element.

/*
The following methods can be applied to turn a pointer into an "output"
value.  When calling a function, no input value would be given for
a parameter, but an output value would be returned.  In the case of
multiple output values, they are returned in the form of a Ruby Array.

         int            *OUTPUT
         short          *OUTPUT
         long           *OUTPUT
         unsigned int   *OUTPUT
         unsigned short *OUTPUT
         unsigned long  *OUTPUT
         unsigned char  *OUTPUT
	 bool           *OUTPUT
         float          *OUTPUT
         double         *OUTPUT
         
For example, suppose you were trying to wrap the modf() function in the
C math library which splits x into integral and fractional parts (and
returns the integer part in one of its parameters).K:

        double modf(double x, double *ip);

You could wrap it with SWIG as follows :

        %include typemaps.i
        double modf(double x, double *OUTPUT);

or you can use the %apply directive :

        %include typemaps.i
        %apply double *OUTPUT { double *ip };
        double modf(double x, double *ip);

The Ruby output of the function would be a Array containing both
output values. 
*/

%include "fragments.i"
		
%define OUTPUT_TYPEMAP(type, converter, convtype)
%typemap(in,numinputs=0) type *OUTPUT(type temp), type &OUTPUT(type temp) "$1 = &temp;";
%typemap(argout, fragment="output_helper") type *OUTPUT, type &OUTPUT {
   VALUE o = converter(convtype (*$1));
   $result = output_helper($result, o);
}
%enddef

OUTPUT_TYPEMAP(int, INT2NUM, (int));
OUTPUT_TYPEMAP(short, INT2NUM, (int));
OUTPUT_TYPEMAP(long, INT2NUM, (int));
OUTPUT_TYPEMAP(unsigned int, UINT2NUM, (unsigned int));
OUTPUT_TYPEMAP(unsigned short, UINT2NUM, (unsigned int));
OUTPUT_TYPEMAP(unsigned long, UINT2NUM, (unsigned int));
OUTPUT_TYPEMAP(unsigned char, UINT2NUM, (unsigned int));
OUTPUT_TYPEMAP(signed char, INT2NUM, (int));
OUTPUT_TYPEMAP(float, rb_float_new, (double));
OUTPUT_TYPEMAP(double, rb_float_new, (double));

#undef OUTPUT_TYPEMAP

%typemap(in,numinputs=0) bool *OUTPUT(bool temp), bool &OUTPUT(bool temp) "$1 = &temp;";
%typemap(argout, fragment="output_helper") bool *OUTPUT, bool &OUTPUT {
    VALUE o = (*$1) ? Qtrue : Qfalse;
    $result = output_helper($result, o);
}

// INOUT
// Mappings for an argument that is both an input and output
// parameter

/*
The following methods can be applied to make a function parameter both
an input and output value.  This combines the behavior of both the
"INPUT" and "OUTPUT" methods described earlier.  Output values are
returned in the form of a Ruby array.

         int            *INOUT
         short          *INOUT
         long           *INOUT
         unsigned int   *INOUT
         unsigned short *INOUT
         unsigned long  *INOUT
         unsigned char  *INOUT
	 bool           *INOUT
         float          *INOUT
         double         *INOUT
         
For example, suppose you were trying to wrap the following function :

        void neg(double *x) {
             *x = -(*x);
        }

You could wrap it with SWIG as follows :

        %include typemaps.i
        void neg(double *INOUT);

or you can use the %apply directive :

        %include typemaps.i
        %apply double *INOUT { double *x };
        void neg(double *x);

Unlike C, this mapping does not directly modify the input value (since
this makes no sense in Ruby).  Rather, the modified input value shows
up as the return value of the function.  Thus, to apply this function
to a Ruby variable you might do this :

       x = neg(x)

Note : previous versions of SWIG used the symbol 'BOTH' to mark
input/output arguments.   This is still supported, but will be slowly
phased out in future releases.

*/

%typemap(in) int *INOUT = int *INPUT;
%typemap(in) short *INOUT = short *INPUT;
%typemap(in) long *INOUT = long *INPUT;
%typemap(in) unsigned *INOUT = unsigned *INPUT;
%typemap(in) unsigned short *INOUT = unsigned short *INPUT;
%typemap(in) unsigned long *INOUT = unsigned long *INPUT;
%typemap(in) unsigned char *INOUT = unsigned char *INPUT;
%typemap(in) signed char *INOUT = signed char *INPUT;
%typemap(in) bool *INOUT = bool *INPUT;
%typemap(in) float *INOUT = float *INPUT;
%typemap(in) double *INOUT = double *INPUT;

%typemap(in) int &INOUT = int &INPUT;
%typemap(in) short &INOUT = short &INPUT;
%typemap(in) long &INOUT = long &INPUT;
%typemap(in) unsigned &INOUT = unsigned &INPUT;
%typemap(in) unsigned short &INOUT = unsigned short &INPUT;
%typemap(in) unsigned long &INOUT = unsigned long &INPUT;
%typemap(in) unsigned char &INOUT = unsigned char &INPUT;
%typemap(in) signed char &INOUT = signed char &INPUT;
%typemap(in) bool &INOUT = bool &INPUT;
%typemap(in) float &INOUT = float &INPUT;
%typemap(in) double &INOUT = double &INPUT;

%typemap(argout) int *INOUT = int *OUTPUT;
%typemap(argout) short *INOUT = short *OUTPUT;
%typemap(argout) long *INOUT = long *OUTPUT;
%typemap(argout) unsigned *INOUT = unsigned *OUTPUT;
%typemap(argout) unsigned short *INOUT = unsigned short *OUTPUT;
%typemap(argout) unsigned long *INOUT = unsigned long *OUTPUT;
%typemap(argout) unsigned char *INOUT = unsigned char *OUTPUT;
%typemap(argout) signed char *INOUT = signed char *OUTPUT;
%typemap(argout) bool *INOUT = bool *OUTPUT;
%typemap(argout) float *INOUT = float *OUTPUT;
%typemap(argout) double *INOUT = double *OUTPUT;

%typemap(argout) int &INOUT = int &OUTPUT;
%typemap(argout) short &INOUT = short &OUTPUT;
%typemap(argout) long &INOUT = long &OUTPUT;
%typemap(argout) unsigned &INOUT = unsigned &OUTPUT;
%typemap(argout) unsigned short &INOUT = unsigned short &OUTPUT;
%typemap(argout) unsigned long &INOUT = unsigned long &OUTPUT;
%typemap(argout) unsigned char &INOUT = unsigned char &OUTPUT;
%typemap(argout) signed char &INOUT = signed char &OUTPUT;
%typemap(argout) bool &INOUT = bool &OUTPUT;
%typemap(argout) float &INOUT = float &OUTPUT;
%typemap(argout) double &INOUT = double &OUTPUT;

// --------------------------------------------------------------------
// Special types
// --------------------------------------------------------------------

/*
The typemaps.i library also provides the following mappings :

struct timeval *
time_t

    Ruby has builtin class Time.  INPUT/OUTPUT typemap for timeval and
    time_t is provided.

int PROG_ARGC
char **PROG_ARGV

    Some C function receive argc and argv from C main function.
    This typemap provides ignore typemap which pass Ruby ARGV contents
    as argc and argv to C function.
*/


// struct timeval *
%{
#ifdef __cplusplus
extern "C" {
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
struct timeval rb_time_timeval(VALUE);
#endif
#ifdef __cplusplus
}
#endif
%}

%typemap(in) struct timeval *INPUT (struct timeval temp)
{
    if (NIL_P($input))
	$1 = NULL;
    else {
	temp = rb_time_timeval($input);
	$1 = &temp;
    }
}

%typemap(in,numinputs=0) struct timeval *OUTPUT(struct timeval temp)
{
    $1 = &temp;
}

%typemap(argout) struct timeval *OUTPUT
{
    $result = rb_time_new($1->tv_sec, $1->tv_usec);
}

%typemap(out) struct timeval *
{
    $result = rb_time_new($1->tv_sec, $1->tv_usec);
}

%typemap(out) struct timespec *
{
    $result = rb_time_new($1->tv_sec, $1->tv_nsec / 1000);
}

// time_t
%typemap(in) time_t
{
    if (NIL_P($input))
	$1 = (time_t)-1;
    else
	$1 = NUM2LONG(rb_funcall($input, rb_intern("tv_sec"), 0));
}

%typemap(out) time_t
{
    $result = rb_time_new($1, 0);
}

// argc and argv
%typemap(in,numinputs=0) int PROG_ARGC {
    $1 = RARRAY(rb_argv)->len + 1;
}

%typemap(in,numinputs=0) char **PROG_ARGV {
    int i, n;
    VALUE ary = rb_eval_string("[$0] + ARGV");
    n = RARRAY(ary)->len;
    $1 = (char **)malloc(n + 1);
    for (i = 0; i < n; i++) {
	VALUE v = rb_obj_as_string(RARRAY(ary)->ptr[i]);
	$1[i] = (char *)malloc(RSTRING(v)->len + 1);
	strcpy($1[i], RSTRING(v)->ptr);
    }
}

%typemap(freearg) char **PROG_ARGV {
    int i, n = RARRAY(rb_argv)->len + 1;
    for (i = 0; i < n; i++) free($1[i]);
    free($1);
}

// FILE *
%{
#ifdef __cplusplus
extern "C" {
#endif
#include "rubyio.h"
#ifdef __cplusplus
}
#endif
%}

%typemap(in) FILE *READ {
    OpenFile *of;
    GetOpenFile($input, of);
    rb_io_check_readable(of);
    $1 = GetReadFile(of);
    rb_read_check($1);
}

%typemap(in) FILE *READ_NOCHECK {
    OpenFile *of;
    GetOpenFile($input, of);
    rb_io_check_readable(of);
    $1 = GetReadFile(of);
}

%typemap(in) FILE *WRITE {
    OpenFile *of;
    GetOpenFile($input, of);
    rb_io_check_writable(of);
    $1 = GetWriteFile(of);
}

/* Overloading information */

%typemap(typecheck) double *INOUT = double;
%typemap(typecheck) signed char *INOUT = signed char;
%typemap(typecheck) unsigned char *INOUT = unsigned char;
%typemap(typecheck) unsigned long *INOUT = unsigned long;
%typemap(typecheck) unsigned short *INOUT = unsigned short;
%typemap(typecheck) unsigned int *INOUT = unsigned int;
%typemap(typecheck) long *INOUT = long;
%typemap(typecheck) short *INOUT = short;
%typemap(typecheck) int *INOUT = int;
%typemap(typecheck) float *INOUT = float;

%typemap(typecheck) double &INOUT = double;
%typemap(typecheck) signed char &INOUT = signed char;
%typemap(typecheck) unsigned char &INOUT = unsigned char;
%typemap(typecheck) unsigned long &INOUT = unsigned long;
%typemap(typecheck) unsigned short &INOUT = unsigned short;
%typemap(typecheck) unsigned int &INOUT = unsigned int;
%typemap(typecheck) long &INOUT = long;
%typemap(typecheck) short &INOUT = short;
%typemap(typecheck) int &INOUT = int;
%typemap(typecheck) float &INOUT = float;
