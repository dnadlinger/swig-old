//
// SWIG typemaps for STL - common utilities
// Luigi Ballabio
// May 16, 2003
//
// Perl implementation

%apply size_t { std::size_t };

%{
#include <string>

double SwigSvToNumber(SV* sv) {
    return SvIOK(sv) ? double(SvIVX(sv)) : SvNVX(sv);
}
std::string SwigSvToString(SV* sv) {
    STRLEN len;
    return SvPV(sv,len);
}
void SwigSvFromString(SV* sv, const std::string& s) {
    sv_setpv(sv,s.c_str());
}
%}

