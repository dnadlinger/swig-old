/* -----------------------------------------------------------------------------
 * csharp.cxx
 *
 *     CSharp wrapper module.
 *
 * Author(s) : William Fulton
 *             Neil Cawse
 *
 * Copyright (C) 1999-2002.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 * ----------------------------------------------------------------------------- */

char cvsroot_csharp_cxx[] = "$Header$";

#include <limits.h> // for INT_MAX
#include "swigmod.h"
#ifndef MACSWIG
#include "swigconfig.h"
#endif
#include <ctype.h>

class CSHARP : public Language {
  static const char *usage;
  const  String *empty_string;

  Hash   *swig_types_hash;
  File   *f_runtime;
  File   *f_header;
  File   *f_wrappers;
  File   *f_init;

  bool   proxy_flag; // Flag for generating proxy classes
  bool   have_default_constructor_flag;
  bool   native_function_flag;     // Flag for when wrapping a native function
  bool   enum_constant_flag; // Flag for when wrapping an enum or constant
  bool   static_flag; // Flag for when wrapping a static functions or member variables
  bool   variable_wrapper_flag; // Flag for when wrapping a nonstatic member variable
  bool   wrapping_member_flag; // Flag for when wrapping a member variable/enum/const
  bool   global_variable_flag; // Flag for when wrapping a global variable

  String *imclass_name;  // intermediary class name
  String *module_class_name;  // module class name
  String *imclass_class_code; // intermediary class code
  String *proxy_class_def;
  String *proxy_class_code;
  String *module_class_code;
  String *proxy_class_name;
  String *variable_name; //Name of a variable being wrapped
  String *proxy_class_constants_code;
  String *module_class_constants_code;
  String *package; // Package name
  String *imclass_imports; //intermediary class imports from %pragma
  String *module_imports; //module imports from %pragma
  String *imclass_baseclass; //inheritance for intermediary class class from %pragma
  String *module_baseclass; //inheritance for module class from %pragma
  String *imclass_interfaces; //interfaces for intermediary class class from %pragma
  String *module_interfaces; //interfaces for module class from %pragma
  String *imclass_class_modifiers; //class modifiers for intermediary class overriden by %pragma
  String *module_class_modifiers; //class modifiers for module class overriden by %pragma
  String *upcasts_code; //C++ casts for inheritance hierarchies C++ code
  String *imclass_cppcasts_code; //C++ casts up inheritance hierarchies intermediary class code
  String *destructor_call; //C++ destructor call if any

  enum type_additions {none, pointer, reference};

  public:

  /* -----------------------------------------------------------------------------
   * CSHARP()
   * ----------------------------------------------------------------------------- */

  CSHARP() : 
    empty_string(NewString("")),

    swig_types_hash(NULL),
    f_runtime(NULL),
    f_header(NULL),
    f_wrappers(NULL),
    f_init(NULL),

    proxy_flag(true),
    have_default_constructor_flag(false),
    native_function_flag(false),
    enum_constant_flag(false),
    static_flag(false),
    variable_wrapper_flag(false),
    wrapping_member_flag(false),
    global_variable_flag(false),

    imclass_name(NULL),
    module_class_name(NULL),
    imclass_class_code(NULL),
    proxy_class_def(NULL),
    proxy_class_code(NULL),
    module_class_code(NULL),
    proxy_class_name(NULL),
    variable_name(NULL),
    proxy_class_constants_code(NULL),
    module_class_constants_code(NULL),
    package(NULL),
    imclass_imports(NULL),
    module_imports(NULL),
    imclass_baseclass(NULL),
    module_baseclass(NULL),
    imclass_interfaces(NULL),
    module_interfaces(NULL),
    imclass_class_modifiers(NULL),
    module_class_modifiers(NULL),
    upcasts_code(NULL),
    imclass_cppcasts_code(NULL),
    destructor_call(NULL)

    {
    }

  /* -----------------------------------------------------------------------------
   * getProxyName()
   *
   * Test to see if a type corresponds to something wrapped with a proxy class
   * Return NULL if not otherwise the proxy class name
   * ----------------------------------------------------------------------------- */

  String *getProxyName(SwigType *t) {
    if (proxy_flag) {
      Node *n = classLookup(t);
      if (n) {
        return Getattr(n,"sym:name");
      }
    }
    return NULL;
  }

  /* ------------------------------------------------------------
   * main()
   * ------------------------------------------------------------ */

  virtual void main(int argc, char *argv[]) {

    SWIG_library_directory("csharp");

    // Look for certain command line options
    for (int i = 1; i < argc; i++) {
      if (argv[i]) {
        if (strcmp(argv[i],"-package") == 0) {
          if (argv[i+1]) {
            package = NewString("");
            Printf(package, argv[i+1]);
            Swig_mark_arg(i);
            Swig_mark_arg(i+1);
            i++;
          } else {
            Swig_arg_error();
          }
        } else if ((strcmp(argv[i],"-noproxy") == 0)) {
          Swig_mark_arg(i);
          proxy_flag = false;
        } else if (strcmp(argv[i],"-help") == 0) {
          Printf(stderr,"%s\n", usage);
        }
      }
    }

    // Add a symbol to the parser for conditional compilation
    Preprocessor_define("SWIGCSHARP 1",0);

    // Add typemap definitions
    SWIG_typemap_lang("csharp");
    SWIG_config_file("csharp.swg");

    allow_overloading();
  }

  /* ---------------------------------------------------------------------
   * top()
   * --------------------------------------------------------------------- */

  virtual int top(Node *n) {

    /* Initialize all of the output files */
    String *outfile = Getattr(n,"outfile");

    f_runtime = NewFile(outfile,"w");
    if (!f_runtime) {
      Printf(stderr,"Unable to open %s\n", outfile);
      SWIG_exit(EXIT_FAILURE);
    }
    f_init = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");

    /* Register file targets with the SWIG file handler */
    Swig_register_filebyname("header",f_header);
    Swig_register_filebyname("wrapper",f_wrappers);
    Swig_register_filebyname("runtime",f_runtime);
    Swig_register_filebyname("init",f_init);
    swig_types_hash = NewHash();

    // Make the intermediary class and module class names. The intermediary class name can be set in the module directive.
    Node* optionsnode = Getattr( Getattr(n,"module") ,"options");
    if (optionsnode)
        if (Getattr(optionsnode,"imclassname"))
          imclass_name = Copy(Getattr(optionsnode,"imclassname"));
    if (!imclass_name) {
      imclass_name = NewStringf("%sPINVOKE", Getattr(n,"name"));
      module_class_name = Copy(Getattr(n,"name"));
    } else {
      // Rename the module name if it is the same as intermediary class name - a backwards compatibility solution
      if (Cmp(imclass_name, Getattr(n,"name")) == 0)
        module_class_name = NewStringf("%sModule", Getattr(n,"name"));
      else
        module_class_name = Copy(Getattr(n,"name"));
    }


    imclass_class_code = NewString("");
    proxy_class_def = NewString("");
    proxy_class_code = NewString("");
    module_class_constants_code = NewString("");
    imclass_baseclass = NewString("");
    imclass_interfaces = NewString("");
    imclass_class_modifiers = NewString(""); // package access only to the intermediary class by default
    module_class_code = NewString("");
    module_baseclass = NewString("");
    module_interfaces = NewString("");
    module_imports = NewString("");
    module_class_modifiers = NewString("public");
    imclass_imports = NewString("");
    imclass_cppcasts_code = NewString("");
    upcasts_code = NewString("");
    if (!package) package = NewString("");

    Swig_banner(f_runtime);               // Print the SWIG banner message

    if (NoInclude) {
      Printf(f_runtime,"#define SWIG_NOINCLUDE\n");
    }

    String *wrapper_name = NewString("");

    Printf(wrapper_name, "CSharp_%%f", imclass_name);
    Swig_name_register((char*)"wrapper", Char(wrapper_name));
    Swig_name_register((char*)"set", (char*)"set_%v");
    Swig_name_register((char*)"get", (char*)"get_%v");
    Swig_name_register((char*)"member", (char*)"%c_%m");

    Delete(wrapper_name);

    Printf(f_wrappers,"\n#ifdef __cplusplus\n");
    Printf(f_wrappers,"extern \"C\" {\n");
    Printf(f_wrappers,"#endif\n\n");

    /* Emit code */
    Language::top(n);

    // Generate the intermediary class
    {
      String *filen = NewStringf("%s%s.cs", SWIG_output_directory(), imclass_name);
      File *f_im = NewFile(filen,"w");
      if(!f_im) {
        Printf(stderr,"Unable to open %s\n", filen);
        SWIG_exit(EXIT_FAILURE);
      }
      Delete(filen); filen = NULL;

      // Start writing out the intermediary class
      if(Len(package) > 0)
        Printf(f_im, "//package %s;\n\n", package);

      emitBanner(f_im);
      if(imclass_imports)
        Printf(f_im, "%s\n", imclass_imports);

      if (Len(imclass_class_modifiers) > 0)
        Printf(f_im, "%s ", imclass_class_modifiers);
      Printf(f_im, "class %s ", imclass_name);

      if (imclass_baseclass && *Char(imclass_baseclass))
          Printf(f_im, ": %s ", imclass_baseclass);
      if (Len(imclass_interfaces) > 0)
        Printv(f_im, "implements ", imclass_interfaces, " ", NIL);
      Printf(f_im, "{\n");

      // Add the intermediary class methods
      Printv(f_im, imclass_class_code, NIL);
      Printv(f_im, imclass_cppcasts_code, NIL);

      // Finish off the class
      Printf(f_im, "}\n");
      Close(f_im);
    }

    // Generate the C# module class
    {
      String *filen = NewStringf("%s%s.cs", SWIG_output_directory(), module_class_name);
      File *f_module = NewFile(filen,"w");
      if(!f_module) {
        Printf(stderr,"Unable to open %s\n", filen);
        SWIG_exit(EXIT_FAILURE);
      }
      Delete(filen); filen = NULL;

      // Start writing out the module class
      if(Len(package) > 0)
        Printf(f_module, "//package %s;\n\n", package);

      emitBanner(f_module);
      if(module_imports)
        Printf(f_module, "%s\n", module_imports);

      if (Len(module_class_modifiers) > 0)
        Printf(f_module, "%s ", module_class_modifiers);
      Printf(f_module, "class %s ", module_class_name);

      if (module_baseclass && *Char(module_baseclass))
          Printf(f_module, ": %s ", module_baseclass);
      if (Len(module_interfaces) > 0)
        Printv(f_module, "implements ", module_interfaces, " ", NIL);
      Printf(f_module, "{\n");

      // Add the wrapper methods
      Printv(f_module, module_class_code, NIL);

      // Write out all the enums constants
      if (Len(module_class_constants_code) != 0 )
        Printv(f_module, "  // enums and constants\n", module_class_constants_code, NIL);

      // Finish off the class
      Printf(f_module, "}\n");
      Close(f_module);
    }

    if(upcasts_code)
      Printv(f_wrappers,upcasts_code,NIL);

    Printf(f_wrappers,"#ifdef __cplusplus\n");
    Printf(f_wrappers,"}\n");
    Printf(f_wrappers,"#endif\n");

    // Output a C# type wrapper class for each SWIG type
    for (String *swig_type = Firstkey(swig_types_hash); swig_type; swig_type = Nextkey(swig_types_hash)) {
      emitTypeWrapperClass(swig_type, Getattr(swig_types_hash, swig_type));
    }

    Delete(swig_types_hash); swig_types_hash = NULL;
    Delete(imclass_name); imclass_name = NULL;
    Delete(imclass_class_code); imclass_class_code = NULL;
    Delete(proxy_class_def); proxy_class_def = NULL;
    Delete(proxy_class_code); proxy_class_code = NULL;
    Delete(module_class_constants_code); module_class_constants_code = NULL;
    Delete(imclass_baseclass); imclass_baseclass = NULL;
    Delete(imclass_interfaces); imclass_interfaces = NULL;
    Delete(imclass_class_modifiers); imclass_class_modifiers = NULL;
    Delete(module_class_name); module_class_name = NULL;
    Delete(module_class_code); module_class_code = NULL;
    Delete(module_baseclass); module_baseclass = NULL;
    Delete(module_interfaces); module_interfaces = NULL;
    Delete(module_imports); module_imports = NULL;
    Delete(module_class_modifiers); module_class_modifiers = NULL;
    Delete(imclass_imports); imclass_imports = NULL;
    Delete(imclass_cppcasts_code); imclass_cppcasts_code = NULL;
    Delete(upcasts_code); upcasts_code = NULL;
    Delete(package); package = NULL;

    /* Close all of the files */
    Dump(f_header,f_runtime);
    Dump(f_wrappers,f_runtime);
    Wrapper_pretty_print(f_init,f_runtime);
    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);
    Close(f_runtime);
    Delete(f_runtime);
    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------------
   * emitBanner()
   * ----------------------------------------------------------------------------- */

  void emitBanner(File *f) {
    Printf(f, "/* ----------------------------------------------------------------------------\n");
    Printf(f, " * This file was automatically generated by SWIG (http://www.swig.org).\n");
    Printf(f, " * Version: %s\n", PACKAGE_VERSION);
    Printf(f, " *\n");
    Printf(f, " * Do not make changes to this file unless you know what you are doing--modify\n");
    Printf(f, " * the SWIG interface file instead.\n");
    Printf(f, " * ----------------------------------------------------------------------------- */\n\n");
  }

  /* ----------------------------------------------------------------------
   * nativeWrapper()
   * ---------------------------------------------------------------------- */

  virtual int nativeWrapper(Node *n) {
    String *wrapname = Getattr(n,"wrap:name");

    if (!addSymbol(wrapname,n)) return SWIG_ERROR;

    if (Getattr(n,"type")) {
      Swig_save("nativeWrapper",n,"name",NIL);
      Setattr(n,"name", wrapname);
      native_function_flag = true;
      functionWrapper(n);
      Swig_restore(n);
      native_function_flag = false;
    } else {
      Printf(stderr,"%s : Line %d. No return type for %%native method %s.\n", input_file, line_number, Getattr(n,"wrap:name"));
    }

    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * functionWrapper()
   * ---------------------------------------------------------------------- */

  virtual int functionWrapper(Node *n) {
    String   *symname = Getattr(n,"sym:name");
    SwigType *t = Getattr(n,"type");
    ParmList *l = Getattr(n,"parms");
    String    *tm;
    Parm      *p;
    int       i;
    String    *c_return_type = NewString("");
    String    *im_return_type = NewString("");
    String    *cleanup = NewString("");
    String    *outarg = NewString("");
    String    *body = NewString("");
    int       num_arguments = 0;
    int       num_required = 0;
    bool      is_void_return;
    String    *overloaded_name = getOverloadedName(n);

    if (!Getattr(n,"sym:overloaded")) {
      if (!addSymbol(Getattr(n,"sym:name"),n)) return SWIG_ERROR;
    }

    /* 
     * Generate the proxy class properties for public member variables.
     * Not for enums and constants.
     */
    if(proxy_flag && wrapping_member_flag && !enum_constant_flag) {
      // Capitalize the first letter in the variable in the getter/setter function name
      bool getter_flag = Cmp(symname, Swig_name_set(Swig_name_member(proxy_class_name, variable_name))) != 0;

      String *getter_setter_name = NewString("");
      if(!getter_flag)
        Printf(getter_setter_name,"set");
      else 
        Printf(getter_setter_name,"get");
      Putc(toupper((int) *Char(variable_name)), getter_setter_name);
      Printf(getter_setter_name, "%s", Char(variable_name)+1);

      Setattr(n,"proxyfuncname", getter_setter_name);
      Setattr(n,"imfuncname", symname);

      proxyClassFunctionHandler(n);
      Delete(getter_setter_name);
    }

    /*
       The rest of this function deals with generating the intermediary class wrapper function (that wraps
       a c/c++ function) and generating the PInvoke c code. Each C# wrapper function has a 
       matching PInvoke c function call.
       */

    // A new wrapper function object
    Wrapper *f = NewWrapper();

    // Make a wrapper name for this function
    String *wname = Swig_name_wrapper(overloaded_name);

    /* Attach the non-standard typemaps to the parameter list. */
    Swig_typemap_attach_parms("ctype", l, f);
    Swig_typemap_attach_parms("imtype", l, f);

    /* Get return types */
    if ((tm = Swig_typemap_lookup_new("ctype",n,"",0))) {
      Printf(c_return_type,"%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CTYPE_UNDEF, input_file, line_number, 
          "No ctype typemap defined for %s\n", SwigType_str(t,0));
    }

    if ((tm = Swig_typemap_lookup_new("imtype",n,"",0))) {
      Printf(im_return_type,"%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSTYPE_UNDEF, input_file, line_number, 
          "No imtype typemap defined for %s\n", SwigType_str(t,0));
    }

    is_void_return = (Cmp(c_return_type, "void") == 0);
    if (!is_void_return)
      Wrapper_add_localv(f,"jresult", c_return_type, "jresult = 0",NIL);

    Printv(imclass_class_code, 
           "\n  [DllImport(\"",module_class_name,"\", EntryPoint=\"CSharp_",overloaded_name,"\")]\n", NIL);

    Printf(imclass_class_code, "  public static extern %s %s(", im_return_type, overloaded_name);

    Printv(f->def, " DllExport ", c_return_type, " ", wname, "(", NIL);

    // Emit all of the local variables for holding arguments.
    emit_args(t,l,f);

    /* Attach the standard typemaps */
    emit_attach_parmmaps(l,f);
    Setattr(n,"wrap:parms",l);

    /* Get number of required and total arguments */
    num_arguments = emit_num_arguments(l);
    num_required  = emit_num_required(l);
    int gencomma = 0;

    // Now walk the function parameter list and generate code to get arguments
    for (i = 0, p=l; i < num_arguments; i++) {

      while (checkAttribute(p,"tmap:in:numinputs","0")) {
        p = Getattr(p,"tmap:in:next");
      }

      SwigType *pt = Getattr(p,"type");
      String   *ln = Getattr(p,"lname");
      String   *im_param_type = NewString("");
      String   *c_param_type = NewString("");
      String   *arg = NewString("");

      Printf(arg,"j%s", ln);

      /* Get the ctype types of the parameter */
      if ((tm = Getattr(p,"tmap:ctype"))) {
        Printv(c_param_type, tm, NIL);
      } else {
        Swig_warning(WARN_CSHARP_TYPEMAP_CTYPE_UNDEF, input_file, line_number, 
            "No ctype typemap defined for %s\n", SwigType_str(pt,0));
      }

      /* Get the intermediary class parameter types of the parameter */
      if ((tm = Getattr(p,"tmap:imtype"))) {
        Printv(im_param_type, tm, NIL);
      } else {
        Swig_warning(WARN_CSHARP_TYPEMAP_CSTYPE_UNDEF, input_file, line_number, 
            "No imtype typemap defined for %s\n", SwigType_str(pt,0));
      }

      /* Add parameter to intermediary class method */
      if(gencomma) Printf(imclass_class_code, ", ");
      Printf(imclass_class_code, "%s %s", im_param_type, arg);

      // Add parameter to C function
      Printv(f->def, gencomma?", ":"", c_param_type, " ", arg, NIL);

      gencomma = 1;
      
      // Get typemap for this argument
      if ((tm = Getattr(p,"tmap:in"))) {
        addThrows(n, "tmap:in", p);
        Replaceall(tm,"$source",arg); /* deprecated */
        Replaceall(tm,"$target",ln); /* deprecated */
        Replaceall(tm,"$arg",arg); /* deprecated? */
        Replaceall(tm,"$input", arg);
        Setattr(p,"emit:input", arg);
        Printf(f->code,"%s\n", tm);
        p = Getattr(p,"tmap:in:next");
      } else {
        Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, 
            "Unable to use type %s as a function argument.\n",SwigType_str(pt,0));
        p = nextSibling(p);
      }
      Delete(im_param_type);
      Delete(c_param_type);
      Delete(arg);
    }

    /* Insert constraint checking code */
    for (p = l; p;) {
      if ((tm = Getattr(p,"tmap:check"))) {
        addThrows(n, "tmap:check", p);
        Replaceall(tm,"$target",Getattr(p,"lname")); /* deprecated */
        Replaceall(tm,"$arg",Getattr(p,"emit:input")); /* deprecated? */
        Replaceall(tm,"$input",Getattr(p,"emit:input"));
        Printv(f->code,tm,"\n",NIL);
        p = Getattr(p,"tmap:check:next");
      } else {
        p = nextSibling(p);
      }
    }

    /* Insert cleanup code */
    for (p = l; p;) {
      if ((tm = Getattr(p,"tmap:freearg"))) {
        addThrows(n, "tmap:freearg", p);
        Replaceall(tm,"$source",Getattr(p,"emit:input")); /* deprecated */
        Replaceall(tm,"$arg",Getattr(p,"emit:input")); /* deprecated? */
        Replaceall(tm,"$input",Getattr(p,"emit:input"));
        Printv(cleanup,tm,"\n",NIL);
        p = Getattr(p,"tmap:freearg:next");
      } else {
        p = nextSibling(p);
      }
    }

    /* Insert argument output code */
    for (p = l; p;) {
      if ((tm = Getattr(p,"tmap:argout"))) {
        addThrows(n, "tmap:argout", p);
        Replaceall(tm,"$source",Getattr(p,"emit:input")); /* deprecated */
        Replaceall(tm,"$target",Getattr(p,"lname")); /* deprecated */
        Replaceall(tm,"$arg",Getattr(p,"emit:input")); /* deprecated? */
        Replaceall(tm,"$result","jresult");
        Replaceall(tm,"$input",Getattr(p,"emit:input"));
        Printv(outarg,tm,"\n",NIL);
        p = Getattr(p,"tmap:argout:next");
      } else {
        p = nextSibling(p);
      }
    }

    // Get any C# exception classes in the throws typemap
    ParmList *throw_parm_list = NULL;
    if ((throw_parm_list = Getattr(n,"throws"))) {
      Swig_typemap_attach_parms("throws", throw_parm_list, f);
      for (p = throw_parm_list; p; p=nextSibling(p)) {
        if ((tm = Getattr(p,"tmap:throws"))) {
          addThrows(n, "tmap:throws", p);
        }
      }
    }

    if (Cmp(nodeType(n), "constant") == 0) {
      // Wrapping a constant hack
      Swig_save("functionWrapper",n,"wrap:action",NIL);

      // below based on Swig_VargetToFunction()
      SwigType *ty = Swig_wrapped_var_type(Getattr(n,"type"));
      Setattr(n,"wrap:action", NewStringf("result = (%s) %s;\n", SwigType_lstr(ty,0), Getattr(n, "value")));
    }

    // Now write code to make the function call
    if(!native_function_flag)
      emit_action(n,f);

    if (Cmp(nodeType(n), "constant") == 0)
      Swig_restore(n);

    /* Return value if necessary  */
    if(!native_function_flag) {
      if ((tm = Swig_typemap_lookup_new("out",n,"result",0))) {
        addThrows(n, "tmap:out", n);
        Replaceall(tm,"$source", "result"); /* deprecated */
        Replaceall(tm,"$target", "jresult"); /* deprecated */
        Replaceall(tm,"$result","jresult");
        Printf(f->code,"%s", tm);
        if (Len(tm))
          Printf(f->code,"\n");
      } else {
        Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number,
            "Unable to use return type %s in function %s.\n", SwigType_str(t,0), Getattr(n,"name"));
      }
    }

    /* Output argument output code */
    Printv(f->code,outarg,NIL);

    /* Output cleanup code */
    Printv(f->code,cleanup,NIL);

    /* Look to see if there is any newfree cleanup code */
    if (Getattr(n,"feature:new")) {
      if ((tm = Swig_typemap_lookup_new("newfree",n,"result",0))) {
        addThrows(n, "tmap:newfree", n);
        Replaceall(tm,"$source","result"); /* deprecated */
        Printf(f->code,"%s\n",tm);
      }
    }

    /* See if there is any return cleanup code */
    if(!native_function_flag) {
      if ((tm = Swig_typemap_lookup_new("ret", n, "result", 0))) {
        Replaceall(tm,"$source","result"); /* deprecated */
        Printf(f->code,"%s\n",tm);
      }
    }

    /* Finish C function and intermediary class function definitions */
    Printf(imclass_class_code, ")");
    generateThrowsClause(n, imclass_class_code);
    Printf(imclass_class_code, ";\n");
    Printf(f->def,") {");

    if(!is_void_return)
      Printv(f->code, "    return jresult;\n", NIL);
    Printf(f->code, "}\n");

    /* Substitute the cleanup code */
    Replaceall(f->code,"$cleanup",cleanup);

    if(!is_void_return)
      Replaceall(f->code,"$null","0");
    else
      Replaceall(f->code,"$null","");

    /* Dump the function out */
    if(!native_function_flag)
      Wrapper_print(f,f_wrappers);

    Setattr(n,"wrap:name", wname);

    /* Emit warnings for the few cases that can't be overloaded in C# */
    if (Getattr(n,"sym:overloaded")) {
      if (!Getattr(n,"sym:nextSibling"))
        Swig_overload_rank(n);
    }

    if(!(proxy_flag && is_wrapping_class()) && !enum_constant_flag) {
      moduleClassFunctionHandler(n);
    }

    Delete(c_return_type);
    Delete(im_return_type);
    Delete(cleanup);
    Delete(outarg);
    Delete(body);
    Delete(overloaded_name);
    DelWrapper(f);
    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------
   * variableWrapper()
   * ----------------------------------------------------------------------- */

  virtual int variableWrapper(Node *n) {
    Language::variableWrapper(n);
    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------
   * globalvariableHandler()
   * ------------------------------------------------------------------------ */

  virtual int globalvariableHandler(Node *n) {
    SwigType  *t = Getattr(n,"type");
    String    *tm;

    // Get the variable type
    if ((tm = Swig_typemap_lookup_new("cstype",n,"",0))) {
      substituteClassname(t, tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
          "No cstype typemap defined for %s\n", SwigType_str(t,0));
    }

    // Output the property's field declaration and accessor methods
    Printf(module_class_code, "  public static %s %s {", tm, Getattr(n, "sym:name"));

    variable_name = Getattr(n,"sym:name");
    global_variable_flag = true;
    int ret = Language::globalvariableHandler(n);
    global_variable_flag = false;

    Printf(module_class_code, "\n  }\n\n");

    return ret;
  }

  /* -----------------------------------------------------------------------
   * constantWrapper()
   * ------------------------------------------------------------------------ */

  virtual int constantWrapper(Node *n) {
    String *symname = Getattr(n,"sym:name");
    SwigType *t     = Getattr(n,"type");
    ParmList  *l    = Getattr(n,"parms");
    String *tm;
    String *return_type = NewString("");
    String *constants_code = NewString("");
    SwigType *original_type = t;

    if (!addSymbol(symname,n)) return SWIG_ERROR;

    bool is_enum_item = (Cmp(nodeType(n), "enumitem") == 0);

    /* Adjust the enum type for the Swig_typemap_lookup. We want the same cstype typemap for all the enum items.
     * The type of each enum item depends on what value it is assigned, but is usually a C int. */
    if (is_enum_item) {
      t = NewStringf("enum %s", Getattr(parentNode(n), "sym:name"));
      Setattr(n,"type", t);
    }

    /* Attach the non-standard typemaps to the parameter list. */
    Swig_typemap_attach_parms("cstype", l, NULL);

    /* Get C# return types */
    bool classname_substituted_flag = false;
    
    if ((tm = Swig_typemap_lookup_new("cstype",n,"",0))) {
      classname_substituted_flag = substituteClassname(t, tm);
      Printf(return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
          "No cstype typemap defined for %s\n", SwigType_str(t,0));
    }

    // Add the stripped quotes back in
    String *new_value = NewString("");
    Swig_save("constantWrapper",n,"value",NIL);
    if(SwigType_type(t) == T_STRING) {
      Printf(new_value, "\"%s\"", Copy(Getattr(n, "value")));
      Setattr(n, "value", new_value);
    }
    else if(SwigType_type(t) == T_CHAR) {
      Printf(new_value, "\'%s\'", Copy(Getattr(n, "value")));
      Setattr(n, "value", new_value);
    }

    // The %csconst feature determines how the constant value is obtained
    String *const_feature = Getattr(n,"feature:cs:const");
    bool const_feature_flag = const_feature && Cmp(const_feature, "0") != 0;

    // enums are wrapped using a public final static int in C#.
    // Other constants are wrapped using a public final static [cstype] in C#.
    Printf(constants_code, "  public %s %s %s = ", (const_feature_flag ? "const" : "static readonly"), return_type, ((proxy_flag && wrapping_member_flag) ? variable_name : symname));

    if ((is_enum_item && Getattr(n,"enumvalue") == 0) || !const_feature_flag) {
      // Enums without value and default constant handling will work with any type of C constant and initialises the C# variable from C through a PInvoke call.

      if(classname_substituted_flag) // This handles function pointers using the %constant directive
        Printf(constants_code, "new %s(%s.%s(), false);\n", return_type, imclass_name, Swig_name_get(symname));
      else
        Printf(constants_code, "%s.%s();\n", imclass_name, Swig_name_get(symname));

      // Each constant and enum value is wrapped with a separate PInvoke function call
      enum_constant_flag = true;
      variableWrapper(n);
      enum_constant_flag = false;
    } else if (is_enum_item) {
      // Alternative enum item handling will use the explicit value of the enum item and hope that it compiles as C# code
      Printf(constants_code, "%s;\n", Getattr(n,"enumvalue"));
    } else {
      // Alternative constant handling will use the C syntax to make a true C# constant and hope that it compiles as C# code
      Printf(constants_code, "%s;\n", Getattr(n,"value"));
    }

    if(proxy_flag && wrapping_member_flag)
      Printv(proxy_class_constants_code, constants_code, NIL);
    else
      Printv(module_class_constants_code, constants_code, NIL);

    Swig_restore(n);
    if (is_enum_item) {
      Delete(Getattr(n,"type"));
      Setattr(n,"type", original_type);
    }
    Delete(new_value);
    Delete(return_type);
    Delete(constants_code);
    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------------
   * pragmaDirective()
   *
   * Valid Pragmas:
   * imclassbase            - base (extends) for the intermediary class
   * imclassclassmodifiers  - class modifiers for the intermediary class
   * imclasscode            - text (C# code) is copied verbatim to the intermediary class
   * imclassimports         - import statements for the intermediary class
   * imclassinterfaces      - interface (implements) for the intermediary class
   *
   * modulebase              - base (extends) for the module class
   * moduleclassmodifiers    - class modifiers for the module class
   * modulecode              - text (C# code) is copied verbatim to the module class
   * moduleimports           - import statements for the module class
   * moduleinterfaces        - interface (implements) for the module class
   *
   * ----------------------------------------------------------------------------- */

  virtual int pragmaDirective(Node *n) {
    if (!ImportMode) {
      String *lang = Getattr(n,"lang");
      String *code = Getattr(n,"name");
      String *value = Getattr(n,"value");

      if(Strcmp(lang, "csharp") == 0) {

        String *strvalue = NewString(value);
        Replaceall(strvalue,"\\\"", "\"");

        if(Strcmp(code, "imclassbase") == 0) {
          Delete(imclass_baseclass);
          imclass_baseclass = Copy(strvalue);
        } 
        else if(Strcmp(code, "imclassclassmodifiers") == 0) {
          Delete(imclass_class_modifiers);
          imclass_class_modifiers = Copy(strvalue);
        } 
        else if(Strcmp(code, "imclasscode") == 0) {
          Printf(imclass_class_code, "%s\n", strvalue);
        } 
        else if(Strcmp(code, "imclassimports") == 0) {
          Delete(imclass_imports);
          imclass_imports = Copy(strvalue);
        } 
        else if(Strcmp(code, "imclassinterfaces") == 0) {
          Delete(imclass_interfaces);
          imclass_interfaces = Copy(strvalue);
        } 
        else if(Strcmp(code, "modulebase") == 0) {
          Delete(module_baseclass);
          module_baseclass = Copy(strvalue);
        } 
        else if(Strcmp(code, "moduleclassmodifiers") == 0) {
          Delete(module_class_modifiers);
          module_class_modifiers = Copy(strvalue);
        } 
        else if(Strcmp(code, "modulecode") == 0) {
          Printf(module_class_code, "%s\n", strvalue);
        } 
        else if(Strcmp(code, "moduleimports") == 0) {
          Delete(module_imports);
          module_imports = Copy(strvalue);
        } 
        else if(Strcmp(code, "moduleinterfaces") == 0) {
          Delete(module_interfaces);
          module_interfaces = Copy(strvalue);
        } else {
          Printf(stderr,"%s : Line %d. Unrecognized pragma.\n", input_file, line_number);
        }
        Delete(strvalue);
      }
    }
    return Language::pragmaDirective(n);
  }

  /* -----------------------------------------------------------------------------
   * emitProxyClassDefAndCPPCasts()
   * ----------------------------------------------------------------------------- */

  void emitProxyClassDefAndCPPCasts(Node *n) {
    String *c_classname = SwigType_namestr(Getattr(n,"name"));
    String *c_baseclass = NULL;
    String *baseclass = NULL;
    String *c_baseclassname = NULL;
    String *classDeclarationName = Getattr(n,"classDeclaration:name");

    /* Deal with inheritance */
    List *baselist = Getattr(n,"bases");
    if (baselist) {
      Node *base = Firstitem(baselist);
      c_baseclassname = Getattr(base,"name");
      baseclass = Copy(getProxyName(c_baseclassname));
      if (baseclass){
        c_baseclass = SwigType_namestr(Getattr(base,"name"));
      }
      base = Nextitem(baselist);
      if (base) {
        Swig_warning(WARN_CSHARP_MULTIPLE_INHERITANCE, input_file, line_number, 
            "Warning for %s proxy: Base %s ignored. Multiple inheritance is not supported in C#.\n", classDeclarationName, Getattr(base,"name"));
      }
    }

    bool derived = baseclass && getProxyName(c_baseclassname);
    if (!baseclass)
      baseclass = NewString("");

    // Inheritance from pure C# classes
    const String *pure_baseclass = typemapLookup("csbase", classDeclarationName, WARN_NONE);
    if (Len(pure_baseclass) > 0 && Len(baseclass) > 0) {
      Swig_warning(WARN_CSHARP_MULTIPLE_INHERITANCE, input_file, line_number, 
          "Warning for %s proxy: Base %s ignored. Multiple inheritance is not supported in C#.\n", classDeclarationName, pure_baseclass);
    }

    // Pure C# interfaces
    const String *pure_interfaces = typemapLookup(derived ? "csinterfaces_derived" : "csinterfaces", classDeclarationName, WARN_NONE);

    // Start writing the proxy class
    Printv(proxy_class_def,
        typemapLookup("csimports", classDeclarationName, WARN_NONE), // Import statements
        "\n",
        typemapLookup("csclassmodifiers", classDeclarationName, WARN_CSHARP_TYPEMAP_CLASSMOD_UNDEF), // Class modifiers
        " class $csclassname",       // Class name and bases
        (derived || *Char(pure_baseclass) || *Char(pure_interfaces)) ?
        " : " : 
        "",
        baseclass,
        pure_baseclass,
        ((derived || *Char(pure_baseclass)) && *Char(pure_interfaces)) ? // Interfaces
        ", " :
        "",
        pure_interfaces,
        " {\n",
        "  private IntPtr swigCPtr;\n",  // Member variables for memory handling
        derived ? 
        "" : 
        "  protected bool swigCMemOwn;\n",
        "\n",
        "  ",
        typemapLookup("csptrconstructormodifiers", classDeclarationName, WARN_CSHARP_TYPEMAP_PTRCONSTMOD_UNDEF), // pointer constructor modifiers
        " $csclassname(IntPtr cPtr, bool cMemoryOwn) ", // Constructor used for wrapping pointers
        derived ? 
        ": base($imclassname.$csclassnameTo$baseclass(cPtr), cMemoryOwn) {\n" : 
        "{\n    swigCMemOwn = cMemoryOwn;\n",
        "    swigCPtr = cPtr;\n",
        "  }\n",
        NIL);

    if(!have_default_constructor_flag) { // All proxy classes need a constructor
      Printv(proxy_class_def, 
          "\n",
          "  protected $csclassname() : this(IntPtr.Zero, false) {\n",
          "  }\n",
          NIL);
    }

    // C++ destructor is wrapped by the Dispose method
    // Note that the method name is specified in a typemap attribute called methodname
    String *destruct = NewString("");
    const String *tm = NULL;
    Node *attributes = NewHash();
    String *destruct_methodname = NULL;
    if (derived) {
      tm = typemapLookup("csdestruct_derived", classDeclarationName, WARN_NONE, attributes);
      destruct_methodname = Getattr(attributes, "tmap:csdestruct_derived:methodname");
    } else {
      tm = typemapLookup("csdestruct", classDeclarationName, WARN_NONE, attributes);
      destruct_methodname = Getattr(attributes, "tmap:csdestruct:methodname");
    }
    if (!destruct_methodname) {
      Swig_error(input_file, line_number, 
          "No methodname attribute defined in csdestruct%s typemap for %s\n", (derived ? "_derived" : ""), proxy_class_name);
    }

    // Emit the Finalize and Dispose methods
    if (tm) {
      // Finalize method
      if (*Char(destructor_call)) {
        Printv(proxy_class_def, 
            typemapLookup("csfinalize", classDeclarationName, WARN_NONE),
            NIL);
      }
      // Dispose method
      Printv(destruct, tm, NIL);
      if (*Char(destructor_call))
        Replaceall(destruct, "$imcall", destructor_call);
      else
        Replaceall(destruct, "$imcall", "throw new MethodAccessException(\"C++ destructor does not have public access\")");
      if (*Char(destruct))
        Printv(proxy_class_def, "\n  public ", derived ? "override" : "virtual", " void ", destruct_methodname, "() ", destruct, "\n", NIL);
    }
    Delete(attributes);
    Delete(destruct);

    // Emit various other methods
    Printv(proxy_class_def, 
        typemapLookup("csgetcptr", classDeclarationName, WARN_CSHARP_TYPEMAP_GETCPTR_UNDEF), // getCPtr method
        typemapLookup("cscode", classDeclarationName, WARN_NONE), // extra C# code
        "\n",
        NIL);

    // Substitute various strings into the above template
    Replaceall(proxy_class_code, "$csclassname", proxy_class_name);
    Replaceall(proxy_class_def,  "$csclassname", proxy_class_name);

    Replaceall(proxy_class_def,  "$baseclass", baseclass);
    Replaceall(proxy_class_code, "$baseclass", baseclass);

    Replaceall(proxy_class_def,  "$imclassname", imclass_name);
    Replaceall(proxy_class_code, "$imclassname", imclass_name);

    // Add code to do C++ casting to base class (only for classes in an inheritance hierarchy)
    if(derived){
      Printv(imclass_cppcasts_code,"\n  [DllImport(\"", module_class_name, "\", EntryPoint=\"CSharp_", proxy_class_name ,"To", baseclass ,"\")]\n", NIL);
      Printv(imclass_cppcasts_code,"  public static extern IntPtr ",
          "$csclassnameTo$baseclass(IntPtr objectRef);\n",
          NIL);

      Replaceall(imclass_cppcasts_code, "$csclassname", proxy_class_name);
      Replaceall(imclass_cppcasts_code, "$baseclass", baseclass);

      Printv(upcasts_code,
          "DllExport long CSharp_$imclazznameTo$imbaseclass",
          "(long objectRef) {\n",
          "    long baseptr = 0;\n"
          "    *($cbaseclass **)&baseptr = *($cclass **)&objectRef;\n"
          "    return baseptr;\n"
          "}\n",
          "\n",
          NIL); 

      Replaceall(upcasts_code, "$imbaseclass", baseclass);
      Replaceall(upcasts_code, "$cbaseclass",  c_baseclass);
      Replaceall(upcasts_code, "$imclazzname", proxy_class_name);
      Replaceall(upcasts_code, "$cclass",      c_classname);
    }
    Delete(baseclass);
  }

  /* ----------------------------------------------------------------------
   * classHandler()
   * ---------------------------------------------------------------------- */

  virtual int classHandler(Node *n) {

    File *f_proxy = NULL;
    if (proxy_flag) {
      proxy_class_name = NewString(Getattr(n,"sym:name"));

      if (!addSymbol(proxy_class_name,n)) return SWIG_ERROR;

      if (Cmp(proxy_class_name, imclass_name) == 0) {
        Printf(stderr, "Class name cannot be equal to intermediary class name: %s\n", proxy_class_name);
        SWIG_exit(EXIT_FAILURE);
      }

      if (Cmp(proxy_class_name, module_class_name) == 0) {
        Printf(stderr, "Class name cannot be equal to module class name: %s\n", proxy_class_name);
        SWIG_exit(EXIT_FAILURE);
      }

      String *filen = NewStringf("%s%s.cs", SWIG_output_directory(), proxy_class_name);
      f_proxy = NewFile(filen,"w");
      if(!f_proxy) {
        Printf(stderr, "Unable to create proxy class file: %s\n", filen);
        SWIG_exit(EXIT_FAILURE);
      }
      Delete(filen); filen = NULL;

      emitBanner(f_proxy);

      if(Len(package) > 0)
        Printf(f_proxy, "//package %s;\n\n", package);

      Clear(proxy_class_def);
      Clear(proxy_class_code);

      have_default_constructor_flag = false;
      destructor_call = NewString("");
      proxy_class_constants_code = NewString("");
    }
    Language::classHandler(n);

    if (proxy_flag) {

      emitProxyClassDefAndCPPCasts(n);

      Printv(f_proxy, proxy_class_def, proxy_class_code, NIL);

      // Write out all the enums and constants
      if (Len(proxy_class_constants_code) != 0 )
        Printv(f_proxy, "  // enums and constants\n", proxy_class_constants_code, NIL);

      Printf(f_proxy, "}\n");
      Close(f_proxy);
      f_proxy = NULL;

      Delete(proxy_class_name); proxy_class_name = NULL;
      Delete(destructor_call); destructor_call = NULL;
      Delete(proxy_class_constants_code); proxy_class_constants_code = NULL;
    }
    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * memberfunctionHandler()
   * ---------------------------------------------------------------------- */

  virtual int memberfunctionHandler(Node *n) {
    Language::memberfunctionHandler(n);

    if (proxy_flag) {
      String *overloaded_name = getOverloadedName(n);
      String *intermediary_function_name = Swig_name_member(proxy_class_name, overloaded_name);
      Setattr(n,"proxyfuncname", Getattr(n, "sym:name"));
      Setattr(n,"imfuncname", intermediary_function_name);
      proxyClassFunctionHandler(n);
      Delete(overloaded_name);
    }
    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * staticmemberfunctionHandler()
   * ---------------------------------------------------------------------- */

  virtual int staticmemberfunctionHandler(Node *n) {

    static_flag = true;
    Language::staticmemberfunctionHandler(n);

    if (proxy_flag) {
      String *overloaded_name = getOverloadedName(n);
      String *intermediary_function_name = Swig_name_member(proxy_class_name, overloaded_name);
      Setattr(n,"proxyfuncname", Getattr(n,"sym:name"));
      Setattr(n,"imfuncname", intermediary_function_name);
      proxyClassFunctionHandler(n);
      Delete(overloaded_name);
    }
    static_flag = false;

    return SWIG_OK;
  }


  /* -----------------------------------------------------------------------------
   * proxyClassFunctionHandler()
   *
   * Function called for creating a C# wrapper function around a c++ function in the 
   * proxy class. Used for both static and non-static C++ class functions.
   * C++ class static functions map to C# static functions.
   * Two extra attributes in the Node must be available. These are "proxyfuncname" - 
   * the name of the C# class proxy function, which in turn will call "imfuncname" - 
   * the intermediary (PInvoke) function name in the intermediary class.
   * ----------------------------------------------------------------------------- */

  void proxyClassFunctionHandler(Node *n) {
    SwigType  *t = Getattr(n,"type");
    ParmList  *l = Getattr(n,"parms");
    String    *intermediary_function_name = Getattr(n,"imfuncname");
    String    *proxy_function_name = Getattr(n,"proxyfuncname");
    String    *tm;
    Parm      *p;
    int       i;
    String    *imcall = NewString("");
    String    *return_type = NewString("");
    String    *function_code = NewString("");
    bool      setter_flag = false;

    if(!proxy_flag) return;

    if (l) {
      if (SwigType_type(Getattr(l,"type")) == T_VOID) {
        l = nextSibling(l);
      }
    }

    /* Attach the non-standard typemaps to the parameter list */
    Swig_typemap_attach_parms("in", l, NULL);
    Swig_typemap_attach_parms("cstype", l, NULL);
    Swig_typemap_attach_parms("csin", l, NULL);

    /* Get return types */
    if ((tm = Swig_typemap_lookup_new("cstype",n,"",0))) {
      substituteClassname(t, tm);
      Printf(return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
          "No cstype typemap defined for %s\n", SwigType_str(t,0));
    }

    if(proxy_flag && wrapping_member_flag && !enum_constant_flag) {
      // Properties
      setter_flag = (Cmp(Getattr(n, "sym:name"), Swig_name_set(Swig_name_member(proxy_class_name, variable_name))) == 0);
    }

    /* Start generating the proxy function */
    Printf(function_code, "  %s ", Getattr(n,"feature:cs:methodmodifiers"));
    if (static_flag)
      Printf(function_code, "static ");
    if (Getattr(n,"virtual:derived"))
        Printf(function_code, "override ");
    else if (checkAttribute(n, "storage", "virtual"))
        Printf(function_code, "virtual ");

    Printf(function_code, "%s %s(", return_type, proxy_function_name);

    Printv(imcall, imclass_name, ".", intermediary_function_name, "(", NIL);
    if (!static_flag)
      Printv(imcall, "swigCPtr", NIL);

    emit_mark_varargs(l);

    int gencomma = !static_flag;

    /* Output each parameter */
    for (i = 0, p=l; p; i++) {

      /* Ignored varargs */
      if (checkAttribute(p,"varargs:ignore","1")) {
        p = nextSibling(p);
        continue;
      }

      /* Ignored parameters */
      if (checkAttribute(p,"tmap:in:numinputs","0")) {
        p = Getattr(p,"tmap:in:next");
        continue;
      }

      /* Ignore the 'this' argument for variable wrappers */
      if (!(variable_wrapper_flag && i==0)) 
      {
        SwigType *pt = Getattr(p,"type");
        String   *param_type = NewString("");

        /* Get the C# parameter type */
        if ((tm = Getattr(p,"tmap:cstype"))) {
          substituteClassname(pt, tm);
          Printf(param_type, "%s", tm);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
              "No cstype typemap defined for %s\n", SwigType_str(pt,0));
        }

        if (gencomma)
          Printf(imcall, ", ");

        String *arg = variable_wrapper_flag ? NewString("value") : makeParameterName(n, p, i);

        // Use typemaps to transform type used in C# wrapper function (in proxy class) to type used in PInvoke function (in intermediary class)
        if ((tm = Getattr(p,"tmap:csin"))) {
          addThrows(n, "tmap:csin", p);
          substituteClassname(pt, tm);
          Replaceall(tm, "$csinput", arg);
          Printv(imcall, tm, NIL);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSIN_UNDEF, input_file, line_number, 
              "No csin typemap defined for %s\n", SwigType_str(pt,0));
        }

        /* Add parameter to proxy function */
        if (gencomma >= 2)
          Printf(function_code, ", ");
        gencomma = 2;
        Printf(function_code, "%s %s", param_type, arg);

        Delete(arg);
        Delete(param_type);
      }
      p = Getattr(p,"tmap:in:next");
    }

    Printf(imcall, ")");
    Printf(function_code, ")");

    // Transform return type used in PInvoke function (in intermediary class) to type used in C# wrapper function (in proxy class)
    if ((tm = Swig_typemap_lookup_new("csout",n,"",0))) {
      addThrows(n, "tmap:csout", n);
      if (Getattr(n,"feature:new"))
        Replaceall(tm,"$owner","true");
      else
        Replaceall(tm,"$owner","false");
      substituteClassname(t, tm);
      Replaceall(tm, "$imcall", imcall);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, 
          "No csout typemap defined for %s\n", SwigType_str(t,0));
    }

    generateThrowsClause(n, function_code);
    Printf(function_code, " %s\n\n", tm ? tm : empty_string);

    if(proxy_flag && wrapping_member_flag && !enum_constant_flag) {
      // Properties
      if(setter_flag) {
        // Setter method
        if ((tm = Swig_typemap_lookup_new("csvarin",n,"",0))) {
          if (Getattr(n,"feature:new"))
            Replaceall(tm,"$owner","true");
          else
            Replaceall(tm,"$owner","false");
          substituteClassname(t, tm);
          Replaceall(tm, "$imcall", imcall);
          Printf(proxy_class_code, "%s", tm);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, 
              "No csvarin typemap defined for %s\n", SwigType_str(t,0));
        }
      } else {
        // Getter method
        if ((tm = Swig_typemap_lookup_new("csvarout",n,"",0))) {
          if (Getattr(n,"feature:new"))
            Replaceall(tm,"$owner","true");
          else
            Replaceall(tm,"$owner","false");
          substituteClassname(t, tm);
          Replaceall(tm, "$imcall", imcall);
          Printf(proxy_class_code, "%s", tm);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, 
              "No csvarout typemap defined for %s\n", SwigType_str(t,0));
        }
      }
    } else {
      // Normal function call
      Printv(proxy_class_code, function_code, NIL);
    }

    Delete(function_code);
    Delete(return_type);
    Delete(imcall);
  }

  /* ----------------------------------------------------------------------
   * constructorHandler()
   * ---------------------------------------------------------------------- */

  virtual int constructorHandler(Node *n) {

    ParmList *l = Getattr(n,"parms");
    String    *tm;
    Parm      *p;
    int       i;

    Language::constructorHandler(n);

    if(proxy_flag) {
      String *overloaded_name = getOverloadedName(n);
      String *imcall = NewString("");

      Printf(proxy_class_code, "  %s %s(", Getattr(n,"feature:cs:methodmodifiers"), proxy_class_name);
      Printv(imcall, " : this(", imclass_name, ".", Swig_name_construct(overloaded_name), "(", NIL);

      /* Attach the non-standard typemaps to the parameter list */
      Swig_typemap_attach_parms("in", l, NULL);
      Swig_typemap_attach_parms("cstype", l, NULL);
      Swig_typemap_attach_parms("csin", l, NULL);

      emit_mark_varargs(l);

      int gencomma = 0;

      /* Output each parameter */
      for (i = 0, p=l; p; i++) {

        /* Ignored varargs */
        if (checkAttribute(p,"varargs:ignore","1")) {
          p = nextSibling(p);
          continue;
        }

        /* Ignored parameters */
        if (checkAttribute(p,"tmap:in:numinputs","0")) {
          p = Getattr(p,"tmap:in:next");
          continue;
        }

        SwigType *pt = Getattr(p,"type");
        String   *param_type = NewString("");

        /* Get the C# parameter type */
        if ((tm = Getattr(p,"tmap:cstype"))) {
          substituteClassname(pt, tm);
          Printf(param_type, "%s", tm);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
              "No cstype typemap defined for %s\n", SwigType_str(pt,0));
        }

        if (gencomma)
          Printf(imcall, ", ");

        String *arg = makeParameterName(n, p, i);

        // Use typemaps to transform type used in C# wrapper function (in proxy class) to type used in PInvoke function (in intermediary class)
        if ((tm = Getattr(p,"tmap:csin"))) {
          addThrows(n, "tmap:csin", p);
          substituteClassname(pt, tm);
          Replaceall(tm, "$csinput", arg);
          Printv(imcall, tm, NIL);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSIN_UNDEF, input_file, line_number, 
              "No csin typemap defined for %s\n", SwigType_str(pt,0));
        }

        /* Add parameter to proxy function */
        if(gencomma)
          Printf(proxy_class_code, ", ");
        Printf(proxy_class_code, "%s %s", param_type, arg);
        gencomma = 1;

        Delete(arg);
        Delete(param_type);
        p = Getattr(p,"tmap:in:next");
      }

      Printf(imcall, "), true)");

      Printf(proxy_class_code, ")");
      Printf(proxy_class_code,"%s", imcall);
      generateThrowsClause(n, proxy_class_code);
      Printf(proxy_class_code, " {\n");
      Printf(proxy_class_code, "  }\n\n");

      if(!gencomma)  // We must have a default constructor
        have_default_constructor_flag = true;

      Delete(overloaded_name);
      Delete(imcall);
    }

    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * destructorHandler()
   * ---------------------------------------------------------------------- */

  virtual int destructorHandler(Node *n) {
    Language::destructorHandler(n);
    String *symname = Getattr(n,"sym:name");

    if(proxy_flag) {
      Printv(destructor_call, imclass_name, ".", Swig_name_destroy(symname), "(swigCPtr)", NIL);
    }
    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * membervariableHandler()
   * ---------------------------------------------------------------------- */

  virtual int membervariableHandler(Node *n) {
    SwigType  *t = Getattr(n,"type");
    String    *tm;

    // Get the variable type
    if ((tm = Swig_typemap_lookup_new("cstype",n,"",0))) {
      substituteClassname(t, tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
          "No cstype typemap defined for %s\n", SwigType_str(t,0));
    }

    // Output the property's field declaration and accessor methods
    Printf(proxy_class_code, "  public %s %s {", tm, Getattr(n, "sym:name"));

    variable_name = Getattr(n,"sym:name");
    wrapping_member_flag = true;
    variable_wrapper_flag = true;
    Language::membervariableHandler(n);
    wrapping_member_flag = false;
    variable_wrapper_flag = false;

    Printf(proxy_class_code, "\n  }\n\n");

    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * staticmembervariableHandler()
   * ---------------------------------------------------------------------- */

  virtual int staticmembervariableHandler(Node *n) {

    bool static_const_member_flag = (Getattr(n, "value") == 0);
    if(static_const_member_flag) {
      SwigType  *t = Getattr(n,"type");
      String    *tm;

      // Get the variable type
      if ((tm = Swig_typemap_lookup_new("cstype",n,"",0))) {
        substituteClassname(t, tm);
      } else {
        Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
            "No cstype typemap defined for %s\n", SwigType_str(t,0));
      }

      // Output the property's field declaration and accessor methods
      Printf(proxy_class_code, "  public static %s %s {", tm, Getattr(n, "sym:name"));
    }

    variable_name = Getattr(n,"sym:name");
    wrapping_member_flag = true;
    static_flag = true;
    Language::staticmembervariableHandler(n);
    wrapping_member_flag = false;
    static_flag = false;

    if(static_const_member_flag)
      Printf(proxy_class_code, "\n  }\n\n");

    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * memberconstantHandler()
   * ---------------------------------------------------------------------- */

  virtual int memberconstantHandler(Node *n) {
    variable_name = Getattr(n,"sym:name");
    wrapping_member_flag = true;
    Language::memberconstantHandler(n);
    wrapping_member_flag = false;
    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------------
   * getOverloadedName()
   * ----------------------------------------------------------------------------- */

  String *getOverloadedName(Node *n) {

    /* Although PInvoke functions are designed to handle overloaded functions, a C# IntPtr is used for all classes in the SWIG
     * intermediary class. The intermediary class methods are thus mangled when overloaded to give a unique name. */
    String *overloaded_name = NewStringf("%s", Getattr(n,"sym:name"));

    if (Getattr(n,"sym:overloaded")) {
      Printv(overloaded_name, Getattr(n,"sym:overname"), NIL);
    }

    return overloaded_name;
  }

  /* -----------------------------------------------------------------------------
   * moduleClassFunctionHandler()
   * ----------------------------------------------------------------------------- */

  void moduleClassFunctionHandler(Node *n) {
    SwigType  *t = Getattr(n,"type");
    ParmList  *l = Getattr(n,"parms");
    String    *tm;
    Parm      *p;
    int       i;
    String    *imcall = NewString("");
    String    *return_type = NewString("");
    String    *function_code = NewString("");
    int       num_arguments = 0;
    int       num_required = 0;
    String    *overloaded_name = getOverloadedName(n);
    String    *func_name = NULL;
    bool      setter_flag = false;

    if (l) {
      if (SwigType_type(Getattr(l,"type")) == T_VOID) {
        l = nextSibling(l);
      }
    }

    /* Attach the non-standard typemaps to the parameter list */
    Swig_typemap_attach_parms("cstype", l, NULL);
    Swig_typemap_attach_parms("csin", l, NULL);

    /* Get return types */
    if ((tm = Swig_typemap_lookup_new("cstype",n,"",0))) {
      substituteClassname(t, tm);
      Printf(return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
          "No cstype typemap defined for %s\n", SwigType_str(t,0));
    }

    /* Change function name for global variables */
    if (proxy_flag && global_variable_flag) {
      // Capitalize the first letter in the variable to create the getter/setter function name
      func_name = NewString("");
      setter_flag = (Cmp(Getattr(n,"sym:name"), Swig_name_set(variable_name)) == 0);
      if(setter_flag)
        Printf(func_name,"set");
      else 
        Printf(func_name,"get");
      Putc(toupper((int) *Char(variable_name)), func_name);
      Printf(func_name, "%s", Char(variable_name)+1);
    } else {
      func_name = Copy(Getattr(n,"sym:name"));
    }

    /* Start generating the function */
    Printf(function_code, "  %s static %s %s(", Getattr(n,"feature:cs:methodmodifiers"), return_type, func_name);
    Printv(imcall, imclass_name, ".", overloaded_name, "(", NIL);

    /* Get number of required and total arguments */
    num_arguments = emit_num_arguments(l);
    num_required  = emit_num_required(l);

    int gencomma = 0;

    /* Output each parameter */
    for (i = 0, p=l; i < num_arguments; i++) {

      /* Ignored parameters */
      while (checkAttribute(p,"tmap:in:numinputs","0")) {
        p = Getattr(p,"tmap:in:next");
      }

      SwigType *pt = Getattr(p,"type");
      String   *param_type = NewString("");

      /* Get the C# parameter type */
      if ((tm = Getattr(p,"tmap:cstype"))) {
        substituteClassname(pt, tm);
        Printf(param_type, "%s", tm);
      } else {
        Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, 
            "No cstype typemap defined for %s\n", SwigType_str(pt,0));
      }

      if (gencomma)
        Printf(imcall, ", ");

      String *arg = makeParameterName(n, p, i);

      // Use typemaps to transform type used in C# wrapper function (in proxy class) to type used in PInvoke function (in intermediary class)
      if ((tm = Getattr(p,"tmap:csin"))) {
        addThrows(n, "tmap:csin", p);
        substituteClassname(pt, tm);
        Replaceall(tm, "$csinput", arg);
        Printv(imcall, tm, NIL);
      } else {
        Swig_warning(WARN_CSHARP_TYPEMAP_CSIN_UNDEF, input_file, line_number, 
            "No csin typemap defined for %s\n", SwigType_str(pt,0));
      }

      /* Add parameter to module class function */
      if (gencomma >= 2)
        Printf(function_code, ", ");
      gencomma = 2;
      Printf(function_code, "%s %s", param_type, arg);

      p = Getattr(p,"tmap:in:next");
      Delete(arg);
      Delete(param_type);
    }

    Printf(imcall, ")");
    Printf(function_code, ")");

    // Transform return type used in PInvoke function (in intermediary class) to type used in C# wrapper function (in module class)
    if ((tm = Swig_typemap_lookup_new("csout",n,"",0))) {
      addThrows(n, "tmap:csout", n);
      if (Getattr(n,"feature:new"))
        Replaceall(tm,"$owner","true");
      else
        Replaceall(tm,"$owner","false");
      substituteClassname(t, tm);
      Replaceall(tm, "$imcall", imcall);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, 
          "No csout typemap defined for %s\n", SwigType_str(t,0));
    }

    generateThrowsClause(n, function_code);
    Printf(function_code, " %s\n\n", tm ? tm : empty_string);

    if (proxy_flag && global_variable_flag) {
      // Properties
      if(setter_flag) {
        // Setter method
        if ((tm = Swig_typemap_lookup_new("csvarin",n,"",0))) {
          if (Getattr(n,"feature:new"))
            Replaceall(tm,"$owner","true");
          else
            Replaceall(tm,"$owner","false");
          substituteClassname(t, tm);
          Replaceall(tm, "$imcall", imcall);
          Printf(module_class_code, "%s", tm);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, 
              "No csvarin typemap defined for %s\n", SwigType_str(t,0));
        }
      } else {
        // Getter method
        if ((tm = Swig_typemap_lookup_new("csvarout",n,"",0))) {
          if (Getattr(n,"feature:new"))
            Replaceall(tm,"$owner","true");
          else
            Replaceall(tm,"$owner","false");
          substituteClassname(t, tm);
          Replaceall(tm, "$imcall", imcall);
          Printf(module_class_code, "%s", tm);
        } else {
          Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, 
              "No csvarout typemap defined for %s\n", SwigType_str(t,0));
        }
      }
    } else {
      // Normal function call
      Printv(module_class_code, function_code, NIL);
    }

    Delete(function_code);
    Delete(return_type);
    Delete(imcall);
    Delete(func_name);
  }

  /* -----------------------------------------------------------------------------
   * substituteClassname()
   *
   * Substitute $csclassname with the proxy class name for classes/structs/unions that SWIG knows about.
   * Otherwise use the $descriptor name for the C# class name. Note that the $&csclassname substitution
   * is the same as a $&descriptor substitution, ie one pointer added to descriptor name.
   * Inputs:
   *   pt - parameter type
   *   tm - cstype typemap
   * Outputs:
   *   tm - cstype typemap with $csclassname substitution
   * Return:
   *   substitution_performed - flag indicating if a substitution was performed
   * ----------------------------------------------------------------------------- */

  bool substituteClassname(SwigType *pt, String *tm) {
    bool substitution_performed = false;
    if (Strstr(tm, "$csclassname") || Strstr(tm,"$&csclassname")) {
      String *classname = getProxyName(pt);
      if (classname) {
        Replaceall(tm,"$&csclassname", classname); // getProxyName() works for pointers to classes too
        Replaceall(tm,"$csclassname", classname);
      }
      else { // use $descriptor if SWIG does not know anything about this type. Note that any typedefs are resolved.
        String *descriptor = NULL;
        SwigType *type = Copy(SwigType_typedef_resolve_all(pt));

        if (Strstr(tm, "$&csclassname")) {
          SwigType_add_pointer(type);
          descriptor = NewStringf("SWIGTYPE%s", SwigType_manglestr(type));
          Replaceall(tm, "$&csclassname", descriptor);
        }
        else { // $csclassname
          descriptor = NewStringf("SWIGTYPE%s", SwigType_manglestr(type));
          Replaceall(tm, "$csclassname", descriptor);
        }

        // Add to hash table so that the type wrapper classes can be created later
        Setattr(swig_types_hash, descriptor, type);
        Delete(descriptor);
        Delete(type);
      }
      substitution_performed = true;
    }
    return substitution_performed;
  }

  /* -----------------------------------------------------------------------------
   * makeParameterName()
   *
   * Inputs: 
   *   n - Node
   *   p - parameter node
   *   arg_num - parameter argument number
   * Return:
   *   arg - a unique parameter name
   * ----------------------------------------------------------------------------- */

  String *makeParameterName(Node *n, Parm *p, int arg_num) {

    // Use C parameter name unless it is a duplicate or an empty parameter name
    String   *pn = Getattr(p,"name");
    int count = 0;
    ParmList *plist = Getattr(n,"parms");
    while (plist) {
      if ((Cmp(pn, Getattr(plist,"name")) == 0))
        count++;
      plist = nextSibling(plist);
    }
    String *arg = (!pn || (count > 1)) ? NewStringf("arg%d",arg_num) : Copy(Getattr(p,"name"));

    return arg;
  }

  /* -----------------------------------------------------------------------------
   * emitTypeWrapperClass()
   * ----------------------------------------------------------------------------- */

  void emitTypeWrapperClass(String *classname, SwigType *type) {
    String *filen = NewStringf("%s%s.cs", SWIG_output_directory(), classname);
    File *f_swigtype = NewFile(filen,"w");
    String *swigtype = NewString("");

    // Emit banner and package name
    emitBanner(f_swigtype);
    if(Len(package) > 0)
      Printf(f_swigtype, "//package %s;\n\n", package);

    // Pure C# baseclass and interfaces
    const String *pure_baseclass = typemapLookup("csbase", type, WARN_NONE);
    const String *pure_interfaces = typemapLookup("csinterfaces", type, WARN_NONE);

    // Emit the class
    Printv(swigtype,
        typemapLookup("csimports", type, WARN_NONE), // Import statements
        "\n",
        typemapLookup("csclassmodifiers", type, WARN_CSHARP_TYPEMAP_CLASSMOD_UNDEF), // Class modifiers
        " class $csclassname",       // Class name and bases
        *Char(pure_baseclass) ?
        " : " : 
        "",
        pure_baseclass,
        *Char(pure_interfaces) ?  // Interfaces
        " : " :
        "",
        pure_interfaces,
        " {\n",
        "  private IntPtr swigCPtr;\n",
        "\n",
        "  ",
        typemapLookup("csptrconstructormodifiers", type, WARN_CSHARP_TYPEMAP_PTRCONSTMOD_UNDEF), // pointer constructor modifiers
        " $csclassname(IntPtr cPtr, bool bFutureUse) {\n", // Constructor used for wrapping pointers
        "    swigCPtr = cPtr;\n",
        "  }\n",
        "\n",
        "  protected $csclassname() {\n", // Default constructor
        "    swigCPtr = IntPtr.Zero;\n",
        "  }\n",
        typemapLookup("csgetcptr", type, WARN_CSHARP_TYPEMAP_GETCPTR_UNDEF), // getCPtr method
        typemapLookup("cscode", type, WARN_NONE), // extra C# code
        "}\n",
        "\n",
        NIL);

        Replaceall(swigtype, "$csclassname", classname);
        Printv(f_swigtype, swigtype, NIL);

        Close(f_swigtype);
        Delete(filen);
        Delete(swigtype);
  }

  /* -----------------------------------------------------------------------------
   * typemapLookup()
   * ----------------------------------------------------------------------------- */

  const String *typemapLookup(const String *op, String *type, int warning, Node *typemap_attributes=NULL) {
    String *tm = NULL;
    const String *code = NULL;

    if((tm = Swig_typemap_search(op, type, NULL, NULL))) {
      code = Getattr(tm,"code");
      if (typemap_attributes)
        Swig_typemap_attach_kwargs(tm,op,typemap_attributes);
    }

    if (!code) {
      code = empty_string;
      if (warning != WARN_NONE)
        Swig_warning(warning, input_file, line_number, "No %s typemap defined for %s\n", op, type);
    }

    return code ? code : empty_string;
  }

  /* -----------------------------------------------------------------------------
   * addThrows()
   * ----------------------------------------------------------------------------- */

  void addThrows(Node *n, const String *typemap, Node *parameter) {
    // Get the comma separated throws clause - held in "throws" attribute in the typemap passed in
    String *throws_attribute = NewStringf("%s:throws", typemap);
    String *throws = Getattr(parameter,throws_attribute);

    if (throws) {
      String *throws_list = Getattr(n,"csharp:throwslist");
      if (!throws_list) {
        throws_list = NewList();
        Setattr(n,"csharp:throwslist", throws_list);
      }

      // Put the exception classes in the throws clause into a temporary List
      List *temp_classes_list = Split(throws,',',INT_MAX);

      // Add the exception classes to the node throws list, but don't duplicate if already in list
      if (temp_classes_list && Len(temp_classes_list) > 0) {
        for (String *cls = Firstitem(temp_classes_list); cls; cls = Nextitem(temp_classes_list)) {
          String *exception_class = NewString(cls);
          Replaceall(exception_class," ","");  // remove spaces
          Replaceall(exception_class,"\t",""); // remove tabs
          if (Len(exception_class) > 0) {
            // $csclassname substitution
            SwigType *pt = Getattr(parameter,"type");
            substituteClassname(pt, exception_class);

            // Don't duplicate the C# exception class in the throws clause
            bool found_flag = false;
            for (String *item = Firstitem(throws_list); item; item = Nextitem(throws_list)) {
              if (Strcmp(item, exception_class) == 0)
                found_flag = true;
            }
            if (!found_flag)
              Append(throws_list, exception_class);
          }
          Delete(exception_class);
        } 
      }
      Delete(temp_classes_list);
    } 
    Delete(throws_attribute);
  }

  /* -----------------------------------------------------------------------------
   * generateThrowsClause()
   * ----------------------------------------------------------------------------- */

  void generateThrowsClause(Node *n, String *code) {
    // Add the throws clause into code
    List *throws_list = Getattr(n,"csharp:throwslist");
    if (throws_list) {
      String *cls = Firstitem(throws_list);
      Printf(code, " throws %s", cls);
      while ( (cls = Nextitem(throws_list)) )
        Printf(code, ", %s", cls);
    }
  }

};   /* class CSHARP */

/* -----------------------------------------------------------------------------
 * swig_csharp()    - Instantiate module
 * ----------------------------------------------------------------------------- */

extern "C" Language *
swig_csharp(void) {
  return new CSHARP();
}

/* -----------------------------------------------------------------------------
 * Static member variables
 * ----------------------------------------------------------------------------- */

const char *CSHARP::usage = (char*)"\
C# Options (available with -csharp)\n\
     -package <name> - set name of the assembly to <name>\n\
     -noproxy        - Generate the low-level functional interface instead\n\
                       of proxy classes\n\
\n";

