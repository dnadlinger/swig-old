/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * wrapperloader.swg
 *
 * Support code for dynamically linking the C wrapper library from the D
 * wrapper module.
 * ----------------------------------------------------------------------------- */

%pragma(d) wrapperloadercode = %{
/*
 * Dynamically loads the C wrapper library as a shared library.
 *
 * Currently requires Tango, but there is no reason why Phobos could not be
 * supported too.
 */
static import tango.sys.SharedLib;

static this() {
  tango.sys.SharedLib.SharedLib library;
  version (Posix) {
    library = tango.sys.SharedLib.SharedLib.load("lib$wraplibrary.so");
  }
  version (Win32) {
    library = tango.sys.SharedLib.SharedLib.load("$wraplibrary.dll");
  }

  char[] bindCode(char[] functionPointer, char[] symbol) {
    return functionPointer ~ " = cast(typeof(" ~ functionPointer ~
      "))library.getSymbol(`" ~ symbol ~ "`);";
  }

  //#if !defined(SWIG_D_NO_EXCEPTION_HELPER)
  mixin(bindCode("swigRegisterExceptionCallbacks", "SWIGRegisterExceptionCallbacks_$module"));
  //#endif // SWIG_D_NO_EXCEPTION_HELPER
  //#if !defined(SWIG_D_NO_STRING_HELPER)
  mixin(bindCode("swigRegisterStringCallback", "SWIGRegisterStringCallback_$module"));
  //#endif // SWIG_D_NO_STRING_HELPER
  $wrapperloaderbindcode
}

//#if !defined(SWIG_D_NO_EXCEPTION_HELPER)
extern(C) void function(
  SwigExceptionCallback exceptionCallback,
  SwigExceptionCallback illegalArgumentCallback,
  SwigExceptionCallback illegalElementCallback,
  SwigExceptionCallback ioCallback,
  SwigExceptionCallback noSuchElementCallback) swigRegisterExceptionCallbacks;
//#endif // SWIG_D_NO_EXCEPTION_HELPER

//#if !defined(SWIG_D_NO_STRING_HELPER)
extern(C) void function(SwigStringCallback callback) swigRegisterStringCallback;
//#endif // SWIG_D_NO_STRING_HELPER
%}

%pragma(d) wrapperloaderbindcommand = %{
  mixin(bindCode("$function", "$symbol"));%}
