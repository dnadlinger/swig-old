/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * d.swg
 *
 * Main library file for the D language module.
 * ----------------------------------------------------------------------------- */

/*
 * ctype, imtype, cstype:
 * ----------------------
 * Mapping of types between the C/C++ library, the C/C++ library wrapper
 * exposing the C functions, the D wrapper module importing these functions and
 * the D proxy code.
 *
 * The ctype typemap is used to determine the types to use in the C wrapper
 * functions. The types from the imtype typemap are used in the extern(C)
 * declarations of these functions in the wrap D module. The cstype typemap
 * contains the D types used in the D proxy module/class.
 */

/*
 * in, out, directorin, directorout:
 * ---------------------------------
 * Used for converting between the types for C/C++ and D when generating the
 * code for the wrapper functions (on the C++ side).
 *
 * The code from the in typemap is used to convert arguments to the C wrapper
 * function to the type used in the wrapped code (ctype->original C++ type), the
 * out typemap is utilized to convert values from the wrapped code to wrapper
 * function return types (original C++ type->ctype).
 *
 * The directorin typemap is used to convert parameters to the type used in the
 * D director callback function, its return value is processed by directorout
 * (see below).
 */

/*
 * csin, csout, csdirectorin, csdirectorout:
 * -----------------------------------------
 * Typemaps for code generation in D proxy and type wrapper classes.
 *
 * The csin typemap is used for converting function parameter types from the
 * type used in the proxy module or class to the type used in the D wrap module.
 *
 * The csout typemap is used for converting function return values from the
 * return type used in the wrap D module to the type returned by the proxy
 * function. The $excode special variable in csout typemaps is replaced by the
 * excode typemap attribute code if the method can throw any exceptions from
 * unmanaged code, otherwise by nothing.
 *
 * The code from the csdirectorin and csdirectorout typemaps is used for
 * conversion in director callback functions. Arguments are converted to the
 * type used in the proxy class method they are calling by using the code from
 * csdirectorin, the proxy class method return value is converted to the type
 * the C++ code expects via the csdirectorout typemap.
 *
 * The full chain of type conversions when a director callback is invoked looks
 * like this:
 *
 *      type CPPClass::method(type a)
 *        ↑                     ↓
 *   <directorout>         <directorin>
 *        ↑                     ↓
 *      ctype methodCallback(ctype a)     C++
 * ::::::::::::::::::::::::::::::::::::::::::
 *     imtype methodCallback(imtype a)     D
 *        ↑                     ↓
 *  <csdirectorout>       <csdirectorin>
 *        ↑                     ↓
 *      cstype DClass.method(cstype a)
 */

/*
 * typecheck typemaps:
 * -------------------
 * Because, unlike many scripting languages, D does not need any dynamic
 * dispatch helper to acces an overloaded function, the purpose of these is
 * merely to issue a warning for overloaded C++ functions that cannot be
 * overloaded in D (as more than one C++ type maps to a single D type).
 */

/*
 * csbase, dinterfaces, dinterfaces_derived, dclassmodifiers, dcode, dimports,
 * dconstructor, ddestructor, ddispose, ddispose_derived, directordisconnect,
 * dbody, dbody_derived:
 * ---------------------------------------------------------------------------
 * These typemaps are used for generating the skeleton of proxy classes for C++
 * types.
 *
 * By overriding csbase, dinterfaces or dinterfaces_derived, the inheritance
 * chain of the generated proxy class for a type can be modified. dclassmodifiers
 * allows you to add any custom modifiers around the class keyword.
 *
 * Using dcode and dimports, you can specify additional D code which will be
 * emitted into the class body respectively the imports section of the D module
 * the class is written to.
 *
 * dconstructor, ddestructor, ddispose and ddispose_derived are used to generate
 * the class constructor, destructor and dispose() method, respectively. The
 * auxiliary code for handling the pointer to the C++ object is stored in dbody
 * and dbody_derived. You can override them for specific types.
 */

// -----------------------------------------------------------------------------

// Typemaps for exception handling.
%include <dexception.swg>

// Typemaps for primitive types.
%include <dprimitives.swg>

// Typemaps for non-primitive types (C/C++ classes and structs).
%include <dclasses.swg>

// Typemaps for enumeration types.
%include <denums.swg>

// Typemaps for member function pointers.
%include <dmemberfunctionpointers.swg>

// Typemaps for wrapping pointers to/arrays of C chars as D strings.
%include <dstrings.swg>

// Typemaps for handling void function return types and empty parameter lists.
%include <dvoid.swg>

// Typemaps containing D code used when generating D proxy classes.
%include <dclassgen.swg>

// Mapping of C++ operator overloading methods to D.
%include <doperators.swg>

// Helper code string and exception handling.
%include <dhead.swg>

// Wrapper loader code for dynamically linking the C wrapper library from the D
// wrapper module.
%include <wrapperloader.swg>

// List of all reserved D keywords.
%include <dkw.swg>

// D-specific directives.
%include <ddirectives.swg>
