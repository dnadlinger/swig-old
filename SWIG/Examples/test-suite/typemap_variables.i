%module typemap_variables

// Check typemap name matching rules for variables
// Some of these are using qualified names, which is not right... the test will be adjusted as these get fixed

// Scripting languages use varin/varout for variables (except non-static member variables where in/out are used ???)
%typemap(varin)  int                           "this_will_not_compile_varin "
%typemap(varout) int                           "this_will_not_compile_varout"
%typemap(varin)  int globul                    "/*int globul varin */"
%typemap(varout) int globul                    "/*int globul varout*/"
%typemap(varin)  int Space::nspace             "/*int nspace varin */"
%typemap(varout) int Space::nspace             "/*int nspace varout*/"
//%typemap(varin)  int member                    "/*int member varin */"
//%typemap(varout) int member                    "/*int member varout*/"
%typemap(varin)  int Space::Struct::smember    "/*int smember varin */"
%typemap(varout) int Space::Struct::smember    "/*int smember varout*/"

// Statically typed languages use in/out for variables
%typemap(in)  int                           "this_will_not_compile_in "
%typemap(out) int                           "this_will_not_compile_out"
%typemap(in)  int globul                    "/*int globul in */ $1=0;"
%typemap(out) int globul                    "/*int globul out*/"
%typemap(in)  int Space::nspace             "/*int nspace in */ $1=0;"
%typemap(out) int Space::nspace             "/*int nspace out*/"
%typemap(in)  int member                    "/*int member in */ $1=0;"
%typemap(out) int member                    "/*int member out*/"
%typemap(in)  int Space::Struct::smember    "/*int smember in */ $1=0;"
%typemap(out) int Space::Struct::smember    "/*int smember out*/"

%typemap(javain)  int                           "this_will_not_compile_javain "
%typemap(javaout) int                           "this_will_not_compile_javaout"
%typemap(javain)  int globul                    "/*int globul in */  $javainput"
%typemap(javaout) int globul                    "/*int globul out*/  { return $jnicall; }"
%typemap(javain)  int Space::nspace             "/*int nspace in */  $javainput"
%typemap(javaout) int Space::nspace             "/*int nspace out*/  { return $jnicall; }"
%typemap(javain)  int member                    "/*int member in */  $javainput"
%typemap(javaout) int member                    "/*int member out*/  { return $jnicall; }"
%typemap(javain)  int Space::Struct::smember    "/*int smember in */ $javainput"
%typemap(javaout) int Space::Struct::smember    "/*int smember out*/ { return $jnicall; }"

%inline %{

int globul;

namespace Space {
  int nspace;
  struct Struct {
    int member;
    static int smember;
//    static short memberfunction() { return 0; } //javaout and jstype typemaps don't use fully qualified name, but other typemaps do
  };
  int Struct::smember = 0;
}
%}

