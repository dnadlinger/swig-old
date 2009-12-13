/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * wchar.i
 *
 * Typemaps for the wchar_t type
 * These are mapped to a C# String and are passed around by value.
 *
 * Support code for wide strings can be turned off by defining SWIG_D_NO_WSTRING_HELPER
 *
 * ----------------------------------------------------------------------------- */

#if !defined(SWIG_D_NO_WSTRING_HELPER)
#if !defined(SWIG_D_WSTRING_HELPER_)
#define SWIG_D_WSTRING_HELPER_
%insert(runtime) %{
/* Callback for returning strings to C# without leaking memory */
typedef void * (SWIGSTDCALL* SWIG_DWStringHelperCallback)(const wchar_t *);
static SWIG_DWStringHelperCallback SWIG_d_wstring_callback = NULL;
%}

%pragma(d) wrapdmodulecode=%{
  protected class SWIGWStringHelper {

    public delegate string SWIGWStringDelegate(IntPtr message);
    static SWIGWStringDelegate wstringDelegate = new SWIGWStringDelegate(CreateWString);

    [DllImport("$dllimport", EntryPoint="SWIGRegisterWStringCallback_$proxydmodule")]
    public static extern void SWIGRegisterWStringCallback_$proxydmodule(SWIGWStringDelegate wstringDelegate);

    static string CreateWString([MarshalAs(UnmanagedType.LPWStr)]IntPtr cString) {
      return System.Runtime.InteropServices.Marshal.PtrToStringUni(cString);
    }

    static SWIGWStringHelper() {
      SWIGRegisterWStringCallback_$proxydmodule(wstringDelegate);
    }
  }

  static protected SWIGWStringHelper swigWStringHelper = new SWIGWStringHelper();
%}

%insert(runtime) %{
#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterWStringCallback_$proxydmodule(SWIG_DWStringHelperCallback callback) {
  SWIG_d_wstring_callback = callback;
}
%}
#endif // SWIG_D_WSTRING_HELPER_
#endif // SWIG_D_NO_WSTRING_HELPER


// wchar_t
%typemap(cwtype) wchar_t "wchar_t"
%typemap(dwtype) wchar_t "char"
%typemap(dptype) wchar_t "char"

%typemap(din) wchar_t "$dinput"
%typemap(dout, excode=SWIGEXCODE) wchar_t {
    char ret = $wcall;$excode
    return ret;
  }
%typemap(csvarin, excode=SWIGEXCODE2) wchar_t %{
    set {
      $wcall;$excode
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) wchar_t %{
    get {
      char ret = $wcall;$excode
      return ret;
    } %}

%typemap(in) wchar_t %{ $1 = ($1_ltype)$input; %}
%typemap(out) wchar_t %{ $result = (wchar_t)$1; %}

%typemap(typecheck) wchar_t = char;

// wchar_t *
%typemap(cwtype) wchar_t * "wchar_t *"
%typemap(dwtype, inattributes="[MarshalAs(UnmanagedType.LPWStr)]", out="IntPtr" ) wchar_t * "string"
%typemap(dptype) wchar_t * "string"

%typemap(din) wchar_t * "$dinput"
%typemap(dout, excode=SWIGEXCODE) wchar_t * {
    string ret = System.Runtime.InteropServices.Marshal.PtrToStringUni($wcall);$excode
    return ret;
  }
%typemap(csvarin, excode=SWIGEXCODE2) wchar_t * %{
    set {
      $wcall;$excode
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) wchar_t * %{
    get {
      string ret = $wcall;$excode
      return ret;
    } %}

%typemap(in) wchar_t * %{ $1 = ($1_ltype)$input; %}
%typemap(out) wchar_t * %{ $result = (wchar_t *)$1; %}

%typemap(typecheck) wchar_t * = char *;

