/* inout_typemaps.i

   Support for INPUT, OUTPUT, and INOUT typemaps. OUTPUT variables are returned
   as multiple values.

*/


%define INOUT_TYPEMAP(type_, OUTresult_, INbind_)
// OUTPUT map.
%typemap(lin,numinputs=0) type_ *OUTPUT, type_ &OUTPUT
%{(let (($out (ff:allocate-fobject '$*in_fftype :c)))
     $body
     OUTresult_
     (ff:free-fobject $out)) %}

// INPUT map.
%typemap(in) type_ *INPUT, type_ &INPUT
%{ $1 = &$input; %}

%typemap(ctype) type_ *INPUT, type_ &INPUT "$*1_ltype";


// INOUT map.
%typemap(lin,numinputs=1) type_ *INOUT, type_ &INOUT
%{(let (($out (ff:allocate-fobject '$*in_fftype :c)))
     INbind_
     $body
     OUTresult_
     (ff:free-fobject $out)) %}

%enddef

// $in, $out, $lclass,
// $in_fftype, $*in_fftype

INOUT_TYPEMAP(int,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(short,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(long,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(unsigned int,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(unsigned short,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(unsigned long,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(char,
	      (cl::push (code-char (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out))
		    ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(float,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(double,
	      (cl::push (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) $in));
INOUT_TYPEMAP(bool,
	      (cl::push (not (zerop (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out)))
		    ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out) (if $in 1 0)));

INOUT_TYPEMAP(char *,
              (cl::push (ff:char*-to-string (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out))
		    ACL_result),
	      (cl::setf (ff:fslot-value-typed (cl::quote $*in_fftype) :c $out)
		    (ff:string-to-char* $in)))

%typemap(lisptype) bool *INPUT, bool &INPUT "boolean";

// long long support not yet complete
// INOUT_TYPEMAP(long long);
// INOUT_TYPEMAP(unsigned long long);
