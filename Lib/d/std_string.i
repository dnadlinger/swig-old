/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * std_string.i
 *
 * Typemaps for std::string and const std::string&
 * These are mapped to a D char[] and are passed around by value.
 *
 * To use non-const std::string references, use the following %apply. Note
 * that they are passed by value.
 * %apply const std::string & {std::string &};
 * ----------------------------------------------------------------------------- */

%{
#include <string>
%}

namespace std {

%naturalvar string;

class string;

// string
%typemap(ctype) string, const string & "char *"
%typemap(imtype) string, const string & "char*"
%typemap(cstype) string, const string & "char[]"

%typemap(in, canthrow=1) string, const string &
%{ if (!$input) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "null string");
    return $null;
  }
  $1.assign($input); %}
%typemap(in, canthrow=1) const string &
%{ if (!$input) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "null string");
    return $null;
   }
   std::string $1_str($input);
   $1 = &$1_str; %}

%typemap(out) string %{ $result = SWIG_d_string_callback($1.c_str()); %}
%typemap(out) const string & %{ $result = SWIG_csharp_string_callback($1->c_str()); %}

%typemap(csin) string, const string & "tango.stdc.stringz.toStringz($csinput)"
%typemap(csout, excode=SWIGEXCODE) string, const string & {
  char[] ret = tango.stdc.stringz.fromStringz($imcall).dup;$excode
  return ret;
}

%typemap(directorin) string, const string & %{ $input = SWIG_d_string_callback($1.c_str()); %}

%typemap(directorout, canthrow=1) string
%{ if (!$input) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "null string");
    return $null;
  }
  $result.assign($input); %}

%typemap(directorout, canthrow=1, warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const string &
%{ if (!$input) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "null string");
    return $null;
  }
  /* possible thread/reentrant code problem */
  static std::string $1_str;
  $1_str = $input;
  $result = &$1_str; %}

%typemap(csdirectorin) string, const string & "$iminput"
%typemap(csdirectorout) string, const string & "tango.stdc.stringz.toStringz($cscall)"

%typemap(throws, canthrow=1) string, const string &
%{ SWIG_DSetPendingException(SWIG_DException, $1.c_str());
  return $null; %}

%typemap(typecheck) string, const string & = char *;

} // namespace std
