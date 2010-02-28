/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dvoid.swg
 *
 * Typemaps for handling void function return types and empty parameter lists.
 * ----------------------------------------------------------------------------- */

%typemap(cwtype) void "void"
%typemap(dwtype) void "void"
%typemap(dptype) void "void"

%typemap(out, null="") void ""
%typemap(ddirectorin) void "$winput"
%typemap(ddirectorout) void "$dpcall"
%typemap(directorin) void ""

%typemap(dout, excode=SWIGEXCODE) void {
  $wcall;$excode
}
