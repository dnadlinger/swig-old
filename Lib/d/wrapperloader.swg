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
static import std.algorithm;
static import std.loader;
static import std.range;

static this() {
  string[] possibleFileNames;
  version (Posix) {
    version (OSX) {
      possibleFileNames ~= ["lib$wraplibrary.dylib", "lib$wraplibrary.bundle"];
    }
    possibleFileNames ~= ["lib$wraplibrary.so"];
  } else version (Windows) {
    possibleFileNames ~= ["$wraplibrary.dll", "lib$wraplibrary.so"];
  } else {
    static assert(false, "Operating system not supported by the wrapper loading code.");
  }

  string[] errorMessages;
  scope std.loader.ExeModule library;
  foreach (fileName; possibleFileNames) {
    try {
      library = new std.loader.ExeModule(fileName);
    } catch (Exception e) {
      errorMessages ~= e.msg;
    }
    if (library !is null) {
      break;
    }
  }

  if (library is null) {
    throw new Exception(
      "Could not load wrapper library dynamically. Tried: " ~
      std.algorithm.reduce!`a ~ b`(
        std.algorithm.map!`"\n " ~ a.at!(0) ~ "(" ~ a.at!(1) ~ ")"`(std.range.zip(possibleFileNames, errorMessages))
      )
    );
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
