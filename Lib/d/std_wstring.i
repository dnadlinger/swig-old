/* -----------------------------------------------------------------------------
 * std_wstring.i
 *
 * Typemaps for std::wstring and const std::wstring&
 * These are mapped to a C# String and are passed around by value.
 *
 * To use non-const std::wstring references use the following %apply.  Note
 * that they are passed by value.
 * %apply const std::wstring & {std::wstring &};
 * ----------------------------------------------------------------------------- */

%include <wchar.i>

%{
#include <string>
%}

namespace std {

%naturalvar wstring;

class wstring;

// wstring
%typemap(cwtype, out="void *") wstring "wchar_t *"
%typemap(dwtype, inattributes="[MarshalAs(UnmanagedType.LPWStr)]") wstring "string"
%typemap(dptype) wstring "string"
%typemap(ddirectorin) wstring "$winput"
%typemap(ddirectorout) wstring "$dpcall"

%typemap(in, canthrow=1) wstring
%{ if (!$input) {
    SWIG_DSetPendingExceptionArgument(SWIG_DIllegalArgumentException, "null wstring", 0);
    return $null;
   }
   $1.assign($input); %}
%typemap(out) wstring %{ $result = SWIG_d_wstring_callback($1.c_str()); %}

%typemap(directorout, canthrow=1) wstring
%{ if (!$input) {
    SWIG_DSetPendingExceptionArgument(SWIG_DIllegalArgumentException, "null wstring", 0);
    return $null;
   }
   $result.assign($input); %}

%typemap(directorin) wstring %{ $input = SWIG_d_wstring_callback($1.c_str()); %}

%typemap(din) wstring "$dinput"
%typemap(dout, excode=SWIGEXCODE) wstring {
    string ret = $wcall;$excode
    return ret;
  }

%typemap(typecheck) wstring = wchar_t *;

%typemap(throws, canthrow=1) wstring
%{ std::string message($1.begin(), $1.end());
   SWIG_DSetPendingException(SWIG_DApplicationException, message.c_str());
   return $null; %}

// const wstring &
%typemap(cwtype, out="void *") const wstring & "wchar_t *"
%typemap(dwtype, inattributes="[MarshalAs(UnmanagedType.LPWStr)]") const wstring & "string"
%typemap(dptype) const wstring & "string"

%typemap(ddirectorin) const wstring & "$winput"
%typemap(ddirectorout) const wstring & "$dpcall"

%typemap(in, canthrow=1) const wstring &
%{ if (!$input) {
    SWIG_DSetPendingExceptionArgument(SWIG_DIllegalArgumentException, "null wstring", 0);
    return $null;
   }
   std::wstring $1_str($input);
   $1 = &$1_str; %}
%typemap(out) const wstring & %{ $result = SWIG_d_wstring_callback($1->c_str()); %}

%typemap(din) const wstring & "$dinput"
%typemap(dout, excode=SWIGEXCODE) const wstring & {
    string ret = $wcall;$excode
    return ret;
  }

%typemap(directorout, canthrow=1, warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const wstring &
%{ if (!$input) {
    SWIG_DSetPendingExceptionArgument(SWIG_DIllegalArgumentException, "null wstring", 0);
    return $null;
   }
   /* possible thread/reentrant code problem */
   static std::wstring $1_str;
   $1_str = $input;
   $result = &$1_str; %}

%typemap(directorin) const wstring & %{ $input = SWIG_d_wstring_callback($1.c_str()); %}

%typemap(csvarin, excode=SWIGEXCODE2) const wstring & %{
    set {
      $wcall;$excode
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) const wstring & %{
    get {
      string ret = $wcall;$excode
      return ret;
    } %}

%typemap(typecheck) const wstring & = wchar_t *;

%typemap(throws, canthrow=1) const wstring &
%{ std::string message($1.begin(), $1.end());
   SWIG_DSetPendingException(SWIG_DApplicationException, message.c_str());
   return $null; %}

}

