// Massive primitive datatype test.
%module(directors="1") primitive_types

 /*

 if your language has problems with MyInt* and/or Hello*,
 you need to change the constant reference typemaps from something
 like:

 %typemap(in) const char & (char temp), 
             const signed char & (signed char temp), 
             const unsigned char & (unsigned char temp), 
             const short & (short temp), 
             const unsigned short & (unsigned short temp), 
             const int & (int temp), 
             const unsigned int & (unsigned int temp), 
             const long & (long temp), 
             const unsigned long & (unsigned long temp), 
             const long long & ($*1_ltype temp), 
             const float & (float temp), 
             const double & (double temp)
  %{ temp = ($*1_ltype)$input;  $1 = &temp; %}

  to the following:

  %typemap(in) const char & ($basetype temp), 
             const signed char & ($basetype temp), 
             const unsigned char & ($basetype temp), 
             const short & ($basetype temp), 
             const unsigned short & ($basetype temp), 
             const int & ($basetype temp), 
             const unsigned int & ($basetype temp), 
             const long & ($basetype temp), 
             const unsigned long & ($basetype temp), 
             const long long & ($basetype temp), 
             const float & ($basetype temp), 
             const double & ($basetype temp)
  %{ temp = ($basetype)$input;  $1 = &temp; %}

  the other tipical change is to add the enum SWIGTYPE to the
  integer throws typemaps:

  %typemap(throws) int, 
                  long, 
                  short, 
                  unsigned int, 
                  unsigned long, 
                  unsigned short,
                  enum SWIGTYPE {
    Tcl_SetObjResult(interp, Tcl_NewIntObj((long) $1));
    SWIG_fail;
  }

  or just add the %apply directive after all the typemaps declaration

  %apply int { enum SWIGTYPE };


  Also note that this test should not only compile, if you run the
  program

     grep 'resultobj = SWIG_NewPointerObj' primitive_types_wrap.cxx 
 
  you should get only two calls:

    resultobj = SWIG_NewPointerObj((void *) result, SWIGTYPE_p_Test, 1);
    resultobj = SWIG_NewPointerObj((void *) result, SWIGTYPE_p_TestDirector, 1);

  if you get a lot more, some typemap could be not defined.

  The same with

     grep SWIG_ConvertPtr primitive_types_wrap.cxx| egrep -v 'Test'

  you should only get

    #define SWIG_ConvertPtr(obj, pp, type, flags)

 */

//
// Try your language module with and without 
// these nowarn flags.
//
#pragma SWIG nowarn=451
 //#pragma SWIG nowarn=515
 //#pragma SWIG nowarn=509

%{
#include <stddef.h>
#include <iostream>
#include <sstream>
%}

%feature("director") TestDirector;

%{
  // Integer class, only visible in C++
  struct MyInt
  {
    char name[5];
    int val;

    MyInt(int v = 0): val(v) {
    }
    
    operator int() const { return val; }
  };

  // Template primitive type, only visible in C++
  template <class T>
  struct Param
  {
    char name[5];
    T val;

    Param(T v = 0): val(v) {
    }
    
    operator T() const { return val; }
  };

  typedef char namet[5];
  extern namet gbl_namet;
  namet gbl_namet;

%}


//
// adding applies for incomplete swig type MyInt
//
%apply int { MyInt };
%apply const int& {  const MyInt& };
%apply int { Param<int> };
%apply char { Param<char> };
%apply float { Param<float> };
%apply double { Param<double> };
%apply const int&  { const Param<int>& };
%apply const char&  { const Param<char>& };
%apply const float&  { const Param<float>& };
%apply const double&  { const Param<double>& };



//
// These applies shouldn't be needed ....!!
//
%apply const int& { const Hello&  };

%apply long { pint };
%apply const long& { const pint& };
  

//
// Some simple types
%inline %{
  enum Hello {
    Hi, Hola
  };

  typedef char namet[5];
  typedef char* pchar;
  typedef const char* pcharc;
  typedef int* pint;

  char* const def_pchar = (char *const)"hello";
  const char* const def_pcharc = "hija";

  const namet def_namet = {'h','o',0, 'l','a'};

  extern namet gbl_namet;
  
%}


/* all the primitive types */
#define def_bool 1
#define def_schar 1
#define def_uchar 1
#define def_int 1
#define def_uint 1
#define def_short 1
#define def_ushort 1
#define def_long 1
#define def_ulong 1
#define def_llong 1
#define def_ullong 1
#define def_float 1
#define def_double 1
#define def_char 'H'
#define def_pint  0
#define def_sizet 1
#define def_hello Hola
#define def_myint 1
#define def_parami 1
#define def_paramd 1
#define def_paramc 'c'

/* types that can be declared as static const class members */
%define %test_prim_types_stc(macro, pfx)
macro(bool,               pfx, bool)
macro(signed char,        pfx, schar)
macro(unsigned char,      pfx, uchar)
macro(int,                pfx, int)
macro(unsigned int,       pfx, uint)
macro(short,              pfx, short)
macro(unsigned short,     pfx, ushort)
macro(long,               pfx, long)
macro(unsigned long,      pfx, ulong)
macro(long long,          pfx, llong)
macro(unsigned long long, pfx, ullong)
//macro(float,              pfx, float)
//macro(double,             pfx, double)
macro(char,               pfx, char)
%enddef


/* types that can be used to test overloading */
%define %test_prim_types_ovr(macro, pfx)
%test_prim_types_stc(macro, pfx)
macro(pchar,              pfx, pchar)
%enddef

/* all the types */
%define %test_prim_types(macro, pfx)
%test_prim_types_ovr(macro, pfx)
macro(pcharc,             pfx, pcharc)
macro(pint,               pfx, pint)
/* these ones should behave like primitive types too */
macro(Hello,              pfx, hello)
macro(MyInt,              pfx, myint)
macro(Param<int>,         pfx, parami)
macro(Param<double>,      pfx, paramd)
macro(Param<char>,        pfx, paramc)
macro(size_t,             pfx, sizet)
%enddef


/* function passing by value */
%define val_decl(type, pfx, name)
  type pfx##_##name(type x) throw (type) { return x; }
%enddef
/* function passing by ref */
%define ref_decl(type, pfx, name)
  const type& pfx##_##name(const type& x) throw (type) { return x; }
%enddef

/* C++ constant declaration */
%define cct_decl(type, pfx, name)
  const type pfx##_##name = def##_##name;
%enddef

/* C++ static constant declaration */
%define stc_decl(type, pfx, name)
  static const type pfx##_##name = def##_##name;
%enddef

/* Swig constant declaration */
%define sct_decl(type, pfx, name)
  %constant type pfx##_##name = def##_##name;
%enddef

/* variable delaration */
%define var_decl(type, pfx, name)
  type pfx##_##name;
%enddef

/* virtual function passing by value */
%define vval_decl(type, pfx, name)
  virtual val_decl(type, pfx, name)
%enddef
/* virtual function passing by ref */
%define vref_decl(type, pfx, name)
  virtual ref_decl(type, pfx, name)
%enddef


%test_prim_types(sct_decl, sct)

%inline {
  %test_prim_types(val_decl, val)
  %test_prim_types(ref_decl, ref)
  %test_prim_types(cct_decl, cct)
  %test_prim_types(var_decl, var)

  var_decl(namet, var, namet)

  void var_init() 
  {
    var_pchar = 0;
    var_pcharc = 0;
    var_pint = 0;
    var_namet[0] = 'h';
  }
  
}

/* check variables */
%define var_check(type, pfx, name)
  if (pfx##_##name != def_##name) {
    std::ostringstream a; std::ostringstream b;
    a << pfx##_##name;
    b << def_##name;
    if (a.str() != b.str()) {
      std::cout << "failing in pfx""_""name : "
		<< a.str() << " : " << b.str() << std::endl;
      //      return 0;
    }
  }
%enddef

/* check a function call */
%define call_check(type, pfx, name)
  type pfx##_##tmp##name = def_##name;
  if (pfx##_##name(pfx##_##tmp##name) != def_##name) {
    std::ostringstream a; std::ostringstream b;
    a << pfx##_##name(pfx##_##tmp##name);
    b << def_##name;
    if (a.str() != b.str()) {
      std::cout << "failing in pfx""_""name : "
		<< a.str() << " : " << b.str() << std::endl;
      // return 0;
    }
  }
%enddef

%define wrp_decl(type, pfx, name)
  type wrp##_##pfx##_##name(type x) { 
    return pfx##_##name(x); 
  }
%enddef

/* function passing by value */
%define ovr_decl(type, pfx, name)
  virtual int pfx##_##val(type x) { return 1; }
  virtual int pfx##_##ref(const type& x) { return 1; }
%enddef


#ifdef SWIGPYTHON
%apply (char *STRING, int LENGTH) { (const char *str, size_t len) }
#endif

%inline {
 struct Test 
 {
   Test()
     : var_pchar(0), var_pcharc(0), var_pint(0)
   {
   }

   virtual ~Test()
   {
   }
   
   %test_prim_types_stc(stc_decl, stc)
   %test_prim_types(var_decl, var)
   var_decl(namet, var, namet)


   const char* val_namet(namet x) throw(namet)
   {
     return x;
   }

   const char* val_cnamet(const namet x) throw(namet)
   {
     return x;
   }

#if 0
   /* I have no idea how to define a typemap for 
      const namet&, where namet is a char[ANY]  array */
   const namet& ref_namet(const namet& x) throw(namet)
   {
     return x;
   }
#endif

   
   %test_prim_types(val_decl, val)
   %test_prim_types(ref_decl, ref)

   int c_check() 
   {
     %test_prim_types(call_check, val)
     %test_prim_types(call_check, ref)
     return 1;
   }

   int v_check() 
   {
     %test_prim_types_stc(var_check, stc)
     %test_prim_types(var_check, var)
     var_check(namet, var, namet);
     return 1;
   }

   %test_prim_types_ovr(ovr_decl, ovr)

   int strlen(const char *str, size_t len)
   {
     return len;
   }

   
 };

 struct TestDirector
 {
   TestDirector()
     : var_pchar(0), var_pcharc(0), var_pint(0)
   {
   }

   
   virtual ~TestDirector()
   {
     var_namet[0]='h';
   }

   virtual const char* vval_namet(namet x) throw(namet)
   {
     return x;
   }

   virtual const char* vval_cnamet(const namet x) throw(namet)
   {
     return x;
   }

#if 0
   /* I have no idea how to define a typemap for 
      const namet&, where namet is a char[ANY]  array */
   virtual const namet& vref_namet(const namet& x) throw(namet)
   {
     return x;
   }
#endif


   %test_prim_types_stc(stc_decl, stc)
   %test_prim_types(var_decl, var)
   var_decl(namet, var, namet)

   %test_prim_types(val_decl, val)
   %test_prim_types(ref_decl, ref)

   %test_prim_types(vval_decl, vval)
   %test_prim_types(vref_decl, vref)

   %test_prim_types(wrp_decl, vref)
   %test_prim_types(wrp_decl, vval)

   int c_check() 
   {
     %test_prim_types(call_check, vval)
     %test_prim_types(call_check, vref)
     return 1;
   }

   int v_check() 
   {
     %test_prim_types_stc(var_check, stc)
     %test_prim_types(var_check, var)
     return 1;
   }

   %test_prim_types_ovr(ovr_decl, ovr)
   

   virtual Test* vtest(Test* t) const throw (Test)
   {
     return t;
   }
   

 }; 

 int v_check() 
 {
   %test_prim_types(var_check, cct)
   %test_prim_types(var_check, var)
   var_check(namet, var, namet);
   return 1;
 }

}


%apply SWIGTYPE* { char *};
  
%include "carrays.i"
%array_functions(char,pchar);

