/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dhead.swg
 *
 * Support code for exceptions if the SWIG_D_NO_EXCEPTION_HELPER is not defined
 * Support code for strings if the SWIG_D_NO_STRING_HELPER is not defined
 * ----------------------------------------------------------------------------- */

%insert(runtime) %{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
%}


#if !defined(SWIG_D_NO_EXCEPTION_HELPER)
%insert(runtime) %{
// Support for throwing D exceptions from C/C++.
typedef enum {
  SWIG_DException = 0,
  SWIG_DIllegalArgumentException,
  SWIG_DIllegalElementException,
  SWIG_DIOException,
  SWIG_DNoSuchElementException,
} SWIG_DExceptionCodes;

typedef void (SWIGSTDCALL* SWIG_DExceptionCallback_t)(const char *);

typedef struct {
  SWIG_DExceptionCodes code;
  SWIG_DExceptionCallback_t callback;
} SWIG_DException_t;

static SWIG_DException_t SWIG_d_exceptions[] = {
  { SWIG_DException, NULL },
  { SWIG_DIllegalArgumentException, NULL },
  { SWIG_DIllegalElementException, NULL },
  { SWIG_DIOException, NULL },
  { SWIG_DNoSuchElementException, NULL }
};

static void SWIGUNUSED SWIG_DSetPendingException(SWIG_DExceptionCodes code, const char *msg) {
  if ((size_t)code < sizeof(SWIG_d_exceptions)/sizeof(SWIG_DException_t)) {
    SWIG_d_exceptions[code].callback(msg);
  } else {
    SWIG_d_exceptions[SWIG_DException].callback(msg);
  }
}

#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionCallbacks_$proxydmodule(
  SWIG_DExceptionCallback_t exceptionCallback,
  SWIG_DExceptionCallback_t illegalArgumentCallback,
  SWIG_DExceptionCallback_t illegalElementCallback,
  SWIG_DExceptionCallback_t ioCallback,
  SWIG_DExceptionCallback_t noSuchElementCallback) {

  SWIG_d_exceptions[SWIG_DException].callback = exceptionCallback;
  SWIG_d_exceptions[SWIG_DIllegalArgumentException].callback = illegalArgumentCallback;
  SWIG_d_exceptions[SWIG_DIllegalElementException].callback = illegalElementCallback;
  SWIG_d_exceptions[SWIG_DIOException].callback = ioCallback;
  SWIG_d_exceptions[SWIG_DNoSuchElementException].callback = noSuchElementCallback;
}
%}

%pragma(d) wrapdmoduleimports=%{
// Exception throwing support currently requires Tango, but there is no reason
// why it could not support Phobos.
import tango.core.Exception;
static import tango.stdc.stringz;
%}

%pragma(d) wrapdmodulecode=%{
private class SWIGExceptionHelper {
  static void setException(char* message) {
    auto exception = new Exception(tango.stdc.stringz.fromStringz(message));
    exception.next = SWIGPendingException.retrieve();
    SWIGPendingException.set(exception);
  }
  static void setIllegalArgumentException(char* message) {
    auto exception = new IllegalArgumentException(tango.stdc.stringz.fromStringz(message));
    exception.next = SWIGPendingException.retrieve();
    SWIGPendingException.set(exception);
  }
  static void setIllegalElementException(char* message) {
    auto exception = new IllegalElementException(tango.stdc.stringz.fromStringz(message));
    exception.next = SWIGPendingException.retrieve();
    SWIGPendingException.set(exception);
  }
  static void setIOException(char* message) {
    auto exception = new IOException(tango.stdc.stringz.fromStringz(message));
    exception.next = SWIGPendingException.retrieve();
    SWIGPendingException.set(exception);
  }
  static void setNoSuchElementException(char* message) {
    auto exception = new NoSuchElementException(tango.stdc.stringz.fromStringz(message));
    exception.next = SWIGPendingException.retrieve();
    SWIGPendingException.set(exception);
  }

  static this() {
    SWIGRegisterExceptionCallbacks_$proxydmodule(
      &setException,
      &setIllegalArgumentException,
      &setIllegalElementException,
      &setIOException,
      &setNoSuchElementException);
  }
}

private class SWIGPendingException {
public:
  static bool isPending() {
    bool pending = false;
    if (m_sNumExceptionsPending > 0)
      if (m_sPendingException !is null)
	pending = true;
    return pending;
  }

  static void set(Exception e) {
    if (m_sPendingException !is null) {
      throw new Exception( "FATAL: An earlier pending exception from C/C++ code " ~
	"was missed and thus not thrown (" ~ m_sPendingException.classinfo.name ~
	": " ~ m_sPendingException.msg ~ ")!", e );
    }

    m_sPendingException = e;
    synchronized {
      m_sNumExceptionsPending++;
    }
  }

  static Exception retrieve() {
    Exception e = null;
    if (m_sNumExceptionsPending > 0) {
      if (m_sPendingException !is null) {
	e = m_sPendingException;
	m_sPendingException = null;
	synchronized {
	  m_sNumExceptionsPending--;
	}
      }
    }
    return e;
  }

private:
  // This is intended to be thread-global.
  static int m_sNumExceptionsPending = 0;
  // This is intended to be thread-local.
  static Exception m_sPendingException = null;
}

alias void function( char* message ) SWIGExceptionCallback;
%}
// Callback registering function in wrapperloader.swg.
#endif // SWIG_D_NO_EXCEPTION_HELPER

#if !defined(SWIG_D_NO_STRING_HELPER)
%insert(runtime) %{
// Callback for returning strings to D without leaking memory.
typedef char * (SWIGSTDCALL* SWIG_DStringHelperCallback)(const char *);
static SWIG_DStringHelperCallback SWIG_d_string_callback = NULL;

#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterStringCallback_$proxydmodule(SWIG_DStringHelperCallback callback) {
  SWIG_d_string_callback = callback;
}
%}

%pragma(d) wrapdmodulecode=%{
private class SWIGStringHelper {
  static char* createString(char* cString) {
    // TODO: Research what the C# module is doing here and port it to D.
    return cString;
  }

  static this() {
    SWIGRegisterStringCallback_$proxydmodule(&createString);
  }
}
alias char* function( char* cString ) SWIGStringCallback;
%}
// Callback registering function in wrapperloader.swg.
#endif // SWIG_D_NO_STRING_HELPER

%insert(runtime) %{
/* Contract support */

#define SWIG_contract_assert(nullreturn, expr, msg) if (!(expr)) {SWIG_DSetPendingExceptionArgument(SWIG_DArgumentOutOfRangeException, msg, ""); return nullreturn; } else
%}