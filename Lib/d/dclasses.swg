/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dclasses.swg
 *
 * Typemaps for non-primitive types (C/C++ classes and structs).
 * ----------------------------------------------------------------------------- */

%typemap(ctype) SWIGTYPE "void *"
%typemap(imtype) SWIGTYPE "void*"
%typemap(cstype) SWIGTYPE "$&dclassname"

%typemap(ctype) SWIGTYPE [] "void *"
%typemap(imtype) SWIGTYPE [] "void*"
%typemap(cstype) SWIGTYPE [] "$dclassname"

%typemap(ctype) SWIGTYPE * "void *"
%typemap(imtype) SWIGTYPE * "void*"
%typemap(cstype) SWIGTYPE * "$dclassname"

%typemap(ctype) SWIGTYPE & "void *"
%typemap(imtype) SWIGTYPE & "void*"
%typemap(cstype) SWIGTYPE & "$dclassname"

%typemap(ctype) SWIGTYPE *& "void *"
%typemap(imtype) SWIGTYPE *& "void*"
%typemap(cstype) SWIGTYPE *& "$*dclassname"

%typecheck(SWIG_TYPECHECK_POINTER)
    SWIGTYPE,
    SWIGTYPE *,
    SWIGTYPE &,
    SWIGTYPE [],
    SWIGTYPE *&
  ""


/*
 * By-value conversion typemaps (parameter is converted to a pointer).
 */

%typemap(in, canthrow=1) SWIGTYPE ($&1_type argp)
%{ argp = ($&1_ltype)$input;
   if (!argp) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Attempt to dereference null $1_type");
     return $null;
   }
   $1 = *argp; %}

%typemap(out) SWIGTYPE
#ifdef __cplusplus
%{ $result = new $1_ltype((const $1_ltype &)$1); %}
#else
{
  $&1_ltype $1ptr = ($&1_ltype) malloc(sizeof($1_ltype));
  memmove($1ptr, &$1, sizeof($1_type));
  $result = $1ptr;
}
#endif

%typemap(directorin) SWIGTYPE
  "$input = (void *)&$1;"
%typemap(directorout) SWIGTYPE
%{ if (!$input) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Unexpected null return for type $1_type");
     return $null;
   }
   $result = *($&1_ltype)$input; %}

%typemap(csdirectorin) SWIGTYPE "new $&dclassname($iminput, false)"
%typemap(csdirectorout) SWIGTYPE "$&dclassname.swigGetCObject($cscall)"

%typemap(csin) SWIGTYPE "$&dclassname.swigGetCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE {
  $&dclassname ret = new $&dclassname($imcall, true);$excode
  return ret;
}


/*
 * Pointer conversion typemaps.
 */

%typemap(in) SWIGTYPE * "$1 = ($1_ltype)$input;"
%typemap(out) SWIGTYPE * "$result = (void *)$1;"

%typemap(directorin) SWIGTYPE *
  "$input = (void *) $1;"
%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE *
  "$result = ($1_ltype)$input;"

%typemap(csdirectorin) SWIGTYPE * "($iminput is null) ? null : new $dclassname($iminput, false)"
%typemap(csdirectorout) SWIGTYPE * "$dclassname.swigGetCObject($cscall)"

%typemap(csin) SWIGTYPE * "$dclassname.swigGetCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE * {
  void* cPtr = $imcall;
  $dclassname ret = (cPtr is null) ? null : new $dclassname(cPtr, $owner);$excode
  return ret;
}


/*
 * Reference conversion typemaps.
 */

%typemap(in, canthrow=1) SWIGTYPE & %{ $1 = ($1_ltype)$input;
  if (!$1) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "$1_type type is null");
    return $null;
  } %}
%typemap(out) SWIGTYPE & "$result = (void *)$1;"

%typemap(directorin) SWIGTYPE &
  "$input = ($1_ltype) &$1;"
%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE &
%{ if (!$input) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Unexpected null return for type $1_type");
     return $null;
   }
   $result = ($1_ltype)$input; %}

%typemap(csdirectorin) SWIGTYPE & "new $dclassname($iminput, false)"
%typemap(csdirectorout) SWIGTYPE & "$dclassname.swigGetCObject($cscall)"

%typemap(csin) SWIGTYPE & "$dclassname.swigGetCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE & {
  $dclassname ret = new $dclassname($imcall, $owner);$excode
  return ret;
}


/*
 * Array conversion typemaps.
 */

%typemap(in) SWIGTYPE [] %{ $1 = ($1_ltype)$input; %}
%typemap(out) SWIGTYPE [] %{ $result = $1; %}

%typemap(csin) SWIGTYPE [] "$dclassname.swigGetCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE [] {
  void* cPtr = $imcall;
  $dclassname ret = (cPtr is null) ? null : new $dclassname(cPtr, $owner);$excode
  return ret;
}

// Treat references to arrays like like references to a single element.
%apply SWIGTYPE & { SWIGTYPE ((&)[ANY]) }


/*
 * Pointer reference conversion typemaps.
 */

%typemap(in) SWIGTYPE *& ($*1_ltype temp = 0)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}
%typemap(out) SWIGTYPE *&
%{ $result = (void *)*$1; %}

%typemap(csin) SWIGTYPE *& "$*dclassname.swigGetCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE *& {
  void* cPtr = $imcall;
  $*dclassname ret = (cPtr is null) ? null : new $*dclassname(cPtr, $owner);$excode
  return ret;
}
