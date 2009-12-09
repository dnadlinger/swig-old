/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * denums.swg
 *
 * Typemaps for enumerations.
 * ----------------------------------------------------------------------------- */


/*
 * Typemaps for enumeration types.
 */

%typemap(ctype) enum SWIGTYPE "int"
%typemap(imtype) enum SWIGTYPE "int"
%typemap(cstype) enum SWIGTYPE "$dclassname"

%typemap(in) enum SWIGTYPE %{ $1 = ($1_ltype)$input; %}
%typemap(out) enum SWIGTYPE %{ $result = $1; %}

%typemap(directorout) enum SWIGTYPE %{ $result = ($1_ltype)$input; %}
%typemap(directorin) enum SWIGTYPE "$input = $1;"
%typemap(csdirectorin) enum SWIGTYPE "cast($dclassname)$iminput"
%typemap(csdirectorout) enum SWIGTYPE "cast(int)$cscall"

%typecheck(SWIG_TYPECHECK_POINTER) enum SWIGTYPE ""

%typemap(throws, canthrow=1) enum SWIGTYPE
%{ (void)$1;
   SWIG_DSetPendingException(SWIG_DException, "C++ $1_type exception thrown");
   return $null; %}

%typemap(csin) enum SWIGTYPE "cast(int)$csinput"
%typemap(csout, excode=SWIGEXCODE) enum SWIGTYPE {
  $dclassname ret = cast($dclassname)$imcall;$excode
  return ret;
}

%typemap(dclassmodifiers) enum SWIGTYPE "enum"
%typemap(dcode) enum SWIGTYPE ""


/*
 * Typemaps for (const) references to enumeration types.
 */

%typemap(ctype) const enum SWIGTYPE & "int"
%typemap(imtype) const enum SWIGTYPE & "int"
%typemap(cstype) const enum SWIGTYPE & "$*dclassname"

%typemap(in) const enum SWIGTYPE & ($*1_ltype temp)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}
%typemap(out) const enum SWIGTYPE & %{ $result = *$1; %}

%typemap(directorin) const enum SWIGTYPE & "$input = $1_name;"
%typemap(directorout,warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const enum SWIGTYPE &
%{ static $*1_ltype temp = ($*1_ltype)$input;
   $result = &temp; %}

%typemap(csdirectorin) const enum SWIGTYPE & "cast($*dclassname)$iminput"
%typemap(csdirectorout) const enum SWIGTYPE & "cast(int)$cscall"

%typemap(csin) const enum SWIGTYPE & "cast(int)$csinput"
%typemap(csout, excode=SWIGEXCODE) const enum SWIGTYPE & {
  $*dclassname ret = cast($*dclassname)$imcall;$excode
  return ret;
}

%typemap(throws, canthrow=1) const enum SWIGTYPE &
%{ (void)$1;
   SWIG_DSetPendingException(SWIG_DException, "C++ $1_type exception thrown");
   return $null; %}

%typecheck(SWIG_TYPECHECK_POINTER) const enum SWIGTYPE & ""
