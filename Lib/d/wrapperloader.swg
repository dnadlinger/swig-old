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
  version (Posix) {
    SwigBinder.library = tango.sys.SharedLib.SharedLib.load("lib$wraplibrary.so");
  }
  version (Win32) {
    SwigBinder.library = tango.sys.SharedLib.SharedLib.load("$wraplibrary.dll");
  }

  //#if !defined(SWIG_D_NO_EXCEPTION_HELPER)
  bind(swigRegisterExceptionCallbacks)("SWIGRegisterExceptionCallbacks_$module");
  //#endif // SWIG_D_NO_EXCEPTION_HELPER
  //#if !defined(SWIG_D_NO_STRING_HELPER)
  bind(swigRegisterStringCallback)("SWIGRegisterStringCallback_$module");
  //#endif // SWIG_D_NO_STRING_HELPER
  $wrapperloaderbindcode
}

private {
  // The binding code is heavily inspired by Derelict.
  struct SwigBinder {
  public:
    static tango.sys.SharedLib.SharedLib library;

    static SwigBinder opCall(void** functionPointerAddress) {
      SwigBinder binder;
      binder.m_functionPointerAddress = functionPointerAddress;
      return binder;
    }

    void opCall(char* name) {
      *m_functionPointerAddress = library.getSymbol(name);
    }

  private:
    void** m_functionPointerAddress;
  }

  template bind(Function) {
    static SwigBinder bind(inout Function a) {
      return SwigBinder(cast(void**) &a);
    }
  }
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
  bind($function)("$symbol");%}
