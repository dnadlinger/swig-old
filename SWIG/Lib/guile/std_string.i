//
// SWIG typemaps for std::string
// Luigi Ballabio
// Apr 8, 2002
//
// Guile implementation

// ------------------------------------------------------------------------
// std::string is typemapped by value
// This can prevent exporting methods which return a string
// in order for the user to modify it.
// However, I think I'll wait until someone asks for it...
// ------------------------------------------------------------------------

%include exception.i

%{
#include <string>
%}

namespace std {

    class string;

    %typemap(typecheck) string = char *;
    %typemap(typecheck) const string & = char *;

    %typemap(in) string (char* tempptr) {
        if (gh_string_p($input)) {
            tempptr = SWIG_scm2str($input);
            $1 = std::string(tempptr);
            if (tempptr) SWIG_free(tempptr);
        } else {
            SWIG_exception(SWIG_TypeError, "string expected");
        }
    }

    %typemap(in) const string & (std::string temp,
                                 char* tempptr) {
        if (gh_string_p($input)) {
            tempptr = SWIG_scm2str($input);
            temp = std::string(tempptr);
            if (tempptr) SWIG_free(tempptr);
            $1 = &temp;
        } else {
            SWIG_exception(SWIG_TypeError, "string expected");
        }
    }

    %typemap(out) string {
        $result = gh_str02scm($1.c_str());
    }

    %typemap(out) const string & {
        $result = gh_str02scm($1->c_str());
    }

}
