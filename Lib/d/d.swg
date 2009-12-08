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
 * functions. The types from the imtype typemap are used in the extern( C )
 * declarations of these functions in the wrap D module. The cstype typemap
 * contains the D types used in the D proxy module/class.
 */

// Primitive types.
%typemap(ctype) bool,               const bool &               "unsigned int"
%typemap(ctype) char,               const char &               "char"
%typemap(ctype) signed char,        const signed char &        "signed char"
%typemap(ctype) unsigned char,      const unsigned char &      "unsigned char"
%typemap(ctype) short,              const short &              "short"
%typemap(ctype) unsigned short,     const unsigned short &     "unsigned short"
%typemap(ctype) int,                const int &                "int"
%typemap(ctype) unsigned int,       const unsigned int &       "unsigned int"
%typemap(ctype) long,               const long &               "long"
%typemap(ctype) unsigned long,      const unsigned long &      "unsigned long"
%typemap(ctype) size_t,             const size_t &             "size_t"
%typemap(ctype) long long,          const long long &          "long long"
%typemap(ctype) unsigned long long, const unsigned long long & "unsigned long long"
%typemap(ctype) float,              const float &              "float"
%typemap(ctype) double,             const double &             "double"
%typemap(ctype) long double,        const long double &        "long double"
%typemap(ctype) void                                           "void"

%typemap(imtype) bool,               const bool &                   "uint"
%typemap(imtype) char,               const char &                   "char"
%typemap(imtype) signed char,        const signed char &            "byte"
%typemap(imtype) unsigned char,      const unsigned char &          "ubyte"
%typemap(imtype) short,              const short &                  "short"
%typemap(imtype) unsigned short,     const unsigned short &         "ushort"
%typemap(imtype) int,                const int &                    "int"
%typemap(imtype) unsigned int,       const unsigned int &           "uint"
%typemap(imtype) long,               const long &                   "int"
%typemap(imtype) unsigned long,      const unsigned long &          "uint"
%typemap(imtype) size_t,             const size_t &                 "size_t"
%typemap(imtype) long long,          const long long &              "long"
%typemap(imtype) unsigned long long, const unsigned long long &     "ulong"
%typemap(imtype) float,              const float &                  "float"
%typemap(imtype) double,             const double &                 "double"
%typemap(imtype) void                                               "void"

%typemap(cstype) bool,               const bool &                   "bool"
%typemap(cstype) char,               const char &                   "char"
%typemap(cstype) signed char,        const signed char &            "byte"
%typemap(cstype) unsigned char,      const unsigned char &          "ubyte"
%typemap(cstype) short,              const short &                  "short"
%typemap(cstype) unsigned short,     const unsigned short &         "ushort"
%typemap(cstype) int,                const int &                    "int"
%typemap(cstype) unsigned int,       const unsigned int &           "uint"
%typemap(cstype) long,               const long &                   "int"
%typemap(cstype) unsigned long,      const unsigned long &          "uint"
%typemap(cstype) size_t,             const size_t &                 "size_t"
%typemap(cstype) long long,          const long long &              "long"
%typemap(cstype) unsigned long long, const unsigned long long &     "ulong"
%typemap(cstype) float,              const float &                  "float"
%typemap(cstype) double,             const double &                 "double"
%typemap(cstype) void                                               "void"

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

// Pointer to a class member. This is really tricky, see below.
%typemap(ctype) SWIGTYPE (CLASS::*) "char *"
%typemap(imtype) SWIGTYPE (CLASS::*) "char*"
%typemap(cstype) SWIGTYPE (CLASS::*) "$dclassname"


/*
 * The following typemaps are used for converting between the types for C/C++
 * and D when generating the code for the wrapper functions (on the C++ side):
 * the code from the in typemap is used to convert arguments to the C wrapper
 * function to the type used in the wrapped code (ctype->original c++ type), the
 * out typemap is utilized to convert values from the wrapped code to wrapper
 * function return types (original c++ type->ctype).
 */

// Primitive types.
%typemap(in) bool
%{ $1 = $input ? true : false; %}

%typemap(in) char,
             signed char,
             unsigned char,
             short,
             unsigned short,
             int,
             unsigned int,
             long,
             unsigned long,
             size_t,
             long long,
             unsigned long long,
             float,
             double
%{ $1 = ($1_ltype)$input; %}

%typemap(directorout) bool
%{ $result = $input ? true : false; %}

%typemap(directorout) char,
             signed char,
             unsigned char,
             short,
             unsigned short,
             int,
             unsigned int,
             long,
             unsigned long,
             size_t,
             long long,
             unsigned long long,
             float,
             double
%{ $result = ($1_ltype)$input; %}

%typemap(directorin) bool               "$input = $1;"
%typemap(directorin) char               "$input = $1;"
%typemap(directorin) signed char        "$input = $1;"
%typemap(directorin) unsigned char      "$input = $1;"
%typemap(directorin) short              "$input = $1;"
%typemap(directorin) unsigned short     "$input = $1;"
%typemap(directorin) int                "$input = $1;"
%typemap(directorin) unsigned int       "$input = $1;"
%typemap(directorin) long               "$input = $1;"
%typemap(directorin) unsigned long      "$input = $1;"
%typemap(directorin) size_t             "$input = $1;"
%typemap(directorin) long long          "$input = $1;"
%typemap(directorin) unsigned long long "$input = $1;"
%typemap(directorin) float              "$input = $1;"
%typemap(directorin) double             "$input = $1;"

%typemap(csdirectorin) bool "($iminput ? true : false)"
%typemap(csdirectorin) char,
                       signed char,
                       unsigned char,
                       short,
                       unsigned short,
                       int,
                       unsigned int,
                       long,
                       unsigned long,
                       size_t,
                       long long,
                       unsigned long long,
                       float,
                       double
  "$iminput"

%typemap(csdirectorout) bool,
                        char,
                        signed char,
                        unsigned char,
                        short,
                        unsigned short,
                        int,
                        unsigned int,
                        long,
                        unsigned long,
                        size_t,
                        long long,
                        unsigned long long,
                        float,
                        double
  "$cscall"

%typemap(out) bool               %{ $result = $1; %}
%typemap(out) char               %{ $result = $1; %}
%typemap(out) signed char        %{ $result = $1; %}
%typemap(out) unsigned char      %{ $result = $1; %}
%typemap(out) short              %{ $result = $1; %}
%typemap(out) unsigned short     %{ $result = $1; %}
%typemap(out) int                %{ $result = $1; %}
%typemap(out) unsigned int       %{ $result = $1; %}
%typemap(out) long               %{ $result = $1; %}
%typemap(out) unsigned long      %{ $result = (unsigned long)$1; %}
%typemap(out) size_t             %{ $result = $1; %}
%typemap(out) long long          %{ $result = $1; %}
%typemap(out) unsigned long long %{ $result = $1; %}
%typemap(out) float              %{ $result = $1; %}
%typemap(out) double             %{ $result = $1; %}

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

// const reference to primitive types.
%typemap(in) const bool & ($*1_ltype temp)
%{ temp = $input ? true : false;
   $1 = &temp; %}

%typemap(directorout,warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const bool &
%{ static $*1_ltype temp;
   temp = $input ? true : false;
   $result = &temp; %}

%typemap(csdirectorin) const bool & "$iminput"
%typemap(csdirectorout) const bool & "$cscall"

%typemap(in) const char & ($*1_ltype temp),
             const signed char & ($*1_ltype temp),
             const unsigned char & ($*1_ltype temp),
             const short & ($*1_ltype temp),
             const unsigned short & ($*1_ltype temp),
             const int & ($*1_ltype temp),
             const unsigned int & ($*1_ltype temp),
             const long & ($*1_ltype temp),
             const unsigned long & ($*1_ltype temp),
             const size_t & ($*1_ltype temp),
             const long long & ($*1_ltype temp),
             const unsigned long long & ($*1_ltype temp),
             const float & ($*1_ltype temp),
             const double & ($*1_ltype temp)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}

%typemap(directorout,warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const char &,
             const signed char &,
             const unsigned char &,
             const short &,
             const unsigned short &,
             const int &,
             const unsigned int &,
             const long &,
             const unsigned long &,
             const size_t &,
             const long long &,
             const float &,
             const double &
%{ static $*1_ltype temp;
   temp = ($*1_ltype)$input;
   $result = &temp; %}

%typemap(directorin) const bool &           "$input = $1_name;"
%typemap(directorin) const char &           "$input = $1_name;"
%typemap(directorin) const signed char &    "$input = $1_name;"
%typemap(directorin) const unsigned char &  "$input = $1_name;"
%typemap(directorin) const short &          "$input = $1_name;"
%typemap(directorin) const unsigned short & "$input = $1_name;"
%typemap(directorin) const int &            "$input = $1_name;"
%typemap(directorin) const unsigned int &   "$input = $1_name;"
%typemap(directorin) const long &           "$input = $1_name;"
%typemap(directorin) const unsigned long &  "$input = $1_name;"
%typemap(directorin) const size_t &         "$input = $1_name;"
%typemap(directorin) const long long &      "$input = $1_name;"
%typemap(directorin) const float &          "$input = $1_name;"
%typemap(directorin) const double &         "$input = $1_name;"

%typemap(csdirectorin) const char & ($*1_ltype temp),
                       const signed char & ($*1_ltype temp),
                       const unsigned char & ($*1_ltype temp),
                       const short & ($*1_ltype temp),
                       const unsigned short & ($*1_ltype temp),
                       const int & ($*1_ltype temp),
                       const unsigned int & ($*1_ltype temp),
                       const long & ($*1_ltype temp),
                       const unsigned long & ($*1_ltype temp),
                       const size_t & ($*1_ltype temp),
                       const long long & ($*1_ltype temp),
                       const float & ($*1_ltype temp),
                       const double & ($*1_ltype temp)
  "$iminput"

%typemap(csdirectorout) const char & ($*1_ltype temp),
                        const signed char & ($*1_ltype temp),
                        const unsigned char & ($*1_ltype temp),
                        const short & ($*1_ltype temp),
                        const unsigned short & ($*1_ltype temp),
                        const int & ($*1_ltype temp),
                        const unsigned int & ($*1_ltype temp),
                        const long & ($*1_ltype temp),
                        const unsigned long & ($*1_ltype temp),
                        const size_t & ($*1_ltype temp),
                        const long long & ($*1_ltype temp),
                        const float & ($*1_ltype temp),
                        const double & ($*1_ltype temp)
  "$cscall"


%typemap(out) const bool &               %{ $result = *$1; %}
%typemap(out) const char &               %{ $result = *$1; %}
%typemap(out) const signed char &        %{ $result = *$1; %}
%typemap(out) const unsigned char &      %{ $result = *$1; %}
%typemap(out) const short &              %{ $result = *$1; %}
%typemap(out) const unsigned short &     %{ $result = *$1; %}
%typemap(out) const int &                %{ $result = *$1; %}
%typemap(out) const unsigned int &       %{ $result = *$1; %}
%typemap(out) const long &               %{ $result = *$1; %}
%typemap(out) const unsigned long &      %{ $result = (unsigned long)*$1; %}
%typemap(out) const size_t &             %{ $result = *$1; %}
%typemap(out) const long long &          %{ $result = *$1; %}
%typemap(out) const unsigned long long & %{ $result = *$1; %}
%typemap(out) const float &              %{ $result = *$1; %}
%typemap(out) const double &             %{ $result = *$1; %}

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
%typemap(csdirectorout) SWIGTYPE "$&dclassname.__swig_getCObject($cscall)"

// Pointers and references.
%typemap(in) SWIGTYPE * %{ $1 = ($1_ltype)$input; %}
%typemap(in, fragment="SWIG_UnPackData") SWIGTYPE (CLASS::*) %{
  SWIG_UnpackData($input, (void *)&$1, sizeof($1));
%}
%typemap(in, canthrow=1) SWIGTYPE & %{ $1 = ($1_ltype)$input;
  if (!$1) {
    SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "$1_type type is null");
    return $null;
  } %}
%typemap(out) SWIGTYPE * %{ $result = (void *)$1; %}
%typemap(out, fragment="SWIG_PackData") SWIGTYPE (CLASS::*) %{
  char buf[128];
  char *data = SWIG_PackData(buf, (void *)&$1, sizeof($1));
  *data = '\0';
  $result = SWIG_d_string_callback(buf);
%}
%typemap(out) SWIGTYPE & %{ $result = (void *)$1; %}

%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE *
%{ $result = ($1_ltype)$input; %}
%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE (CLASS::*)
%{ $result = ($1_ltype)$input; %}

%typemap(directorin) SWIGTYPE *
%{ $input = (void *) $1; %}
%typemap(directorin) SWIGTYPE (CLASS::*)
%{ $input = (void *) $1; %}

%typemap(directorout, warning=SWIGWARN_TYPEMAP_DIRECTOROUT_PTR_MSG) SWIGTYPE &
%{ if (!$input) {
     SWIG_DSetPendingException(SWIG_DIllegalArgumentException, "Unexpected null return for type $1_type");
     return $null;
   }
   $result = ($1_ltype)$input; %}
%typemap(directorin) SWIGTYPE &
%{ $input = ($1_ltype) &$1; %}

%typemap(csdirectorin) SWIGTYPE *, SWIGTYPE (CLASS::*) "($iminput is null) ? null : new $dclassname($iminput, false)"
%typemap(csdirectorin) SWIGTYPE & "new $dclassname($iminput, false)"
%typemap(csdirectorout) SWIGTYPE *, SWIGTYPE (CLASS::*), SWIGTYPE & "$dclassname.__swig_getCObject($cscall)"

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
%typemap(csin) SWIGTYPE "$&dclassname.__swig_getCObject($csinput)"
%typemap(csin) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] "$dclassname.__swig_getCObject($csinput)"
%typemap(csin) SWIGTYPE (CLASS::*) "$dclassname.__swig_getCMemberPtr($csinput)"


// The csout typemap is used for converting function return values from the
// return type used in the wrap D module to the type returned by the proxy
// function.
// The $excode special variable is replaced by the excode typemap attribute code
// if the method can throw any exceptions from unmanaged code, otherwise
// replaced by nothing (not yet ported).

// Code which is inserted via excode if exceptions can be thrown.
%define SWIGEXCODE "\n  if ($wrapdmodule.SwigPendingException.isPending) throw $wrapdmodule.SwigPendingException.retrieve();" %enddef

// Indented version of the above.
%define SWIGEXCODE2 "\n    if ($wrapdmodule.SwigPendingException.isPending) throw $wrapdmodule.SwigPendingException.retrieve();" %enddef

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
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE (CLASS::*) {
  char* cMemberPtr = $imcall;
  $dclassname ret = (cMemberPtr is null) ? null : new $dclassname(cMemberPtr, $owner);$excode
  return ret;
}


/*
 * Pointer reference typemaps.
 */
%typemap(ctype) SWIGTYPE *& "void *"
%typemap(imtype) SWIGTYPE *& "void *"
%typemap(cstype) SWIGTYPE *& "$*dclassname"

%typemap(csin) SWIGTYPE *& "$*dclassname.__swig_getCObject($csinput)"
%typemap(csout, excode=SWIGEXCODE) SWIGTYPE *& {
    void* cPtr = $imcall;
    $*dclassname ret = ( cPtr is null ) ? null : new $*dclassname( cPtr, $owner );$excode
    return ret;
  }
%typemap(in) SWIGTYPE *& ($*1_ltype temp = 0)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}
%typemap(out) SWIGTYPE *&
%{ $result = (void *)*$1; %}


/*
 * Enumerations.
 */
// const enum SWIGTYPE & typemaps
%typemap(ctype) const enum SWIGTYPE & "int"
%typemap(imtype) const enum SWIGTYPE & "int"
%typemap(cstype) const enum SWIGTYPE & "$*dclassname"

%typemap(in) const enum SWIGTYPE & ($*1_ltype temp)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}
%typemap(out) const enum SWIGTYPE & %{ $result = *$1; %}

%typemap(directorout,warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const enum SWIGTYPE &
%{ static $*1_ltype temp = ($*1_ltype)$input;
   $result = &temp; %}
%typemap(directorin) const enum SWIGTYPE & "$input = $1_name;"
%typemap(csdirectorin) const enum SWIGTYPE & "cast($*dclassname)$iminput"
%typemap(csdirectorout) const enum SWIGTYPE & "cast(int)$cscall"

%typecheck(SWIG_TYPECHECK_POINTER) const enum SWIGTYPE & ""

%typemap(throws, canthrow=1) const enum SWIGTYPE &
%{ (void)$1;
   SWIG_DSetPendingException(SWIG_DException, "C++ $1_type exception thrown");
   return $null; %}

%typemap(csin) const enum SWIGTYPE & "cast(int)$csinput"
%typemap(csout, excode=SWIGEXCODE) const enum SWIGTYPE & {
  $*dclassname ret = cast($*dclassname)$imcall;$excode
  return ret;
}

// enum SWIGTYPE typemaps
%typemap(ctype) enum SWIGTYPE "int"
%typemap(imtype) enum SWIGTYPE "int"
%typemap(cstype) enum SWIGTYPE "$dclassname"

%typemap(in) enum SWIGTYPE %{ $1 = ($1_ltype)$input; %}
%typemap(out) enum SWIGTYPE %{ $result = $1; %}

%typemap(directorout) enum SWIGTYPE %{ $result = ($1_ltype)$input; %}
%typemap(directorin) enum SWIGTYPE "$input = $1;"
%typemap(csdirectorin) enum SWIGTYPE "cast($dclassname)$iminput"
%typemap(csdirectorout) enum SWIGTYPE "cast(int)$cscall"

%typecheck(SWIG_TYPECHECK_POINTER) enum SWIGTYPE ""

%typemap(throws, canthrow=1) enum SWIGTYPE
%{ (void)$1;
   SWIG_DSetPendingException(SWIG_DException, "C++ $1_type exception thrown");
   return $null; %}

%typemap(csin) enum SWIGTYPE "cast(int)$csinput"
%typemap(csout, excode=SWIGEXCODE) enum SWIGTYPE {
  $dclassname ret = cast($dclassname)$imcall;$excode
  return ret;
}

%typemap(dclassmodifiers) enum SWIGTYPE "enum"
%typemap(dcode) enum SWIGTYPE ""

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
%typemap(csbase)                      SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dclassmodifiers)             SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) "class"
%typemap(dcode)                       SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dimports)                    SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dinterfaces)                 SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dinterfaces_derived)         SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""

%typemap(dconstructor, excode=SWIGEXCODE,directorconnect="\n    __SwigDirectorConnect();") SWIGTYPE %{{
    this( $imcall, true );$excode$directorconnect
  }
%}

%typemap(ddestructor) SWIGTYPE %{
  ~this() {
    dispose();
  }
%}

// Do not use »override« attribute here to stay somewhat compatible to Phobos
// and older Tango versions.
%typemap(ddispose, methodname="dispose", methodmodifiers="public") SWIGTYPE {
    synchronized( this ) {
      if ( __swig_cObject !is null ) {
        if ( __swig_ownCObject ) {
          __swig_ownCObject = false;
          $imcall;
        }
        __swig_cObject = null;
      }
    }
  }

%typemap(ddispose_derived, methodname="dispose", methodmodifiers="public") SWIGTYPE {
    synchronized( this ) {
      if ( __swig_cObject !is null ) {
        if ( __swig_ownCObject ) {
          __swig_ownCObject = false;
          $imcall;
        }
        __swig_cObject = null;
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
// TODO: How much visibility is needed for __swig_getCObject?
%typemap(dbody) SWIGTYPE %{
  private void* __swig_cObject;
  protected bool __swig_ownCObject;

  protected this( void* cObject, bool ownCObject ) {
    __swig_cObject = cObject;
    __swig_ownCObject = ownCObject;
  }

  public static void* __swig_getCObject( $dclassname obj ) {
    return ( obj is null ) ? null : obj.__swig_cObject;
  }
%}

// Derived proxy classes.
%typemap(dbody_derived) SWIGTYPE %{
  private void* __swig_cObject;

  protected this( void* cObject, bool ownCObject ) {
    super( $wrapdmodule.$dclassnameUpcast( cObject ), ownCObject );
    __swig_cObject = cObject;
  }

  public static void* __swig_getCObject( $dclassname obj ) {
    return ( obj is null ) ? null : obj.__swig_cObject;
  }
%}

// Typewrapper classes.
// TODO: How much visibility is needed for the two-arguments constructor?
%typemap(dbody) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] %{
  private void* __swig_cObject;

  public this( void* cObject, bool futureUse ) {
    __swig_cObject = cObject;
  }

  protected this() {
    __swig_cObject = null;
  }

  public static void* __swig_getCObject( $dclassname obj ) {
    return ( obj is null ) ? null : obj.__swig_cObject;
  }
%}

%typemap(dbody) SWIGTYPE (CLASS::*) %{
  private char* __swig_cMemberPtr;

  public this( char* cMemberPtr, bool futureUse ) {
    __swig_cMemberPtr = cMemberPtr;
  }

  protected this() {
    __swig_cMemberPtr = null;
  }

  public static char* __swig_getCMemberPtr( $dclassname obj ) {
    return ( obj is null ) ? null : obj.__swig_cMemberPtr;
  }
%}


/*
 * D-specific directives.
 */
#define %dconst(flag)               %feature("d:const","flag")
#define %dconstvalue(value)         %feature("d:constvalue",value)
#define %dmethodmodifiers           %feature("d:methodmodifiers")
#define %dnothrowexception          %feature("except")

%pragma(d) proxydmoduleimports=%{
static import tango.stdc.stringz;
%}

%pragma(d) wrapdmoduleimports=%{
%}


/*
 * Fragments.
 */
%fragment("SWIG_PackData", "header") {
/* Pack binary data into a string */
SWIGINTERN char * SWIG_PackData(char *c, void *ptr, size_t sz) {
  static const char hex[17] = "0123456789abcdef";
  register const unsigned char *u = (unsigned char *) ptr;
  register const unsigned char *eu =  u + sz;
  for (; u != eu; ++u) {
    register unsigned char uu = *u;
    *(c++) = hex[(uu & 0xf0) >> 4];
    *(c++) = hex[uu & 0xf];
  }
  return c;
}
}

%fragment("SWIG_UnPackData", "header") {
/* Unpack binary data from a string */
SWIGINTERN const char * SWIG_UnpackData(const char *c, void *ptr, size_t sz) {
  register unsigned char *u = (unsigned char *) ptr;
  register const unsigned char *eu = u + sz;
  for (; u != eu; ++u) {
    register char d = *(c++);
    register unsigned char uu;
    if ((d >= '0') && (d <= '9'))
      uu = ((d - '0') << 4);
    else if ((d >= 'a') && (d <= 'f'))
      uu = ((d - ('a'-10)) << 4);
    else
      return (char *) 0;
    d = *(c++);
    if ((d >= '0') && (d <= '9'))
      uu |= (d - '0');
    else if ((d >= 'a') && (d <= 'f'))
      uu |= (d - ('a'-10));
    else
      return (char *) 0;
    *u = uu;
  }
  return c;
}
}


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
