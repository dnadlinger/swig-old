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
  bind( __SWIGRegisterExceptionCallbacks_$proxydmodule )( "SWIGRegisterExceptionCallbacks_$proxydmodule" );
  //#endif // SWIG_D_NO_EXCEPTION_HELPER
  //#if !defined(SWIG_D_NO_STRING_HELPER)
  bind( __SWIGRegisterStringCallback_$proxydmodule )( "SWIGRegisterStringCallback_$proxydmodule" );
  //#endif // SWIG_D_NO_STRING_HELPER
  $wrapperloaderbindcode
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
  __SWIGExceptionCallback exceptionCallback,
  __SWIGExceptionCallback illegalArgumentCallback,
  __SWIGExceptionCallback illegalElementCallback,
  __SWIGExceptionCallback ioCallback,
  __SWIGExceptionCallback noSuchElementCallback) __SWIGRegisterExceptionCallbacks_$proxydmodule;
//#endif // SWIG_D_NO_EXCEPTION_HELPER

//#if !defined(SWIG_D_NO_STRING_HELPER)
extern( C ) void function(__SWIGStringCallback callback) __SWIGRegisterStringCallback_$proxydmodule;
//#endif // SWIG_D_NO_STRING_HELPER
%}


%pragma(d) wrapperloaderbindcommand=%{
  bind( $function )( "$symbol" );%}