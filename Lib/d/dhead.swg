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
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionCallbacks_$module(
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
static import tango.core.Thread;
static import tango.stdc.stringz;
%}

%pragma(d) wrapdmodulecode=%{
private class __SWIGExceptionHelper {
  static this() {
    __SWIGRegisterExceptionCallbacks_$module(
      &setException,
      &setIllegalArgumentException,
      &setIllegalElementException,
      &setIOException,
      &setNoSuchElementException);
  }

  static void setException(char* message) {
    auto exception = new Exception(tango.stdc.stringz.fromStringz(message).dup);
    exception.next = __SWIGPendingException.retrieve();
    __SWIGPendingException.set(exception);
  }
  static void setIllegalArgumentException(char* message) {
    auto exception = new IllegalArgumentException(tango.stdc.stringz.fromStringz(message).dup);
    exception.next = __SWIGPendingException.retrieve();
    __SWIGPendingException.set(exception);
  }
  static void setIllegalElementException(char* message) {
    auto exception = new IllegalElementException(tango.stdc.stringz.fromStringz(message).dup);
    exception.next = __SWIGPendingException.retrieve();
    __SWIGPendingException.set(exception);
  }
  static void setIOException(char* message) {
    auto exception = new IOException(tango.stdc.stringz.fromStringz(message).dup);
    exception.next = __SWIGPendingException.retrieve();
    __SWIGPendingException.set(exception);
  }
  static void setNoSuchElementException(char* message) {
    auto exception = new NoSuchElementException(tango.stdc.stringz.fromStringz(message).dup);
    exception.next = __SWIGPendingException.retrieve();
    __SWIGPendingException.set(exception);
  }
}

package class __SWIGPendingException {
public:
  static this() {
    m_sPendingCount = 0;
    m_sPendingException = new ThreadLocalData( null );
  }

  static bool isPending() {
    bool pending = false;
    if (m_sPendingCount > 0) {
      if (m_sPendingException.val !is null) {
	pending = true;
      }
    }
    return pending;
  }

  static void set(Exception e) {
    if (m_sPendingException.val !is null) {
      throw new Exception( "FATAL: An earlier pending exception from C/C++ code " ~
	"was missed and thus not thrown (" ~ m_sPendingException.val.classinfo.name ~
	": " ~ m_sPendingException.val.msg ~ ")!", e );
    }

    m_sPendingException.val = e;
    synchronized {
      ++m_sPendingCount;
    }
  }

  static Exception retrieve() {
    Exception e = null;
    if (m_sPendingCount > 0) {
      if (m_sPendingException.val !is null) {
	e = m_sPendingException.val;
	m_sPendingException.val = null;
	synchronized {
	  --m_sPendingCount;
	}
      }
    }
    return e;
  }

private:
  // The pending exception counter is stored thread-global.
  static int m_sPendingCount;

  // The reference to the pending exception (if any) is stored thread-local.
  alias tango.core.Thread.ThreadLocal!(Exception) ThreadLocalData;
  static ThreadLocalData m_sPendingException;
}

alias void function( char* message ) __SWIGExceptionCallback;
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
SWIGEXPORT void SWIGSTDCALL SWIGRegisterStringCallback_$module(SWIG_DStringHelperCallback callback) {
  SWIG_d_string_callback = callback;
}
%}

// TODO: Make %pragma(d) wrapdmoduleimports appending instead of replacing and
// add tango.stdc.stringz import here.

%pragma(d) wrapdmodulecode=%{
private class __SWIGStringHelper {
  static this() {
    __SWIGRegisterStringCallback_$module(&createString);
  }

  static char* createString(char* cString) {
    // We are effectively dup'ing the string here.
    return tango.stdc.stringz.toStringz(tango.stdc.stringz.fromStringz(cString));
  }
}
alias char* function( char* cString ) __SWIGStringCallback;
%}
// Callback registering function in wrapperloader.swg.
#endif // SWIG_D_NO_STRING_HELPER

%insert(runtime) %{
/* Contract support */

#define SWIG_contract_assert(nullreturn, expr, msg) if (!(expr)) {SWIG_DSetPendingException(SWIG_DException, msg); return nullreturn; } else
%}