//
// except.i
// Dave Beazley
// April 14, 1997
//
// This SWIG library file provides language independent exception handling

#ifdef AUTODOC
%section "Exception Handling Library",info,after,pre,nosort,skip=1,chop_left=3,chop_right=0,chop_top=0,chop_bottom=0

%text %{
%include exception.i

This library provides language independent support for raising scripting
language exceptions in SWIG generated wrapper code.    Normally, this is
used in conjunction with the %except directive.

To raise an exception, use the following function call :

       SWIG_exception(int exctype, char *msg);

'exctype' is an exception type code and may be one of the following :

       SWIG_MemoryError
       SWIG_IOError
       SWIG_RuntimeError
       SWIG_IndexError
       SWIG_TypeError
       SWIG_DivisionByZero
       SWIG_OverflowError
       SWIG_SyntaxError
       SWIG_ValueError
       SWIG_SystemError
       SWIG_UnknownError

'msg' is an error string that should be reported to the user.

The library is normally used in conjunction with the %except directive
as follows :

%except {
       try {
          $function
       } catch RangeError {
          SWIG_exception(SWIG_IndexError,"Array index out of bounds");
       } catch(...) {
          SWIG_exception(SWIG_UnknownError,"Uncaught exception");
       }
}

It is important to note that the SWIG_exception() function is only available
to the C code generated by SWIG.  It is not available in the scripting language
interface itself.
%}

#endif

%{
#define  SWIG_MemoryError    1
#define  SWIG_IOError        2
#define  SWIG_RuntimeError   3
#define  SWIG_IndexError     4
#define  SWIG_TypeError      5
#define  SWIG_DivisionByZero 6
#define  SWIG_OverflowError  7
#define  SWIG_SyntaxError    8
#define  SWIG_ValueError     9
#define  SWIG_SystemError   10
#define  SWIG_UnknownError  99
%}

#ifdef SWIGTCL8
%{
#define SWIG_exception(a,b)   { Tcl_SetResult(interp,b,TCL_VOLATILE); return TCL_ERROR; }
%}
#else
#ifdef SWIGTCL
%{
#define SWIG_exception(a,b)   { Tcl_SetResult(interp,b,TCL_VOLATILE); return TCL_ERROR; }
%}
#endif
#endif

#ifdef SWIGPERL5
%{
#define SWIG_exception(a,b)   croak(b)
%}
#endif

#ifdef SWIGPERL4
%{
#define SWIG_exception(a,b)   fatal(b)
%}
#endif

#ifdef SWIGPYTHON
%{
static void _SWIG_exception(int code, char *msg) {
  switch(code) {
  case SWIG_MemoryError:
    PyErr_SetString(PyExc_MemoryError,msg);
    break;
  case SWIG_IOError:
    PyErr_SetString(PyExc_IOError,msg);
    break;
  case SWIG_RuntimeError:
    PyErr_SetString(PyExc_RuntimeError,msg);
    break;
  case SWIG_IndexError:
    PyErr_SetString(PyExc_IndexError,msg);
    break;
  case SWIG_TypeError:
    PyErr_SetString(PyExc_TypeError,msg);
    break;
  case SWIG_DivisionByZero:
    PyErr_SetString(PyExc_ZeroDivisionError,msg);
    break;
  case SWIG_OverflowError:
    PyErr_SetString(PyExc_OverflowError,msg);
    break;
  case SWIG_SyntaxError:
    PyErr_SetString(PyExc_SyntaxError,msg);
    break;
  case SWIG_ValueError:
    PyErr_SetString(PyExc_ValueError,msg);
    break;
  case SWIG_SystemError:
    PyErr_SetString(PyExc_SystemError,msg);
    break;
  default:
    PyErr_SetString(PyExc_RuntimeError,msg);
    break;
  }
}

#define SWIG_exception(a,b) { _SWIG_exception(a,b); return NULL; }
%}
#endif

#ifdef SWIGGUILE
%{
  static void _SWIG_exception (int code, const char *msg,
                               const char *subr) {
#define ERROR(scmerr)					\
	scm_error(gh_symbol2scm((char *) (scmerr)),	\
		  (char *) subr, (char *) msg,		\
		  SCM_EOL, SCM_BOOL_F)
#define MAP(swigerr, scmerr)			\
	case swigerr:				\
	  ERROR(scmerr);			\
	  break
    switch (code) {
      MAP(SWIG_MemoryError,	"swig-memory-error");
      MAP(SWIG_IOError,		"swig-io-error");
      MAP(SWIG_RuntimeError,	"swig-runtime-error");
      MAP(SWIG_IndexError,	"swig-index-error");
      MAP(SWIG_TypeError,	"swig-type-error");
      MAP(SWIG_DivisionByZero,	"swig-division-by-zero");
      MAP(SWIG_OverflowError,	"swig-overflow-error");
      MAP(SWIG_SyntaxError,	"swig-syntax-error");
      MAP(SWIG_ValueError,	"swig-value-error");
      MAP(SWIG_SystemError,	"swig-system-error");
    default:
      ERROR("swig-error");
    }
#undef ERROR
#undef MAP
  }

#define SWIG_exception(a,b) _SWIG_exception(a, b, FUNC_NAME)
%}
#endif

#ifdef SWIGJAVA
%{
static void _SWIG_exception(JNIEnv *jenv, int code, const char *msg) {
  char exception_path[512];
  const char *classname;
  const char *class_package = "java/lang";
  

  switch(code) {
  case SWIG_MemoryError:
    classname = "OutOfMemoryError";
    break;
  case SWIG_IOError:
    classname = "IOException";
    class_package = "java/io";
    break;
  case SWIG_SystemError:
  case SWIG_RuntimeError:
    classname = "RuntimeException";
    break;
  case SWIG_OverflowError:
  case SWIG_IndexError:
    classname = "IndexOutOfBoundsException";
    break;
  case SWIG_DivisionByZero:
    classname = "ArithmeticException";
    break;
  case SWIG_SyntaxError:
  case SWIG_ValueError:
  case SWIG_TypeError:
    classname = "IllegalArgumentException";
    break;
  case SWIG_UnknownError:
  default:
    classname = "UnknownError";
    break;
  }
  sprintf(exception_path, "%s/%s", class_package, classname);
  jclass excep;
  jenv->ExceptionClear();
  excep = jenv->FindClass(exception_path);
  if (excep)
  {
    jenv->ThrowNew(excep, msg);
  }
}
#define SWIG_exception(a,b) { _SWIG_exception(jenv, a,b); }
%}
#endif // SWIGJAVA

/* exception.i ends here */
