%module typemap_namespace

/* Secret typedefs */
%{
namespace Foo {
   typedef char    Str1;
   typedef char    Str2;
}
%}

namespace Foo {
    struct Str1;
    struct Str2;

#ifdef SWIGJAVA
    %typemap(jni) Str1 * = char *;
    %typemap(jtype) Str1 * = char *;
    %typemap(jstype) Str1 * = char *;
    %typemap(freearg) Str1 * = char *;
    %typemap(javain) Str1 * = char *;
    %typemap(javaout) Str1 * = char *;
#endif
    %typemap(in) Str1 * = char *;
    %apply char * { Str2 * };
}

%inline %{
namespace Foo {
    char *test1(Str1 *s) {
          return s;
    }
    char *test2(Str2 *s) {
          return s;
    }
}
%}

    

