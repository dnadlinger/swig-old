//
// SWIG typemaps for STL - common utilities
// Luigi Ballabio
// Aug 3, 2002
//
// Guile implementation

%apply size_t { std::size_t };

#define SWIG_bool2scm(b) gh_bool2scm(b ? 1 : 0)
#define SWIG_string2scm(s) gh_str02scm(s.c_str())

%{
#include <string>

inline std::string SWIG_scm2string(SCM x) {
    char* temp;
    temp = SWIG_scm2str(x);
    std::string s(temp);
    if (temp) SWIG_free(temp);
    return s;
}
%}
