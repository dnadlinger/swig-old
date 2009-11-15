/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * wrapperloader.swg
 *
 * Support code for dynamically linking the C wrapper library from the D
 * wrapper module.
 * ----------------------------------------------------------------------------- */

%pragma(d) wrapperloadercode=%{
/*
 * Dynamically loads the C wrapper library as a shared library.
 *
 * Currently requires Tango, but there is no reason why Phobos could not be
 * supported too.
 */
import tango.sys.SharedLib;

static this() {
  version ( Posix ) {
    Binder.library = SharedLib.load( "lib$wraplibrary.so" );
  }
  version ( Win32 ) {
    Binder.library = SharedLib.load( "$wraplibrary.dll" );
  }

  //#if !defined(SWIG_D_NO_EXCEPTION_HELPER)
  bind( SWIGRegisterExceptionCallbacks_$proxydmodule )( "SWIGRegisterExceptionCallbacks_$proxydmodule" );
  //#endif // SWIG_D_NO_EXCEPTION_HELPER
  //#if !defined(SWIG_D_NO_STRING_HELPER)
  bind( SWIGRegisterStringCallback_$proxydmodule )( "SWIGRegisterStringCallback_$proxydmodule" );
  //#endif // SWIG_D_NO_STRING_HELPER
  $wrapperloaderbindcode
}

static ~this() {
  Binder.library.unload();
}

private {
  // The binding code is heavily inspired by Derelict.
  struct Binder {
  public:
    static SharedLib library;

    static Binder opCall( void** functionPointerAddress ) {
      Binder binder;
      binder.m_functionPointerAddress = functionPointerAddress;
      return binder;
    }

    void opCall( char* name ) {
      *m_functionPointerAddress = library.getSymbol( name );
    }

  private:
    void** m_functionPointerAddress;
  }

  template bind( Function ) {
    static Binder bind( inout Function a ) {
      Binder binder = Binder( cast( void** ) &a );
      return binder;
    }
  }
}

//#if !defined(SWIG_D_NO_EXCEPTION_HELPER)
extern( C ) void function(
  SWIGExceptionCallback exceptionCallback,
  SWIGExceptionCallback illegalArgumentCallback,
  SWIGExceptionCallback illegalElementCallback,
  SWIGExceptionCallback ioCallback,
  SWIGExceptionCallback noSuchElementCallback) SWIGRegisterExceptionCallbacks_$proxydmodule;
//#endif // SWIG_D_NO_EXCEPTION_HELPER

//#if !defined(SWIG_D_NO_STRING_HELPER)
extern( C ) void function(SWIGStringCallback callback) SWIGRegisterStringCallback_$proxydmodule;
//#endif // SWIG_D_NO_STRING_HELPER
%}


%pragma(d) wrapperloaderbindcommand=%{
  bind( $function )( "$symbol" );%}