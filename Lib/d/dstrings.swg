/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dstrings.swg
 *
 * Typemaps for wrapping pointers to/arrays of C chars as D strings.
 * ----------------------------------------------------------------------------- */

%typemap(cwtype)  char *, char *&, char[ANY], char[]   "char *"
%typemap(dwtype) char *, char *&, char[ANY], char[]   "char*"
%typemap(dptype) char *, char *&, char[ANY], char[]   "char[]"

%typecheck(SWIG_TYPECHECK_STRING)
    char *,
    char *&,
    char[ANY],
    char[]
  ""

// We need the \0-terminated string conversion functions.
// Could be easily extended to support Phobos as well.
%pragma(d) globalproxyimports = "static import tango.stdc.stringz;";


/*
 * char* typemaps.
 */

%typemap(in) char * %{ $1 = ($1_ltype)$input; %}
%typemap(out) char * %{ $result = SWIG_d_string_callback((const char *)$1); %}
%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) char * %{ $result = ($1_ltype)$input; %}
%typemap(directorin) char * %{ $input = SWIG_d_string_callback((const char *)$1); %}
%typemap(ddirectorin) char * "tango.stdc.stringz.fromStringz($winput)"
%typemap(ddirectorout) char * "tango.stdc.stringz.toStringz($dpcall)"


/*
 * char*& typemaps.
 */

%typemap(in) char *& ($*1_ltype temp = 0) %{
  temp = ($*1_ltype)$input;
  $1 = &temp;
%}
%typemap(out) char *& %{ if ($1) $result = SWIG_d_string_callback((const char *)*$1); %}


/*
 * char array typemaps.
 */

%typemap(in) char[ANY], char[] %{ $1 = ($1_ltype)$input; %}
%typemap(out) char[ANY], char[] %{ $result = SWIG_d_string_callback((const char *)$1); %}

%typemap(directorout) char[ANY], char[] %{ $result = ($1_ltype)$input; %}
%typemap(directorin) char[ANY], char[] %{ $input = SWIG_d_string_callback((const char *)$1); %}

%typemap(ddirectorin) char[ANY], char[] "$winput"
%typemap(ddirectorout) char[ANY], char[] "$dpcall"

%typemap(din) char *, char *&, char[ANY], char[] "tango.stdc.stringz.toStringz($dinput)"
%typemap(dout, excode=SWIGEXCODE) char *, char *&, char[ANY], char[] {
  char[] ret = tango.stdc.stringz.fromStringz($wcall);$excode
  return ret;
}
