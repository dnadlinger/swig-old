//
// SWIG typemaps for std::string
// Luigi Ballabio, Tal Shalif and William Fulton
// May 7, 2002
//
// Java implementation
//
/* ------------------------------------------------------------------------
  Typemaps for std::string and const std::string&
  These are mapped to a Java String and are passed around by value.

  To use non-const std::string references use the following %apply.  Note 
  that they are passed by value.
  %apply const std::string & {std::string &};
  ------------------------------------------------------------------------ */

%include exception.i

%{
#include <string>
%}

namespace std {

class string;

// string
%typemap(jni) string "jstring"
%typemap(jtype) string "String"
%typemap(jstype) string "String"
%typemap(javadirectorin) string "$jniinput"
%typemap(javadirectorout) string "$javacall"

%typemap(in) string 
%{if($input) {
    const char *pstr = (const char *)jenv->GetStringUTFChars($input, 0); 
    if (!pstr) return $null;
    $1 =  std::string(pstr);
    jenv->ReleaseStringUTFChars($input, pstr);
  } 
  else {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null std::string");
    return $null;
  } %}

%typemap(directorin,descriptor="Ljava/lang/String;") string 
%{ $input = jenv->NewStringUTF($1.c_str()); %}

%typemap(out) string 
%{ $result = jenv->NewStringUTF($1.c_str()); %}

%typemap(javain) string "$javainput"

%typemap(javaout) string {
    return $jnicall;
  }

%typemap(typecheck) string = char *;

// const string &
%typemap(jni) const string & "jstring"
%typemap(jtype) const string & "String"
%typemap(jstype) const string & "String"
%typemap(javadirectorin) const string & "$jniinput"
%typemap(javadirectorout) const string & "$javacall"

%typemap(in) const string & 
%{$1 = NULL;
  if($input) {
    const char *pstr = (const char *)jenv->GetStringUTFChars($input, 0); 
    if (!pstr) return $null;
    $1 =  new std::string(pstr);
    jenv->ReleaseStringUTFChars($input, pstr);
  } 
  else {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null std::string");
    return $null;
  } %}

%typemap(freearg) const string & 
%{ delete $1; %}

%typemap(directorin,descriptor="Ljava/lang/String;") const string &
%{ $input = jenv->NewStringUTF($1.c_str()); %}

%typemap(out) const string & 
%{ $result = jenv->NewStringUTF($1->c_str()); %}

%typemap(javain) const string & "$javainput"

%typemap(javaout) const string & {
    return $jnicall;
  }

%typemap(typecheck) const string & = char *;

}

/* ------------------------------------------------------------------------
  Typemaps for std::wstring and const std::wstring&

  These are mapped to a Java String and are passed around by value.
  Warning: Unicode / multibyte characters are handled differently on different 
  OSs so the std::wstring typemaps may not always work as intended. Therefore 
  a #define is required to use them.

  To use non-const std::wstring references use the following %apply.  Note 
  that they are passed by value.
  %apply const std::wstring & {std::wstring &};
  ------------------------------------------------------------------------ */

#ifdef SWIGJAVA_WSTRING

namespace std {

class wstring;

// wstring
%typemap(jni) wstring "jstring"
%typemap(jtype) wstring "String"
%typemap(jstype) wstring "String"
%typemap(javadirectorin) wstring "$jniinput"
%typemap(javadirectorout) wstring "$javacall"

%typemap(in) wstring
%{if($input) {
    const jchar *pstr = jenv->GetStringChars($input, 0);
    if (!pstr) return $null;
    jsize len = jenv->GetStringLength($input);
    if (len) {
      wchar_t *conv_buf = new wchar_t[len];
      for (jsize i = 0; i < len; ++i) {
         conv_buf[i] = pstr[i];
       }
       $1 =  std::wstring(conv_buf, len);
       delete [] conv_buf;
    }
    jenv->ReleaseStringChars($input, pstr);
  } 
  else {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null std::wstring");
    return $null;
  } %}

%typemap(directorin,descriptor="Ljava/lang/String;") wstring 
%{jsize len = $1.length();
  jchar *conv_buf = new jchar[len];
  for (jsize i = 0; i < len; ++i) {
    conv_buf[i] = (jchar)$1[i];
  }
  $input = jenv->NewString(conv_buf, len);
  delete [] conv_buf; %}

%typemap(out) wstring
%{jsize len = $1.length();
  jchar *conv_buf = new jchar[len];
  for (jsize i = 0; i < len; ++i) {
    conv_buf[i] = (jchar)$1[i];
  }
  $result = jenv->NewString(conv_buf, len);
  delete [] conv_buf; %}

%typemap(javain) wstring "$javainput"

%typemap(javaout) wstring {
    return $jnicall;
  }

// const wstring &
%typemap(jni) const wstring & "jstring"
%typemap(jtype) const wstring & "String"
%typemap(jstype) const wstring & "String"
%typemap(javadirectorin) const wstring & "$jniinput"
%typemap(javadirectorout) const wstring & "$javacall"

%typemap(in) const wstring & 
%{$1 = NULL;
  if($input) {
    const jchar *pstr = jenv->GetStringChars($input, 0);
    if (!pstr) return $null;
    jsize len = jenv->GetStringLength($input);
    if (len) {
      wchar_t *conv_buf = new wchar_t[len];
      for (jsize i = 0; i < len; ++i) {
         conv_buf[i] = pstr[i];
       }
       $1 =  new std::wstring(conv_buf, len);
       delete [] conv_buf;
    }
    jenv->ReleaseStringChars($input, pstr);
  } 
  else {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null std::wstring");
    return $null;
  } %}

%typemap(directorin,descriptor="Ljava/lang/String;") const wstring &
%{jsize len = $1->length();
  jchar *conv_buf = new jchar[len];
  for (jsize i = 0; i < len; ++i) {
    conv_buf[i] = (jchar)(*$1)[i];
  }
  $input = jenv->NewString(conv_buf, len);
  delete [] conv_buf; %}

%typemap(out) const wstring & 
%{jsize len = $1->length();
  jchar *conv_buf = new jchar[len];
  for (jsize i = 0; i < len; ++i) {
    conv_buf[i] = (jchar)(*$1)[i];
  }
  $result = jenv->NewString(conv_buf, len);
  delete [] conv_buf; %}

%typemap(javain) const wstring & "$javainput"

%typemap(javaout) const wstring & {
    return $jnicall;
  }

}

#endif

