/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * d.swg
 *
 * D typemaps
 * ----------------------------------------------------------------------------- */

/*
 * Mapping of types between the C/C++ library, the C/C++ library wrapper
 * exposing the C functions, the D wrapper module importing these functions and
 * the D proxy code.
 *
 * The ctype typemap is used to determine the types to use in the C wrapper
 * functions. The types from the imtype typemap are used in the extern(C)
 * declarations of these functions in the wrap D module. The cstype typemap
 * contains the D types used in the D proxy module/class.
 */

%typemap(ctype) char *, char *&, char[ANY], char[]   "char *"
%typemap(imtype) char *, char *&, char[ANY], char[]   "char*"
%typemap(cstype) char *, char *&, char[ANY], char[]   "char[]"

// Non-primitive types.
%typemap(ctype) SWIGTYPE "void *"
%typemap(imtype) SWIGTYPE "void*"
%typemap(cstype) SWIGTYPE "$&dclassname"

%typemap(ctype) SWIGTYPE [] "void *"
%typemap(imtype) SWIGTYPE [] "void*"
%typemap(cstype) SWIGTYPE [] "$dclassname"

%typemap(ctype) SWIGTYPE * "void *"
%typemap(imtype) SWIGTYPE * "void*"
%typemap(cstype) SWIGTYPE * "$dclassname"

%typemap(ctype) SWIGTYPE & "void *"
%typemap(imtype) SWIGTYPE & "void*"
%typemap(cstype) SWIGTYPE & "$dclassname"


/*
 * The following typemaps are used for converting between the types for C/C++
 * and D when generating the code for the wrapper functions (on the C++ side):
 * the code from the in typemap is used to convert arguments to the C wrapper
 * function to the type used in the wrapped code (ctype->original c++ type), the
 * out typemap is utilized to convert values from the wrapped code to wrapper
 * function return types (original c++ type->ctype).
 */


/* char * - treat as char[] */
%typemap(in) char * %{ $1 = ($1_ltype)$input; %}
%typemap(out) char * %{ $result = SWIG_d_string_callback((const char *)$1); %}
%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) char * %{ $result = ($1_ltype)$input; %}
%typemap(directorin) char * %{ $input = SWIG_d_string_callback((const char *)$1); %}
%typemap(csdirectorin) char * "tango.stdc.stringz.fromStringz($iminput)"
%typemap(csdirectorout) char * "tango.stdc.stringz.toStringz($cscall)"

/* char *& - treat as char[] */
%typemap(in) char *& ($*1_ltype temp = 0) %{
  temp = ($*1_ltype)$input;
  $1 = &temp;
%}
%typemap(out) char *& %{ if ($1) $result = SWIG_d_string_callback((const char *)*$1); %}

%typemap(out, null="") void ""
%typemap(csdirectorin) void "$iminput"
%typemap(csdirectorout) void "$cscall"
%typemap(directorin) void ""


// Default handling (yet to be ported to D).
// Object passed by value. Convert to a pointer.
%typemap(in, canthrow=1) SWIGTYPE ($&1_type argp)
%{ argp = ($&1_ltype)$input;
   if (!argp) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Attempt to dereference null $1_type");
     return $null;
   }
   $1 = *argp; %}

%typemap(directorout) SWIGTYPE
%{ if (!$input) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Unexpected null return for type $1_type");
     return $null;
   }
   $result = *($&1_ltype)$input; %}

%typemap(out) SWIGTYPE
#ifdef __cplusplus
%{ $result = new $1_ltype((const $1_ltype &)$1); %}
#else
{
  $&1_ltype $1ptr = ($&1_ltype) malloc(sizeof($1_ltype));
  memmove($1ptr, &$1, sizeof($1_type));
  $result = $1ptr;
}
#endif

%typemap(directorin) SWIGTYPE
%{ $input = (void *)&$1; %}
%typemap(csdirectorin) SWIGTYPE "new $&dclassname($iminput, false)"
%typemap(csdirectorout) SWIGTYPE "$&dclassname.swigGetCObject($cscall)"

// Pointers and references.
%typemap(in) SWIGTYPE * %{ $1 = ($1_ltype)$input; %}
%typemap(in, canthrow=1) SWIGTYPE & %{ $1 = ($1_ltype)$input;
  if (!$1) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "$1_type type is null");
    return $null;
  } %}
%typemap(out) SWIGTYPE * %{ $result = (void *)$1; %}
%typemap(out) SWIGTYPE & %{ $result = (void *)$1; %}

%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE *
%{ $result = ($1_ltype)$input; %}

%typemap(directorin) SWIGTYPE *
%{ $input = (void *) $1; %}

%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE &
%{ if (!$input) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Unexpected null return for type $1_type");
     return $null;
   }
   $result = ($1_ltype)$input; %}
%typemap(directorin) SWIGTYPE &
%{ $input = ($1_ltype) &$1; %}

%typemap(csdirectorin) SWIGTYPE * "($iminput is null) ? null : new $dclassname($iminput, false)"
%typemap(csdirectorin) SWIGTYPE & "new $dclassname($iminput, false)"
%typemap(csdirectorout) SWIGTYPE *, SWIGTYPE & "$dclassname.swigGetCObject($cscall)"

// Default array handling.
%typemap(in) SWIGTYPE [] %{ $1 = ($1_ltype)$input; %}
%typemap(out) SWIGTYPE [] %{ $result = $1; %}

// Char arrays (yet to be ported to D).
%typemap(in) char[ANY], char[] %{ $1 = ($1_ltype)$input; %}
%typemap(out) char[ANY], char[] %{ $result = SWIG_d_string_callback((const char *)$1); %}

%typemap(directorout) char[ANY], char[] %{ $result = ($1_ltype)$input; %}
%typemap(directorin) char[ANY], char[] %{ $input = SWIG_d_string_callback((const char *)$1); %}

%typemap(csdirectorin) char[ANY], char[] "$iminput"
%typemap(csdirectorout) char[ANY], char[] "$cscall"


/*
 * Typecheck typemaps.
 *
 * Because, unlike many scripting languages, D does not need any dynamic dispatch
 * helper to acces an overloaded function, the purpose of these is merely to
 * issue a warning for overloaded C++ functions that cannot be overloaded in D
 * (as more than one C++ type maps to a single D type).
 */
%typecheck(SWIG_TYPECHECK_BOOL)
    bool,
    const bool &
    ""

%typecheck(SWIG_TYPECHECK_CHAR)
    char,
    const char &
    ""

%typecheck(SWIG_TYPECHECK_INT8)
    signed char,
    const signed char &
    ""

%typecheck(SWIG_TYPECHECK_UINT8)
    unsigned char,
    const unsigned char &
    ""

%typecheck(SWIG_TYPECHECK_INT16)
    short,
    const short &
    ""

%typecheck(SWIG_TYPECHECK_UINT16)
    unsigned short,
    const unsigned short &
    ""

%typecheck(SWIG_TYPECHECK_INT32)
    int,
    long,
    const int &,
    const long &
    ""

%typecheck(SWIG_TYPECHECK_UINT32)
    unsigned int,
    unsigned long,
    const unsigned int &,
    const unsigned long &
    ""

%typecheck(SWIG_TYPECHECK_INT64)
    long long,
    const long long &
    ""

%typecheck(SWIG_TYPECHECK_UINT64)
    unsigned long long,
    const unsigned long long &
    ""

%typecheck(SWIG_TYPECHECK_FLOAT)
    float,
    const float &
    ""

%typecheck(SWIG_TYPECHECK_DOUBLE)
    double,
    const double &
    ""

%typecheck(SWIG_TYPECHECK_STRING)
    char *,
    char *&,
    char[ANY],
    char[]
    ""

%typecheck(SWIG_TYPECHECK_POINTER)
    SWIGTYPE,
    SWIGTYPE *,
    SWIGTYPE &,
    SWIGTYPE [],
    SWIGTYPE (CLASS::*)
    ""

/*
 * Exception handling.
 */

%typemap(throws, canthrow=1) int,
                 long,
                 short,
                 unsigned int,
                 unsigned long,
                 unsigned short
%{ char error_msg[256];
   sprintf(error_msg, "C++ $1_type exception thrown, value: %d", $1);
   SWIG_DSetPendingException(SWIG_DException, error_msg);
   return $null; %}

%typemap(throws, canthrow=1) SWIGTYPE, SWIGTYPE &, SWIGTYPE *, SWIGTYPE [ANY]
%{ (void)$1;
   SWIG_DSetPendingException(SWIG_DException, "C++ $1_type exception thrown");
   return $null; %}

%typemap(throws, canthrow=1) char *
%{ SWIG_DSetPendingException(SWIG_DException, $1);
   return $null; %}


/*
 * Typemaps for code generation in D proxy and type wrapper classes.
 */

// The csin typemap is used for converting function parameter types from the
// type used in the proxy module or class to the type used in the D wrap module.
%typemap(csin)   bool,               const bool &,
                 char,               const char &,
                 signed char,        const signed char &,
                 unsigned char,      const unsigned char &,
                 short,              const short &,
                 unsigned short,     const unsigned short &,
                 int,                const int &,
                 unsigned int,       const unsigned int &,
                 long,               const long &,
                 unsigned long,      const unsigned long &,
                 size_t,             const size_t &,
                 long long,          const long long &,
                 unsigned long long, const unsigned long long &,
                 float,              const float &,
                 double,             const double &
    "$csinput"
%typemap(csin) char *, char *&, char[ANY], char[] "tango.stdc.stringz.toStringz($csinput)"
%typemap(csin) SWIGTYPE "$&dclassname.swigGetCObject($csinput)"
%typemap(csin) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] "$dclassname.swigGetCObject($csinput)"


// The csout typemap is used for converting function return values from the
// return type used in the wrap D module to the type returned by the proxy
// function.
// The $excode special variable is replaced by the excode typemap attribute code
// if the method can throw any exceptions from unmanaged code, otherwise
// replaced by nothing (not yet ported).

// Code which is inserted via excode if exceptions can be thrown.
%define SWIGEXCODE "\n  if ($wrapdmodule.SwigPendingException.isPending) throw $wrapdmodule.SwigPendingException.retrieve();" %enddef

%typemap(csout, excode=SWIGEXCODE) bool,               const bool &               {
  bool ret = $imcall ? true : false;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) char,               const char &               {
  char ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) signed char,        const signed char &        {
  byte ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) unsigned char,      const unsigned char &      {
  ubyte ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) short,              const short &              {
  short ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) unsigned short,     const unsigned short &     {
  ushort ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) int,                const int &                {
  int ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) unsigned int,       const unsigned int &       {
  uint ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) long,               const long &               {
  int ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) unsigned long,      const unsigned long &      {
  uint ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) size_t,             const size_t &             {
  size_t ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) long long,          const long long &          {
  long ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) unsigned long long, const unsigned long long & {
  ulong ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) float,              const float &              {
  float ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) double,             const double &             {
  double ret = $imcall;$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) char *, char *&, char[ANY], char[] {
  char[] ret = tango.stdc.stringz.fromStringz($imcall);$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) void {
  $imcall;$excode
}
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE {
  $&dclassname ret = new $&dclassname($imcall, true);$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE & {
  $dclassname ret = new $dclassname($imcall, $owner);$excode
  return ret;
}
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE *, SWIGTYPE [] {
  void* cPtr = $imcall;
  $dclassname ret = (cPtr is null) ? null : new $dclassname(cPtr, $owner);$excode
  return ret;
}


/*
 * Pointer reference typemaps.
 */
%typemap(ctype) SWIGTYPE *& "void *"
%typemap(imtype) SWIGTYPE *& "void *"
%typemap(cstype) SWIGTYPE *& "$*dclassname"

%typemap(csin) SWIGTYPE *& "$*dclassname.swigGetCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE *& {
    void* cPtr = $imcall;
    $*dclassname ret = (cPtr is null) ? null : new $*dclassname(cPtr, $owner);$excode
    return ret;
  }
%typemap(in) SWIGTYPE *& ($*1_ltype temp = 0)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}
%typemap(out) SWIGTYPE *&
%{ $result = (void *)*$1; %}


/*
 * »Aliases« (applys).
 */

// Treat references to arrays like like references to a single element.
%apply SWIGTYPE & { SWIGTYPE ((&)[ANY]) }

// RESEARCH: Is anything similar needed for D?
// /* Marshal C/C++ pointer to IntPtr */
// %typemap(ctype) void *VOID_INT_PTR "void *"
// %typemap(imtype) void *VOID_INT_PTR "IntPtr"
// %typemap(cstype) void *VOID_INT_PTR "IntPtr"
// %typemap(in) void *VOID_INT_PTR %{ $1 = ($1_ltype)$input; %}
// %typemap(out) void *VOID_INT_PTR %{ $result = (void *)$1; %}
// %typemap(csin) void *VOID_INT_PTR "$csinput"
// %typemap(csout, excode=SWIGEXCODE) void *VOID_INT_PTR {
//     IntPtr ret = $imcall;$excode
//     return ret;
//   }


/*
 * Typemaps used for the generation of proxy and type wrapper class code.
 */

// General.
%typemap(csbase)                      SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] ""
%typemap(dclassmodifiers)             SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] "class"
%typemap(dcode)                       SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] ""
%typemap(dimports)                    SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] ""
%typemap(dinterfaces)                 SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] ""
%typemap(dinterfaces_derived)         SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] ""

%typemap(dconstructor, excode=SWIGEXCODE,directorconnect="\n  swigDirectorConnect();") SWIGTYPE {
  this($imcall, true);$excode$directorconnect
}

%typemap(ddestructor) SWIGTYPE %{
~this() {
  dispose();
}
%}

// Do not use »override« attribute here to stay somewhat compatible to Phobos
// and older Tango versions.
%typemap(ddispose, methodname="dispose", methodmodifiers="public") SWIGTYPE {
  synchronized(this) {
    if (m_swigCObject !is null) {
      if (m_swigOwnCObject) {
        m_swigOwnCObject = false;
        $imcall;
      }
      m_swigCObject = null;
    }
  }
}

%typemap(ddispose_derived, methodname="dispose", methodmodifiers="public") SWIGTYPE {
  synchronized(this) {
    if (m_swigCObject !is null) {
      if (m_swigOwnCObject) {
        m_swigOwnCObject = false;
        $imcall;
      }
      m_swigCObject = null;
    }
    super.dispose();
  }
}

%typemap(directordisconnect, methodname="swigDirectorDisconnect") SWIGTYPE %{
protected void $methodname() {
  swigCMemOwn = false;
  $imcall;
}
%}

// Proxy base classes (not derived from another class).
// TODO: How much visibility is needed for swigGetCObject?
%typemap(dbody) SWIGTYPE %{
private void* m_swigCObject;
protected bool m_swigOwnCObject;

protected this(void* cObject, bool ownCObject) {
  m_swigCObject = cObject;
  m_swigOwnCObject = ownCObject;
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}

// Derived proxy classes.
%typemap(dbody_derived) SWIGTYPE %{
private void* m_swigCObject;

protected this(void* cObject, bool ownCObject) {
  super($wrapdmodule.$dclassnameUpcast(cObject), ownCObject);
  m_swigCObject = cObject;
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}

// Typewrapper classes.
// TODO: How much visibility is needed for the two-arguments constructor?
%typemap(dbody) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] %{
private void* m_swigCObject;

public this(void* cObject, bool futureUse) {
  m_swigCObject = cObject;
}

protected this() {
  m_swigCObject = null;
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}


/*
 * D-specific directives.
 */
#define %dconst(flag)               %feature("d:const","flag")
#define %dconstvalue(value)         %feature("d:constvalue",value)
#define %dmethodmodifiers           %feature("d:methodmodifiers")
#define %dnothrowexception          %feature("except")

%pragma(d) proxydmoduleimports = "static import tango.stdc.stringz;";


/*
 * Include helper files.
 */

// List of all reserved D keywords.
%include <dkeywords.swg>

// Wrapper loader code for dynamically linking the C wrapper library from the D
// wrapper module.
%include <wrapperloader.swg>

// Helper code for the strings and exceptions (currently unused, not yet ported to D).
%include <dhead.swg>

// Typemaps for member function pointers.
%include <dmemberfunctionpointers.swg>

// Typemaps for primitive types.
%include <dprimitives.swg>

// Typemaps for enumeration types.
%include <denums.swg>
