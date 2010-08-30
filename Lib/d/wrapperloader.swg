/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * wrapperloader.swg
 *
 * Support code for dynamically linking the C wrapper library from the D
 * wrapper module.
 * ----------------------------------------------------------------------------- */

#if (SWIG_D_VERSION == 1)
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
#else
%pragma(d) wrapperloadercode = %{
/*
 * Dynamically loads the C wrapper library as a shared library.
 *
 * Currently uses the undocumented std.loader module, adapting it to another
 * library should be easy though.
 */
static import std.loader;

static this() {
  scope std.loader.ExeModule library;
  version (Posix) {
    version(darwin) {
      library = new std.loader.ExeModule("lib$wraplibrary.dylib");
	  } else {
	    library = new std.loader.ExeModule("lib$wraplibrary.so");
	  }
  }
  version (Win32) {
    library = new std.loader.ExeModule("$wraplibrary.dll");
  }

  string bindCode(string functionPointer, string symbol) {
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

  // HACK: Do not destruct library.
  library = null;
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
#endif

%pragma(d) wrapperloaderbindcommand = %{
  mixin(bindCode("$function", "$symbol"));%}
