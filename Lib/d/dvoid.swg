/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dvoid.swg
 *
 * Typemaps for handling void function return types and empty parameter lists.
 * ----------------------------------------------------------------------------- */

%typemap(out, null="") void ""
%typemap(csdirectorin) void "$iminput"
%typemap(csdirectorout) void "$cscall"
%typemap(directorin) void ""

%typemap(csout, excode=SWIGEXCODE) void {
  $imcall;$excode
}
