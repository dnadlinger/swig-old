#if defined(SWIGJAVA)
%include <java/std_except.i>
#elif defined(SWIGCSHARP)
%include <csharp/std_except.i>
#else


/* avoid to include std/std_except.i unless we really need it */
#if !defined(SWIG_STD_EXCEPTIONS_AS_CLASSES)
%{
#include <stdexcept>
%}
#else
%include <std/std_except.i>
#endif


// Typemaps used by the STL wrappers that throw exceptions.
// These typemaps are used when methods are declared with an STL
// exception specification, such as
//
//   size_t at() const throw (std::out_of_range);
//

%include <exception.i>

/* 
   Mark all of std exception classes as "exception classes" via
   the "exceptionclass" feature.
   
   If needed, you can disable it by using %noexceptionclass.
*/

%define %std_exception_map(Exception, Code)
  %exceptionclass  Exception; 
#if !defined(SWIG_STD_EXCEPTIONS_AS_CLASSES)
  %typemap(throws,noblock=1) Exception {
    SWIG_exception(Code, $1.what());
  }
  %ignore Exception;
  struct Exception {
  };
#endif
%enddef

namespace std {
  %std_exception_map(bad_exception,      SWIG_SystemError);
  %std_exception_map(domain_error,       SWIG_ValueError);
  %std_exception_map(exception,          SWIG_SystemError);
  %std_exception_map(invalid_argument,   SWIG_ValueError);
  %std_exception_map(length_error,       SWIG_IndexError);
  %std_exception_map(logic_error,        SWIG_RuntimeError);
  %std_exception_map(out_of_range,       SWIG_IndexError);
  %std_exception_map(overflow_error,     SWIG_OverflowError);
  %std_exception_map(range_error,        SWIG_OverflowError);
  %std_exception_map(runtime_error,      SWIG_RuntimeError);
  %std_exception_map(underflow_error,    SWIG_OverflowError);
}

#endif

