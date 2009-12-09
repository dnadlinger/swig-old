/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dprimitves.swg
 *
 * Typemaps for primitive types.
 * ----------------------------------------------------------------------------- */

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


%typemap(csdirectorin) const bool & "$iminput"
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


%typemap(csdirectorout) const bool & "$cscall"
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
