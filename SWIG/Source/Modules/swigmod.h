/* -----------------------------------------------------------------------------
 * swigmod.h
 *
 *     Main header file for SWIG modules
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1998-2000.  The University of Chicago
 * Copyright (C) 1995-1998.  The University of Utah and The Regents of the
 *                           University of California.
 *
 * See the file LICENSE for information on usage and redistribution.
 *
 * $Header$
 * ----------------------------------------------------------------------------- */

#ifndef SWIGMOD_H_
#define SWIGMOD_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "swig.h"
Hash  *Preprocessor_define(const String_or_char *str, int swigmacro);
}

#include "swigwarn.h"

#define NOT_VIRTUAL     0
#define PLAIN_VIRTUAL   1
#define PURE_VIRTUAL    2

extern  char     *input_file;
extern  int       line_number;
extern  int       start_line;
extern  int       CPlusPlus;                        // C++ mode
extern  int       Extend;                           // Extend mode
extern  int       NoInclude;                        // NoInclude flag
extern  int       Verbose;
extern  int       IsVirtual;
extern  int       ImportMode;
extern  int       NoExcept;                         // -no_except option
extern  int       Abstract;                         // abstract base class
extern  int       SmartPointer;                     // smart pointer methods being emitted

/* Overload "argc" and "argv" */
extern String *argv_template_string;
extern String *argc_template_string;

/* Miscellaneous stuff */

#define  tab2   "  "
#define  tab4   "    "
#define  tab8   "        "

class Dispatcher {
 public:
  
  virtual int emit_one(Node *n);
  virtual int emit_children(Node *n);
  virtual int defaultHandler(Node *n);

  /* Top of the parse tree */
  virtual int  top(Node *n) = 0;
  
  /* SWIG directives */

  virtual int applyDirective(Node *n);
  virtual int clearDirective(Node *n);
  virtual int constantDirective(Node *n);
  virtual int extendDirective(Node *n);
  virtual int fragmentDirective(Node *n);
  virtual int importDirective(Node *n);
  virtual int includeDirective(Node *n);
  virtual int insertDirective(Node *n);
  virtual int moduleDirective(Node *n);
  virtual int nativeDirective(Node *n);
  virtual int pragmaDirective(Node *n);
  virtual int typemapDirective(Node *n);
  virtual int typemapitemDirective(Node *n);
  virtual int typemapcopyDirective(Node *n);
  virtual int typesDirective(Node *n);

  /* C/C++ parsing */
  
  virtual int cDeclaration(Node *n);
  virtual int externDeclaration(Node *n);
  virtual int enumDeclaration(Node *n);
  virtual int enumvalueDeclaration(Node *n);
  virtual int classDeclaration(Node *n);
  virtual int classforwardDeclaration(Node *n);
  virtual int constructorDeclaration(Node *n);
  virtual int destructorDeclaration(Node *n);
  virtual int accessDeclaration(Node *n);
  virtual int usingDeclaration(Node *n);
  virtual int namespaceDeclaration(Node *n);
  virtual int templateDeclaration(Node *n);
};

/************************************************************************
 * class language:
 *
 * This class defines the functions that need to be supported by the
 * scripting language being used.    The translator calls these virtual
 * functions to output different types of code for different languages.
 *************************************************************************/

class Language : public Dispatcher {
public:
  Language();
  virtual ~Language();
  virtual int emit_one(Node *n);

  /* Parse command line options */

  virtual void main(int argc, char *argv[]);

  /* Top of the parse tree */

  virtual int  top(Node *n);
  
  /* SWIG directives */
  

  virtual int applyDirective(Node *n);
  virtual int clearDirective(Node *n);
  virtual int constantDirective(Node *n);
  virtual int extendDirective(Node *n);
  virtual int fragmentDirective(Node *n);
  virtual int importDirective(Node *n);
  virtual int includeDirective(Node *n);
  virtual int insertDirective(Node *n);
  virtual int moduleDirective(Node *n);
  virtual int nativeDirective(Node *n);
  virtual int pragmaDirective(Node *n);
  virtual int typemapDirective(Node *n);
  virtual int typemapcopyDirective(Node *n);
  virtual int typesDirective(Node *n);

  /* C/C++ parsing */
  
  virtual int cDeclaration(Node *n);
  virtual int externDeclaration(Node *n);
  virtual int enumDeclaration(Node *n);
  virtual int enumvalueDeclaration(Node *n);
  virtual int classDeclaration(Node *n);
  virtual int classforwardDeclaration(Node *n);
  virtual int constructorDeclaration(Node *n);
  virtual int destructorDeclaration(Node *n);
  virtual int accessDeclaration(Node *n);
  virtual int namespaceDeclaration(Node *n);
  virtual int usingDeclaration(Node *n);

  /* Function handlers */

  virtual int functionHandler(Node *n);
  virtual int globalfunctionHandler(Node *n);
  virtual int memberfunctionHandler(Node *n);
  virtual int staticmemberfunctionHandler(Node *n);
  virtual int callbackfunctionHandler(Node *n);

  /* Variable handlers */

  virtual int variableHandler(Node *n);
  virtual int globalvariableHandler(Node *n);
  virtual int membervariableHandler(Node *n);
  virtual int staticmembervariableHandler(Node *n);

  /* C++ handlers */

  virtual int memberconstantHandler(Node *n);
  virtual int constructorHandler(Node *n);
  virtual int copyconstructorHandler(Node *n);
  virtual int destructorHandler(Node *n);
  virtual int classHandler(Node *n);

  /* Miscellaneous */

  virtual int typedefHandler(Node *n);

  /* Low-level code generation */

  virtual int constantWrapper(Node *n);
  virtual int variableWrapper(Node *n);
  virtual int functionWrapper(Node *n);
  virtual int nativeWrapper(Node *n);

  /* C++ director class generation */
  virtual int classDirector(Node *n);
  virtual int classDirectorInit(Node *n);
  virtual int classDirectorEnd(Node *n);
  virtual int unrollVirtualMethods(Node *n, 
                                   Node *parent, 
                                   Hash *vm, 
                                   int default_director, 
                                   int &virtual_destructor);
  virtual int classDirectorConstructor(Node *n);
  virtual int classDirectorDefaultConstructor(Node *n);
  virtual int classDirectorMethod(Node *n, Node *parent, String *super);
  virtual int classDirectorConstructors(Node *n);
  virtual int classDirectorMethods(Node *n);
  virtual int classDirectorDisown(Node *n);

  /* Miscellaneous */

  virtual  int  validIdentifier(String *s);        /* valid identifier? */
  virtual  int  addSymbol(String *s, Node *n);     /* Add symbol        */
  virtual  Node *symbolLookup(String *s);          /* Symbol lookup     */
  virtual  Node *classLookup(SwigType *s);         /* Class lookup      */
  virtual  int  abstractClassTest(Node *n);	   /* Is class really abstract? */
  
  /* Allow director related code generation */
  void allow_directors(int val = 1);

  /* Return true if directors are enabled */
  int directorsEnabled() const;

  /* Allow director protected members related code generation */
  void allow_dirprot(int val = 1);  

  /* Returns the dirprot mode */
  int dirprot_mode() const;  

  /* Set none comparison string */
  void setSubclassInstanceCheck(String *s);

  /* Set overload variable templates argc and argv */
  void setOverloadResolutionTemplates(String *argc, String *argv);

 protected:
  /* Patch C++ pass-by-value */
  static void patch_parms(Parm *p);

  /* Allow multiple-input typemaps */
  void   allow_multiple_input(int val = 1);

  /* Allow overloaded functions */
  void   allow_overloading(int val = 1);

  /* Wrapping class query */
  int is_wrapping_class();

  /* Return the node for the current class */
  Node *getCurrentClass() const;
    
  /* Return the real name of the current class */
  String *getClassName() const;
  
  /* Return the current class prefix */
  String *getClassPrefix() const;

  /* Fully qualified type name to use */
  String *getClassType() const;

  /* Return true if the current method is part of a smart-pointer */
  int is_smart_pointer() const;

  /* Director subclass comparison test */
  String *none_comparison;

  /* Director constructor "template" code */
  String *director_ctor_code;

 private:
  Hash   *symbols;
  Hash   *classtypes;
  int     overloading;
  int     multiinput;
  int     directors;
  
};

int   SWIG_main(int, char **, Language *);
void  emit_args(SwigType *, ParmList *, Wrapper *f);
void  SWIG_exit(int);           /* use EXIT_{SUCCESS,FAILURE} */
void  SWIG_config_file(const String_or_char *);
const String *SWIG_output_directory();
void  SWIG_config_cppext(const char *ext);

void   SWIG_library_directory(const char *);
int    emit_num_arguments(ParmList *);
int    emit_num_required(ParmList *);
int    emit_isvarargs(ParmList *);
void   emit_attach_parmmaps(ParmList *, Wrapper *f);
void   emit_mark_varargs(ParmList *l);
void   emit_action(Node *n, Wrapper *f);
List  *Swig_overload_rank(Node *n);
String *Swig_overload_dispatch(Node *n, const String_or_char *fmt, int *);
SwigType *cplus_value_type(SwigType *t);

/* directors.cxx start */
String *Swig_csuperclass_call(String* base, String* method, ParmList* l);
String *Swig_class_declaration(Node *n, String *name);
String *Swig_class_name(Node *n);
String *Swig_method_call(String_or_char *name, ParmList *parms);
String *Swig_method_decl(SwigType *s, const String_or_char *id, List *args, int strip, int values);
String *Swig_director_declaration(Node *n);
/* directors.cxx end */

extern "C" {
  void  SWIG_typemap_lang(const char *);
  typedef Language *(*ModuleFactory)(void);
}

void   Swig_register_module(const char *name, ModuleFactory fac);
ModuleFactory Swig_find_module(const char *name);

/* Utilities */

extern int is_public(Node* n); 
extern int is_private(Node* n); 
extern int is_protected(Node* n); 
extern int is_member_director(Node* parentnode, Node* member); 
extern int is_member_director(Node* member); 

#endif




