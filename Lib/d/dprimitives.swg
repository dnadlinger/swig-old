/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dprimitves.swg
 *
 * Typemaps for primitive types.
 * ----------------------------------------------------------------------------- */

%typemap(cwtype) bool,               const bool &               "unsigned int"
%typemap(cwtype) char,               const char &               "char"
%typemap(cwtype) signed char,        const signed char &        "signed char"
%typemap(cwtype) unsigned char,      const unsigned char &      "unsigned char"
%typemap(cwtype) short,              const short &              "short"
%typemap(cwtype) unsigned short,     const unsigned short &     "unsigned short"
%typemap(cwtype) int,                const int &                "int"
%typemap(cwtype) unsigned int,       const unsigned int &       "unsigned int"
%typemap(cwtype) long,               const long &               "long"
%typemap(cwtype) unsigned long,      const unsigned long &      "unsigned long"
%typemap(cwtype) size_t,             const size_t &             "size_t"
%typemap(cwtype) long long,          const long long &          "long long"
%typemap(cwtype) unsigned long long, const unsigned long long & "unsigned long long"
%typemap(cwtype) float,              const float &              "float"
%typemap(cwtype) double,             const double &             "double"
%typemap(cwtype) long double,        const long double &        "long double"
%typemap(cwtype) void                                           "void"

%typemap(dwtype) bool,               const bool &                   "uint"
%typemap(dwtype) char,               const char &                   "char"
%typemap(dwtype) signed char,        const signed char &            "byte"
%typemap(dwtype) unsigned char,      const unsigned char &          "ubyte"
%typemap(dwtype) short,              const short &                  "short"
%typemap(dwtype) unsigned short,     const unsigned short &         "ushort"
%typemap(dwtype) int,                const int &                    "int"
%typemap(dwtype) unsigned int,       const unsigned int &           "uint"
%typemap(dwtype) long,               const long &                   "int"
%typemap(dwtype) unsigned long,      const unsigned long &          "uint"
%typemap(dwtype) size_t,             const size_t &                 "size_t"
%typemap(dwtype) long long,          const long long &              "long"
%typemap(dwtype) unsigned long long, const unsigned long long &     "ulong"
%typemap(dwtype) float,              const float &                  "float"
%typemap(dwtype) double,             const double &                 "double"
%typemap(dwtype) void                                               "void"

%typemap(dptype) bool,               const bool &                   "bool"
%typemap(dptype) char,               const char &                   "char"
%typemap(dptype) signed char,        const signed char &            "byte"
%typemap(dptype) unsigned char,      const unsigned char &          "ubyte"
%typemap(dptype) short,              const short &                  "short"
%typemap(dptype) unsigned short,     const unsigned short &         "ushort"
%typemap(dptype) int,                const int &                    "int"
%typemap(dptype) unsigned int,       const unsigned int &           "uint"
%typemap(dptype) long,               const long &                   "int"
%typemap(dptype) unsigned long,      const unsigned long &          "uint"
%typemap(dptype) size_t,             const size_t &                 "size_t"
%typemap(dptype) long long,          const long long &              "long"
%typemap(dptype) unsigned long long, const unsigned long long &     "ulong"
%typemap(dptype) float,              const float &                  "float"
%typemap(dptype) double,             const double &                 "double"
%typemap(dptype) void                                               "void"


/*
 * Conversion generation typemaps for by-value primitives.
 */

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
  "$1 = ($1_ltype)$input;"

%typemap(in) bool "$1 = $input ? true : false;"


%typemap(out) bool,
              char,
              signed char,
              unsigned char,
              short,
              unsigned short,
              int,
              unsigned int,
              long,
              size_t,
              long long,
              unsigned long long,
              float,
              double
  "$result = $1;"

// Judging from the history of the C# module, the explicit cast is needed for
// certain versions of VC++.
%typemap(out) unsigned long "$result = (unsigned long)$1;"


%typemap(directorin) bool,
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
  "$input = $1;"


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
  "$result = ($1_ltype)$input;"

%typemap(directorout) bool
  "$result = $input ? true : false;"


%typemap(ddirectorin) bool "($winput ? true : false)"
%typemap(ddirectorin) char,
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
  "$winput"


%typemap(ddirectorout) bool,
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
  "$dpcall"


/*
 * Conversion generation typemaps for (const) by-reference primitives.
 */

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

%typemap(in) const bool & ($*1_ltype temp)
%{ temp = $input ? true : false;
   $1 = &temp; %}


%typemap(out) const bool &,
              const char &,
              const signed char &,
              const unsigned char &,
              const short &,
              const unsigned short &,
              const int &,
              const unsigned int &,
              const long &,
              const size_t &,
              const long long &,
              const unsigned long long &,
              const float &,
              const double &
  "$result = *$1;"

// Judging from the history of the C# module, the explicit cast is needed for
// certain versions of VC++.
%typemap(out) const unsigned long &      %{ $result = (unsigned long)*$1; %}


%typemap(directorout,warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG)
             const char &,
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

%typemap(directorout,warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const bool &
%{ static $*1_ltype temp;
   temp = $input ? true : false;
   $result = &temp; %}


%typemap(directorin) const bool &,
		     const char &,
		     const signed char &,
		     const unsigned char &,
		     const short &,
		     const unsigned short &,
		     const int &,
		     const unsigned int &,
		     const long &,
		     const size_t &,
		     const long long &,
		     const unsigned long long &,
		     const float &,
		     const double &
  "$input = $1_name;"


%typemap(ddirectorin) const bool & "$winput"
%typemap(ddirectorin) const char & ($*1_ltype temp),
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
  "$winput"


%typemap(ddirectorout) const bool & "$dpcall"
%typemap(ddirectorout) const char & ($*1_ltype temp),
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
  "$dpcall"


/*
 * Conversion generation typemaps for both values and references.
 */

%typemap(din) bool,               const bool &,
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
  "$dinput"


%typemap(dout, excode=SWIGEXCODE) bool, const bool & {
  bool ret = $wcall ? true : false;$excode
  return ret;
}

%typemap(dout, excode=SWIGEXCODE) char,               const char &,
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
                                  double,             const double & {
  auto ret = $wcall;$excode
  return ret;
}


/*
 * Typecheck typemaps for both.
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
