/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * ddirectives.swg
 *
 * D-specifiv directives.
 * ----------------------------------------------------------------------------- */

#define %dconst(flag)               %feature("d:const","flag")
#define %dconstvalue(value)         %feature("d:constvalue",value)
#define %dmethodmodifiers           %feature("d:methodmodifiers")
#define %dnothrowexception          %feature("except")
