/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * d.cxx
 *
 * D language module for SWIG.
 * ----------------------------------------------------------------------------- */

char cvsroot_d_cxx[] = "$Id$";

#include "swigmod.h"
#include "cparse.h"
#include <ctype.h>

/* Hash type used for upcalls from C/C++ */
typedef DOH UpcallData;

class D:public Language {
  static const char *usage;
  const String *empty_string;
  const String *public_string;
  const String *protected_string;
  const String *static_string;

  Hash *swig_types_hash;
  File *f_begin;
  File *f_runtime;
  File *f_runtime_h;
  File *f_header;
  File *f_wrappers;
  File *f_init;
  File *f_directors;
  File *f_directors_h;
  List *filenames_list;

  bool proxy_flag;		// Flag for generating proxy classes
  bool native_function_flag;	// Flag for when wrapping a native function
  bool static_flag;		// Flag for when wrapping a static functions or member variables
  bool variable_wrapper_flag;	// Flag for when wrapping a nonstatic member variable
  bool wrapping_member_flag;	// Flag for when wrapping a member variable/enum/const
  bool global_variable_flag;	// Flag for when wrapping a global variable

  String *wrap_dmodule_name;	// The name of the D module containing the interface to the C wrapper.
  String *wrap_dmodule_fq_name;	// The fully qualified name of the wrap D module (package name included).
  String *proxy_dmodule_name;	// The name of the proxy module which exposes the (SWIG) module contents as a D module.
  String *proxy_dmodule_fq_name;// The fully qualified name of the proxy D module.
  String *wrap_dmodule_code;	// The D code declaring the wrapper functions.
  String *proxy_class_def;
  String *proxy_class_code;	// The D code making up the body of a proxy class (written to the proxy D module).
  String *proxy_class_epilogue_code; // D code which is emitted to the proxy D module after the class definition.
  String *proxy_dmodule_code;	// The D code for proxy functions/classes which is written to the proxy D module.
  String *proxy_class_name;
  String *variable_name;	//Name of a variable being wrapped
  String *proxy_class_enums_code;
  String *module_class_constants_code;
  String *enum_code;
  String *wrap_library_name;		// The name of the library which contains the C wrapper (used for dynamic linking).
  String *package;		// Optional: Package the D modules are placed in.
  String *dmodule_directory;	// The directory the generated D module files are written to.
  String *wrap_dmodule_imports;	// Imports for the wrap D module from %pragma.
  String *proxy_dmodule_imports;	// Imports for the proxy D module from %pragma.
  String *upcasts_code;		//C++ casts for inheritance hierarchies C++ code
  String *wrap_dmodule_cppcasts_code;	//C++ casts up inheritance hierarchies intermediary class code
  String *director_callback_typedefs;	// Director function pointer typedefs for callbacks
  String *director_callbacks;	// Director callback function pointer member variables
  String *director_dcallbacks_code;	// Director callback method that delegates are set to call
  String *director_delegate_definitions;	// Director delegates definitions in proxy class
  String *director_delegate_instances;	// Director delegates member variables in proxy class
  String *director_method_types;	// Director method types
  String *director_connect_parms;	// Director delegates parameter list for director connect call
  String *destructor_call;	//C++ destructor call if any

  // Dynamic linking:
  String *wrapper_loader_code;		// The D code which is inserted into the wrapper module if dynamic linking is used.
  String *wrapper_loader_bind_command;	// The D code to bind a function pointer to a library symbol.
  String *wrapper_loader_bind_code;	// The cumulated binding commands for the wrapper library.

  // Director method stuff:
  List *dmethods_seq;
  Hash *dmethods_table;
  int n_dmethods;
  int n_directors;
  int first_class_dmethod;
  int curr_class_dmethod;

  static Parm *NewParmFromNode(SwigType *type, const_String_or_char_ptr name, Node *n) {
    Parm *p = NewParm(type, name);
    Setfile(p, Getfile(n));
    Setline(p, Getline(n));
    return p;
  }

public:

  /* ---------------------------------------------------------------------------
   * D::D()
   * --------------------------------------------------------------------------- */
   D():empty_string(NewString("")),
      public_string(NewString("public")),
      protected_string(NewString("protected")),
      swig_types_hash(NULL),
      f_begin(NULL),
      f_runtime(NULL),
      f_runtime_h(NULL),
      f_header(NULL),
      f_wrappers(NULL),
      f_init(NULL),
      f_directors(NULL),
      f_directors_h(NULL),
      filenames_list(NULL),
      proxy_flag(true),
      native_function_flag(false),
      static_flag(false),
      variable_wrapper_flag(false),
      wrapping_member_flag(false),
      global_variable_flag(false),
      wrap_dmodule_name(NULL),
      wrap_dmodule_fq_name(NULL),
      proxy_dmodule_name(NULL),
      proxy_dmodule_fq_name(NULL),
      wrap_dmodule_code(NULL),
      proxy_class_def(NULL),
      proxy_class_code(NULL),
      proxy_class_epilogue_code(NULL),
      proxy_dmodule_code(NULL),
      proxy_class_name(NULL),
      variable_name(NULL),
      proxy_class_enums_code(NULL),
      module_class_constants_code(NULL),
      enum_code(NULL),
      wrap_library_name(NULL),
      package(NULL),
      dmodule_directory(NULL),
      wrap_dmodule_imports(NULL),
      proxy_dmodule_imports(NULL),
      upcasts_code(NULL),
      wrap_dmodule_cppcasts_code(NULL),
      director_callback_typedefs(NULL),
      director_callbacks(NULL),
      director_dcallbacks_code(NULL),
      director_delegate_definitions(NULL),
      director_delegate_instances(NULL),
      director_method_types(NULL),
      director_connect_parms(NULL),
      destructor_call(NULL),
      wrapper_loader_code(NULL),
      wrapper_loader_bind_command(NULL),
      wrapper_loader_bind_code(NULL),
      dmethods_seq(NULL),
      dmethods_table(NULL),
      n_dmethods(0),
      n_directors(0) {
    /* for now, multiple inheritance in directors is disabled, this
       should be easy to implement though */
    director_multiple_inheritance = 0;
    director_language = 1;
  }

  /* ---------------------------------------------------------------------------
   * D::getProxyName()
   *
   * Test to see if a type corresponds to something wrapped with a proxy class
   * Return NULL if not otherwise the proxy class name
   * --------------------------------------------------------------------------- */
   String *getProxyName(SwigType *t) {
    if (proxy_flag) {
      Node *n = classLookup(t);
      if (n) {
	return Getattr(n, "sym:name");
      }
    }
    return NULL;
  }

  /* ---------------------------------------------------------------------------
   * D::directorClassName()
   * --------------------------------------------------------------------------- */
  String *directorClassName(Node *n) {
    String *dirclassname;
    const char *attrib = "director:classname";

    if (!(dirclassname = Getattr(n, attrib))) {
      String *classname = Getattr(n, "sym:name");

      dirclassname = NewStringf("SwigDirector_%s", classname);
      Setattr(n, attrib, dirclassname);
    }

    return dirclassname;
  }

  /* ---------------------------------------------------------------------------
   * D::main()
   * --------------------------------------------------------------------------- */
  virtual void main(int argc, char *argv[]) {
    SWIG_library_directory("d");

    // Look for certain command line options
    for (int i = 1; i < argc; i++) {
      if (argv[i]) {
	if (strcmp(argv[i], "-wrapperlibrary") == 0) {
	  if (argv[i + 1]) {
	    wrap_library_name = NewString("");
	    Printf(wrap_library_name, argv[i + 1]);
	    Swig_mark_arg(i);
	    Swig_mark_arg(i + 1);
	    i++;
	  } else {
	    Swig_arg_error();
	  }
	} else if (strcmp(argv[i], "-package") == 0) {
	  if (argv[i + 1]) {
	    package = NewString("");
	    Printf(package, argv[i + 1]);
	    Swig_mark_arg(i);
	    Swig_mark_arg(i + 1);
	    i++;
	  } else {
	    Swig_arg_error();
	  }
	} else if ((strcmp(argv[i], "-noproxy") == 0)) {
	  Swig_mark_arg(i);
	  proxy_flag = false;
	} else if (strcmp(argv[i], "-help") == 0) {
	  Printf(stdout, "%s\n", usage);
	}
      }
    }

    // Add a symbol to the parser for conditional compilation
    Preprocessor_define("SWIGD 1", 0);

    // Add typemap definitions
    SWIG_typemap_lang("d");
    SWIG_config_file("d.swg");

    allow_overloading();
  }

  /* ---------------------------------------------------------------------------
   * D::top()
   * --------------------------------------------------------------------------- */
  virtual int top(Node *n) {
    // Get any options set in the module directive
    Node *optionsnode = Getattr(Getattr(n, "module"), "options");

    if (optionsnode) {
      if (Getattr(optionsnode, "imclassname"))
	wrap_dmodule_name = Copy(Getattr(optionsnode, "imclassname"));
      /* check if directors are enabled for this module.  note: this
       * is a "master" switch, without which no director code will be
       * emitted.  %feature("director") statements are also required
       * to enable directors for individual classes or methods.
       *
       * use %module(directors="1") modulename at the start of the
       * interface file to enable director generation.
       */
      if (Getattr(optionsnode, "directors")) {
	allow_directors();
      }
      if (Getattr(optionsnode, "dirprot")) {
	allow_dirprot();
      }
      allow_allprotected(GetFlag(optionsnode, "allprotected"));
    }

    /* Initialize all of the output files */
    String *outfile = Getattr(n, "outfile");
    String *outfile_h = Getattr(n, "outfile_h");

    if (!outfile) {
      Printf(stderr, "Unable to determine outfile\n");
      SWIG_exit(EXIT_FAILURE);
    }

    f_begin = NewFile(outfile, "w", SWIG_output_files());
    if (!f_begin) {
      FileErrorDisplay(outfile);
      SWIG_exit(EXIT_FAILURE);
    }

    if (directorsEnabled()) {
      if (!outfile_h) {
        Printf(stderr, "Unable to determine outfile_h\n");
        SWIG_exit(EXIT_FAILURE);
      }
      f_runtime_h = NewFile(outfile_h, "w", SWIG_output_files());
      if (!f_runtime_h) {
	FileErrorDisplay(outfile_h);
	SWIG_exit(EXIT_FAILURE);
      }
    }

    f_runtime = NewString("");
    f_init = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");
    f_directors_h = NewString("");
    f_directors = NewString("");

    /* Register file targets with the SWIG file handler */
    Swig_register_filebyname("header", f_header);
    Swig_register_filebyname("wrapper", f_wrappers);
    Swig_register_filebyname("begin", f_begin);
    Swig_register_filebyname("runtime", f_runtime);
    Swig_register_filebyname("init", f_init);
    Swig_register_filebyname("director", f_directors);
    Swig_register_filebyname("director_h", f_directors_h);

    swig_types_hash = NewHash();
    filenames_list = NewList();

    // Make the package name and the resulting module output path.
    if (package) {
      // Append a dot so we can prepend the package variable directly to the
      // module names in the rest of the code.
      Printv( package, ".", NIL );
    } else {
      // Write the generated D modules to the »root« package by default.
      package = NewString("");
    }

    dmodule_directory = Copy(SWIG_output_directory());
    if (Len(package) > 0) {
      String *package_directory = Copy(package);
      Replaceall(package_directory, ".", SWIG_FILE_DELIMITER);
      Printv(dmodule_directory, package_directory, NIL);

      // TODO: Add some facilities to DOH for automatically creating directories.
      String *create_directory_command = NewStringf( "mkdir -p %s", package_directory );
      system(Char(create_directory_command));
      Delete(create_directory_command);

      Delete(package_directory);
    }

    // Make the wrap and proxy D module names.
    // The wrap module name can be set in the module directive.
    if (!wrap_dmodule_name) {
      wrap_dmodule_name = NewStringf("%s_wrap", Getattr(n, "name"));
    }
    wrap_dmodule_fq_name = NewStringf("%s%s", package, wrap_dmodule_name);
    proxy_dmodule_name = Copy(Getattr(n, "name"));
    proxy_dmodule_fq_name = NewStringf("%s%s", package, proxy_dmodule_name);

    wrap_dmodule_code = NewString("");
    proxy_class_def = NewString("");
    proxy_class_code = NewString("");
    proxy_class_epilogue_code = NewString("");
    module_class_constants_code = NewString("");
    proxy_dmodule_code = NewString("");
    proxy_dmodule_imports = NewString("");
    wrap_dmodule_imports = NewString("");
    wrap_dmodule_cppcasts_code = NewString("");
    director_connect_parms = NewString("");
    upcasts_code = NewString("");
    wrapper_loader_code = NewString("");
    wrapper_loader_bind_command = NewString("");
    wrapper_loader_bind_code = NewString("");
    dmethods_seq = NewList();
    dmethods_table = NewHash();
    n_dmethods = 0;
    n_directors = 0;

    // By default, expect the dynamically loaded wrapper library to be named
    // like the wrapper D module (i.e. [lib]<module>_wrap[.so/.dll] unless the
    // user overrides it).
    if (!wrap_library_name)
      wrap_library_name = Copy(wrap_dmodule_name);

    Swig_banner(f_begin);

    Printf(f_runtime, "\n");
    Printf(f_runtime, "#define SWIGD\n");

    if (directorsEnabled()) {
      Printf(f_runtime, "#define SWIG_DIRECTORS\n");

      /* Emit initial director header and director code: */
      Swig_banner(f_directors_h);
      Printf(f_directors_h, "\n");
      Printf(f_directors_h, "#ifndef SWIG_%s_WRAP_H_\n", proxy_dmodule_name);
      Printf(f_directors_h, "#define SWIG_%s_WRAP_H_\n\n", proxy_dmodule_name);

      Printf(f_directors, "\n\n");
      Printf(f_directors, "/* ---------------------------------------------------\n");
      Printf(f_directors, " * C++ director class methods\n");
      Printf(f_directors, " * --------------------------------------------------- */\n\n");
      if (outfile_h)
	Printf(f_directors, "#include \"%s\"\n\n", Swig_file_filename(outfile_h));
    }

    Printf(f_runtime, "\n");

    Swig_name_register("wrapper", "D_%f");

    Printf(f_wrappers, "\n#ifdef __cplusplus\n");
    Printf(f_wrappers, "extern \"C\" {\n");
    Printf(f_wrappers, "#endif\n\n");

    /* Emit code */
    Language::top(n);

    if (directorsEnabled()) {
      // Insert director runtime into the f_runtime file (make it occur before %header section)
      Swig_insert_file("director.swg", f_runtime);
    }

    // Generate the wrap D module.
    // TODO: Add support for »static« linking.
    {
      String *filen = NewStringf("%s%s.d", dmodule_directory, wrap_dmodule_name);
      File *f_wrapd = NewFile(filen, "w", SWIG_output_files());
      if (!f_wrapd) {
	FileErrorDisplay(filen);
	SWIG_exit(EXIT_FAILURE);
      }
      Append(filenames_list, Copy(filen));
      Delete(filen);
      filen = NULL;

      // Start writing out the intermediary class file.
      emitBanner(f_wrapd);

      Printf(f_wrapd, "module %s;\n", wrap_dmodule_fq_name);

      // RESEARCH: Can this actually be null?
      Printv(f_wrapd, wrap_dmodule_imports, NIL);

      Replaceall(wrapper_loader_code, "$wraplibrary", wrap_library_name);
      Replaceall(wrapper_loader_code, "$wrapperloaderbindcode", wrapper_loader_bind_code);
      Replaceall(wrapper_loader_code, "$module", proxy_dmodule_name);
      Printf(f_wrapd, "%s\n", wrapper_loader_code);

      // Add the wrapper function declarations.
      Replaceall(wrap_dmodule_code, "$proxydmodule", proxy_dmodule_fq_name);
      Replaceall(wrap_dmodule_code, "$wrapdmodule", wrap_dmodule_fq_name);
      Replaceall(wrap_dmodule_code, "$module", proxy_dmodule_name);
      Printv(f_wrapd, wrap_dmodule_code, NIL);

      Close(f_wrapd);
    }

    // Generate the D proxy module for the wrapped module.
    {
      String *filen = NewStringf("%s%s.d", dmodule_directory, proxy_dmodule_name);
      File *f_module = NewFile(filen, "w", SWIG_output_files());
      if (!f_module) {
        FileErrorDisplay(filen);
        SWIG_exit(EXIT_FAILURE);
      }
      Append(filenames_list, Copy(filen));
      Delete(filen);
      filen = NULL;

      // Start writing out the module class file.
      emitBanner(f_module);

      Printf(f_module, "module %s;\n\n", proxy_dmodule_fq_name);

      Printf(f_module, "static import %s;\n", wrap_dmodule_fq_name);

      if (proxy_dmodule_imports)
        Printf(f_module, "%s\n", proxy_dmodule_imports);

      Replaceall(proxy_dmodule_code, "$proxydmodule", proxy_dmodule_fq_name);
      Replaceall(module_class_constants_code, "$proxydmodule", proxy_dmodule_fq_name);

      Replaceall(proxy_dmodule_code, "$wrapdmodule", wrap_dmodule_fq_name);
      Replaceall(module_class_constants_code, "$wrapdmodule", wrap_dmodule_fq_name);

      Replaceall(proxy_dmodule_code, "$module", proxy_dmodule_name);
      Replaceall(module_class_constants_code, "$module", proxy_dmodule_name);

      // Output a D type wrapper class for each SWIG type
      for (Iterator swig_type = First(swig_types_hash); swig_type.key; swig_type = Next(swig_type)) {
	writeTypeWrapperClass(swig_type.key, swig_type.item);
      }

      // Add the proxy functions.
      Printv(f_module, proxy_dmodule_code, NIL);

      // Write out all the global constants
      Printv(f_module, module_class_constants_code, NIL);

      Close(f_module);
    }

    if (upcasts_code)
      Printv(f_wrappers, upcasts_code, NIL);

    Printf(f_wrappers, "#ifdef __cplusplus\n");
    Printf(f_wrappers, "}\n");
    Printf(f_wrappers, "#endif\n");

    // Check for overwriting file problems on filesystems that are case insensitive
    Iterator it1;
    Iterator it2;
    for (it1 = First(filenames_list); it1.item; it1 = Next(it1)) {
      String *item1_lower = Swig_string_lower(it1.item);
      for (it2 = Next(it1); it2.item; it2 = Next(it2)) {
	String *item2_lower = Swig_string_lower(it2.item);
	if (it1.item && it2.item) {
	  if (Strcmp(item1_lower, item2_lower) == 0) {
	    Swig_warning(WARN_LANG_PORTABILITY_FILENAME, input_file, line_number,
			 "Portability warning: File %s will be overwritten by %s on case insensitive filesystems such as "
			 "Windows' FAT32 and NTFS unless the class/module name is renamed\n", it1.item, it2.item);
	  }
	}
	Delete(item2_lower);
      }
      Delete(item1_lower);
    }

    Delete(swig_types_hash);
    swig_types_hash = NULL;
    Delete(filenames_list);
    filenames_list = NULL;
    Delete(wrap_dmodule_name);
    wrap_dmodule_name = NULL;
    Delete(wrap_dmodule_fq_name);
    wrap_dmodule_fq_name = NULL;
    Delete(wrap_dmodule_code);
    wrap_dmodule_code = NULL;
    Delete(proxy_class_def);
    proxy_class_def = NULL;
    Delete(proxy_class_code);
    proxy_class_code = NULL;
    Delete(proxy_class_epilogue_code);
    proxy_class_epilogue_code = NULL;
    Delete(module_class_constants_code);
    module_class_constants_code = NULL;
    Delete(proxy_dmodule_name);
    proxy_dmodule_name = NULL;
    Delete(proxy_dmodule_fq_name);
    proxy_dmodule_fq_name = NULL;
    Delete(proxy_dmodule_code);
    proxy_dmodule_code = NULL;
    Delete(proxy_dmodule_imports);
    proxy_dmodule_imports = NULL;
    Delete(wrap_dmodule_imports);
    wrap_dmodule_imports = NULL;
    Delete(upcasts_code);
    upcasts_code = NULL;
    Delete(wrapper_loader_code);
    wrapper_loader_code = NULL;
    Delete(wrapper_loader_bind_code);
    wrapper_loader_bind_code = NULL;
    Delete(wrapper_loader_bind_command);
    wrapper_loader_bind_command = NULL;
    Delete(dmethods_seq);
    dmethods_seq = NULL;
    Delete(dmethods_table);
    dmethods_table = NULL;
    Delete(package);
    package = NULL;
    Delete(dmodule_directory);
    dmodule_directory = NULL;
    n_dmethods = 0;

    // Merge all the generated C/C++ code and close the output files.
    Dump(f_runtime, f_begin);
    Dump(f_header, f_begin);

    if (directorsEnabled()) {
      Dump(f_directors, f_begin);
      Dump(f_directors_h, f_runtime_h);

      Printf(f_runtime_h, "\n");
      Printf(f_runtime_h, "#endif\n");

      Close(f_runtime_h);
      Delete(f_runtime_h);
      f_runtime_h = NULL;
      Delete(f_directors);
      f_directors = NULL;
      Delete(f_directors_h);
      f_directors_h = NULL;
    }

    Dump(f_wrappers, f_begin);
    Wrapper_pretty_print(f_init, f_begin);
    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);
    Close(f_begin);
    Delete(f_runtime);
    Delete(f_begin);
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::emitBanner()
   * --------------------------------------------------------------------------- */
  void emitBanner(File *f) {
    Printf(f, "/* ----------------------------------------------------------------------------\n");
    Swig_banner_target_lang(f, " *");
    Printf(f, " * ----------------------------------------------------------------------------- */\n\n");
  }

  /* ---------------------------------------------------------------------------
   * D::addUpcallMethod()
   *
   * Adds new director upcall signature.
   * --------------------------------------------------------------------------- */
  UpcallData *addUpcallMethod(String *imclass_method, String *class_method, String *decl, String *overloaded_name, String *return_type, String *param_list) {
    UpcallData *udata;
    String *imclass_methodidx;
    String *class_methodidx;
    Hash *new_udata;
    String *key = NewStringf("%s|%s", imclass_method, decl);

    ++curr_class_dmethod;

    /* Do we know about this director class already? */
    if ((udata = Getattr(dmethods_table, key))) {
      Delete(key);
      return Getattr(udata, "methodoff");
    }

    imclass_methodidx = NewStringf("%d", n_dmethods);
    class_methodidx = NewStringf("%d", n_dmethods - first_class_dmethod);
    n_dmethods++;

    new_udata = NewHash();
    Append(dmethods_seq, new_udata);
    Setattr(dmethods_table, key, new_udata);

    Setattr(new_udata, "method", Copy(class_method));
    Setattr(new_udata, "class_methodidx", class_methodidx);
    Setattr(new_udata, "decl", Copy(decl));
    Setattr(new_udata, "overname", Copy(overloaded_name));
    Setattr(new_udata, "return_type", Copy(return_type));
    Setattr(new_udata, "param_list", Copy(param_list));

    Delete(key);
    return new_udata;
  }

  /* ---------------------------------------------------------------------------
   * D::nativeWrapper()
   * --------------------------------------------------------------------------- */
  virtual int nativeWrapper(Node *n) {
    String *wrapname = Getattr(n, "wrap:name");

    if (!addSymbol(wrapname, n))
      return SWIG_ERROR;

    if (Getattr(n, "type")) {
      Swig_save("nativeWrapper", n, "name", NIL);
      Setattr(n, "name", wrapname);
      native_function_flag = true;
      functionWrapper(n);
      Swig_restore(n);
      native_function_flag = false;
    } else {
      Printf(stderr, "%s : Line %d. No return type for %%native method %s.\n", input_file, line_number, Getattr(n, "wrap:name"));
    }

    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::functionWrapper()
   *
   * Generates the C wrapper code for a function and the corresponding
   * declaration in the wrap D module.
   * --------------------------------------------------------------------------- */
  virtual int functionWrapper(Node *n) {
    String *symname = Getattr(n, "sym:name");
    SwigType *t = Getattr(n, "type");
    ParmList *l = Getattr(n, "parms");
    String *tm;
    Parm *p;
    int i;
    String *c_return_type = NewString("");
    String *im_return_type = NewString("");
    String *cleanup = NewString("");
    String *outarg = NewString("");
    String *body = NewString("");
    int num_arguments = 0;
    int num_required = 0;
    bool is_void_return;
    String *overloaded_name = getOverloadedName(n);

    if (!Getattr(n, "sym:overloaded")) {
      if (!addSymbol(Getattr(n, "sym:name"), n))
	return SWIG_ERROR;
    }

    // A new wrapper function object
    Wrapper *f = NewWrapper();

    // Make a wrapper name for this function
    String *wname = Swig_name_wrapper(overloaded_name);

    /* Attach the non-standard typemaps to the parameter list. */
    Swig_typemap_attach_parms("ctype", l, f);
    Swig_typemap_attach_parms("imtype", l, f);

    /* Get return types */
    if ((tm = Swig_typemap_lookup("ctype", n, "", 0))) {
      String *ctypeout = Getattr(n, "tmap:ctype:out");	// the type in the ctype typemap's out attribute overrides the type in the typemap
      if (ctypeout)
	tm = ctypeout;
      Printf(c_return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CTYPE_UNDEF, input_file, line_number, "No ctype typemap defined for %s\n", SwigType_str(t, 0));
    }

    if ((tm = Swig_typemap_lookup("imtype", n, "", 0))) {
      String *imtypeout = Getattr(n, "tmap:imtype:out");	// the type in the imtype typemap's out attribute overrides the type in the typemap
      if (imtypeout)
	tm = imtypeout;
      Printf(im_return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSTYPE_UNDEF, input_file, line_number, "No imtype typemap defined for %s\n", SwigType_str(t, 0));
    }

    is_void_return = (Cmp(c_return_type, "void") == 0);
    if (!is_void_return)
      Wrapper_add_localv(f, "jresult", c_return_type, "jresult", NIL);

    Printv(f->def, " SWIGEXPORT ", c_return_type, " SWIGSTDCALL ", wname, "(", NIL);

    // Emit all of the local variables for holding arguments.
    emit_parameter_variables(l, f);

    /* Attach the standard typemaps */
    emit_attach_parmmaps(l, f);

    // Parameter overloading
    Setattr(n, "wrap:parms", l);
    Setattr(n, "wrap:name", wname);

    // Wrappers not wanted for some methods where the parameters cannot be overloaded in C#
    if (Getattr(n, "sym:overloaded")) {
      // Emit warnings for the few cases that can't be overloaded in C# and give up on generating wrapper
      Swig_overload_check(n);
      if (Getattr(n, "overload:ignore"))
	return SWIG_OK;
    }

    // Collect the parameter list for the wrap D module declaration of the
    // generated wrapper function.
    String *wrap_dmodule_parameters = NewString( "(" );

    /* Get number of required and total arguments */
    num_arguments = emit_num_arguments(l);
    num_required = emit_num_required(l);
    int gencomma = 0;

    // Now walk the function parameter list and generate code to get arguments
    for (i = 0, p = l; i < num_arguments; i++) {

      while (checkAttribute(p, "tmap:in:numinputs", "0")) {
	p = Getattr(p, "tmap:in:next");
      }

      SwigType *pt = Getattr(p, "type");
      String *ln = Getattr(p, "lname");
      String *im_param_type = NewString("");
      String *c_param_type = NewString("");
      String *arg = NewString("");

      Printf(arg, "j%s", ln);

      /* Get the ctype types of the parameter */
      if ((tm = Getattr(p, "tmap:ctype"))) {
	Printv(c_param_type, tm, NIL);
      } else {
	Swig_warning(WARN_CSHARP_TYPEMAP_CTYPE_UNDEF, input_file, line_number, "No ctype typemap defined for %s\n", SwigType_str(pt, 0));
      }

      /* Get the intermediary class parameter types of the parameter */
      if ((tm = Getattr(p, "tmap:imtype"))) {
	const String *inattributes = Getattr(p, "tmap:imtype:inattributes");
	Printf(im_param_type, "%s%s", inattributes ? inattributes : empty_string, tm);
      } else {
	Swig_warning(WARN_CSHARP_TYPEMAP_CSTYPE_UNDEF, input_file, line_number, "No imtype typemap defined for %s\n", SwigType_str(pt, 0));
      }

      /* Add parameter to intermediary class method */
      if (gencomma)
	Printf(wrap_dmodule_parameters, ", ");
      Printf(wrap_dmodule_parameters, "%s %s", im_param_type, arg);

      // Add parameter to C function
      Printv(f->def, gencomma ? ", " : "", c_param_type, " ", arg, NIL);

      gencomma = 1;

      // Get typemap for this argument
      if ((tm = Getattr(p, "tmap:in"))) {
	canThrow(n, "in", p);
	Replaceall(tm, "$input", arg);
	Setattr(p, "emit:input", arg);
	Printf(f->code, "%s\n", tm);
	p = Getattr(p, "tmap:in:next");
      } else {
	Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
	p = nextSibling(p);
      }
      Delete(im_param_type);
      Delete(c_param_type);
      Delete(arg);
    }

    /* Insert constraint checking code */
    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:check"))) {
	canThrow(n, "check", p);
	Replaceall(tm, "$input", Getattr(p, "emit:input"));
	Printv(f->code, tm, "\n", NIL);
	p = Getattr(p, "tmap:check:next");
      } else {
	p = nextSibling(p);
      }
    }

    /* Insert cleanup code */
    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:freearg"))) {
	canThrow(n, "freearg", p);
	Replaceall(tm, "$input", Getattr(p, "emit:input"));
	Printv(cleanup, tm, "\n", NIL);
	p = Getattr(p, "tmap:freearg:next");
      } else {
	p = nextSibling(p);
      }
    }

    /* Insert argument output code */
    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:argout"))) {
	canThrow(n, "argout", p);
	Replaceall(tm, "$result", "jresult");
	Replaceall(tm, "$input", Getattr(p, "emit:input"));
	Printv(outarg, tm, "\n", NIL);
	p = Getattr(p, "tmap:argout:next");
      } else {
	p = nextSibling(p);
      }
    }

    // Look for usage of throws typemap and the canthrow flag
    ParmList *throw_parm_list = NULL;
    if ((throw_parm_list = Getattr(n, "catchlist"))) {
      Swig_typemap_attach_parms("throws", throw_parm_list, f);
      for (p = throw_parm_list; p; p = nextSibling(p)) {
	if ((tm = Getattr(p, "tmap:throws"))) {
	  canThrow(n, "throws", p);
	}
      }
    }

    String *null_attribute = 0;
    // Now write code to make the function call
    if (!native_function_flag) {
      if (Cmp(nodeType(n), "constant") == 0) {
        // Wrapping a constant hack
        Swig_save("functionWrapper", n, "wrap:action", NIL);

        // below based on Swig_VargetToFunction()
        SwigType *ty = Swig_wrapped_var_type(Getattr(n, "type"), use_naturalvar_mode(n));
        Setattr(n, "wrap:action", NewStringf("result = (%s) %s;", SwigType_lstr(ty, 0), Getattr(n, "value")));
      }

      Swig_director_emit_dynamic_cast(n, f);
      String *actioncode = emit_action(n);

      if (Cmp(nodeType(n), "constant") == 0)
        Swig_restore(n);

      /* Return value if necessary  */
      if ((tm = Swig_typemap_lookup_out("out", n, "result", f, actioncode))) {
	canThrow(n, "out", n);
	Replaceall(tm, "$result", "jresult");

        if (GetFlag(n, "feature:new"))
          Replaceall(tm, "$owner", "1");
        else
          Replaceall(tm, "$owner", "0");

	Printf(f->code, "%s", tm);
	null_attribute = Getattr(n, "tmap:out:null");
	if (Len(tm))
	  Printf(f->code, "\n");
      } else {
	Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number, "Unable to use return type %s in function %s.\n", SwigType_str(t, 0), Getattr(n, "name"));
      }
      emit_return_variable(n, t, f);
    }

    /* Output argument output code */
    Printv(f->code, outarg, NIL);

    /* Output cleanup code */
    Printv(f->code, cleanup, NIL);

    /* Look to see if there is any newfree cleanup code */
    if (GetFlag(n, "feature:new")) {
      if ((tm = Swig_typemap_lookup("newfree", n, "result", 0))) {
	canThrow(n, "newfree", n);
	Printf(f->code, "%s\n", tm);
      }
    }

    /* See if there is any return cleanup code */
    if (!native_function_flag) {
      if ((tm = Swig_typemap_lookup("ret", n, "result", 0))) {
	canThrow(n, "ret", n);
	Printf(f->code, "%s\n", tm);
      }
    }

    // Complete D wrapper parameter list and emit the declaration/binding code.
    Printv(wrap_dmodule_parameters, ")", NIL);
    writeWrapDModuleFunction( overloaded_name, im_return_type,
      wrap_dmodule_parameters, wname );
    Delete( wrap_dmodule_parameters );

    // Finish C function header.
    Printf(f->def, ") {");

    if (!is_void_return)
      Printv(f->code, "    return jresult;\n", NIL);
    Printf(f->code, "}\n");

    /* Substitute the cleanup code */
    Replaceall(f->code, "$cleanup", cleanup);

    /* Substitute the function name */
    Replaceall(f->code, "$symname", symname);

    /* Contract macro modification */
    if (Replaceall(f->code, "SWIG_contract_assert(", "SWIG_contract_assert($null, ") > 0) {
      Setattr(n, "csharp:canthrow", "1");
    }

    if (!null_attribute)
      Replaceall(f->code, "$null", "0");
    else
      Replaceall(f->code, "$null", null_attribute);

    /* Dump the function out */
    if (!native_function_flag) {
      Wrapper_print(f, f_wrappers);

      // Handle %csexception which sets the canthrow attribute
      if (Getattr(n, "feature:except:canthrow"))
	Setattr(n, "csharp:canthrow", "1");

      // A very simple check (it is not foolproof) to help typemap/feature writers for
      // throwing C# exceptions from unmanaged code. It checks for the common methods which
      // set a pending C# exception... the 'canthrow' typemap/feature attribute must be set
      // so that code which checks for pending exceptions is added in the C# proxy method.
      if (!Getattr(n, "csharp:canthrow")) {
	if (Strstr(f->code, "SWIG_exception")) {
	  Swig_warning(WARN_CSHARP_CANTHROW, input_file, line_number,
	  "C code contains a call to SWIG_exception and D code does not handle pending exceptions via the canthrow attribute.\n");
	} else if (Strstr(f->code, "SWIG_DSetPendingException")) {
	  Swig_warning(WARN_CSHARP_CANTHROW, input_file, line_number,
	  "C code contains a call to a SWIG_DSetPendingException method and D code does not handle pending exceptions via the canthrow attribute.\n");
	}
      }
    }

    // If we are not processing an enum or constant, and we were not generating
    // a wrapper function which will be accessed via a proxy class, write a
    // function to the proxy D module.
    if (!(proxy_flag && is_wrapping_class())) {
      writeProxyDModuleFunction(n);
    }

    // If we are processing a public member variable, write the property-style
    // member function to the proxy class.
    if (proxy_flag && wrapping_member_flag) {
      Setattr(n, "proxyfuncname", variable_name);
      Setattr(n, "imfuncname", symname);

      writeProxyClassFunction(n);
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

  /* ---------------------------------------------------------------------------
   * D::globalvariableHandler()
   * --------------------------------------------------------------------------- */
  virtual int globalvariableHandler(Node *n) {
    variable_name = Getattr(n, "sym:name");
    global_variable_flag = true;
    int ret = Language::globalvariableHandler(n);
    global_variable_flag = false;

    return ret;
  }

  /* ---------------------------------------------------------------------------
   * D::enumDeclaration()
   *
   * Wraps C/C++ enums as D enums.
   * --------------------------------------------------------------------------- */
  virtual int enumDeclaration(Node *n) {
    if (ImportMode)
      return SWIG_OK;

    if (getCurrentClass() && (cplus_mode != PUBLIC))
      return SWIG_NOWRAP;

    enum_code = NewString("");
    String *symname = Getattr(n, "sym:name");
    String *typemap_lookup_type = Getattr(n, "name");

    // Emit the enum declaration.
    if (typemap_lookup_type) {
      const String *enummodifiers = typemapLookup(n, "dclassmodifiers", typemap_lookup_type, WARN_D_TYPEMAP_CLASSMOD_UNDEF);
      Printv(enum_code, enummodifiers, " ", symname, " {\n", NIL);
    } else {
      // Handle anonymous enums.
      Printv(enum_code, "enum {\n", NIL);
    }

    // Emit each enum item.
    Language::enumDeclaration(n);

    if (!GetFlag(n, "nonempty")) {
      // Do not wrap empty enums; the resulting D code would be illegal.
      Delete(enum_code);
      return SWIG_NOWRAP;
    }

    // Finish the enum.
    if (typemap_lookup_type) {
      Printv(enum_code,
	typemapLookup(n, "dcode", typemap_lookup_type, WARN_NONE), // Extra D code
	"\n}", NIL);
    } else {
      // Handle anonymous enums.
      Printv(enum_code, "\n}", NIL);
    }

    Replaceall(enum_code, "$dclassname", symname);

    if (proxy_flag && is_wrapping_class()) {
      // Enums defined within the C++ class are written into the proxy
      // class.

      // Add extra indentation
      Replaceall(enum_code, "\n", "\n  ");
      Replaceall(enum_code, "  \n", "\n");

      Printv(proxy_class_enums_code, "  ", enum_code, "\n\n", NIL);
    } else {
      // Global enums are just written to the proxy module.
      Printv( proxy_dmodule_imports,
	typemapLookup(n, "dimports", typemap_lookup_type, WARN_NONE), NIL);
      Printv( proxy_dmodule_code, enum_code, "\n\n", NIL);
    }

    Delete(enum_code);
    enum_code = NULL;
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::enumvalueDeclaration()
   * --------------------------------------------------------------------------- */
  virtual int enumvalueDeclaration(Node *n) {
    if (getCurrentClass() && (cplus_mode != PUBLIC))
      return SWIG_NOWRAP;

    Swig_require("enumvalueDeclaration", n, "*name", "?value", NIL);
    String *symname = Getattr(n, "sym:name");
    String *value = Getattr(n, "value");
    String *name = Getattr(n, "name");
    Node *parent = parentNode(n);
    String *tmpValue;

    // Strange hack from parent method.
    // RESEARCH: What is this doing?
    if (value)
      tmpValue = NewString(value);
    else
      tmpValue = NewString(name);
    // Note that this is used in enumValue() amongst other places
    Setattr(n, "value", tmpValue);

    {
      // Wrap (non-anonymous) C/C++ enum with a proper D enum.
      // Emit the enum item.
      if (!GetFlag(n, "firstenumitem"))
	Printf(enum_code, ",\n");

      Printf(enum_code, "  %s", symname);

      // Check for the %dconstvalue feature
      String *value = Getattr(n, "feature:d:constvalue");

      // Note that in D, enum values must be compile-time constants. Thus,
      // %dconst(0) (getting the enum values at runtime) is not supported.
      value = value ? value : Getattr(n, "enumvalue");
      if (value) {
	Printf(enum_code, " = %s", value);
      }

      // Keep track that the currently processed enum has at least one value.
      SetFlag(parent, "nonempty");
    }

    Delete(tmpValue);
    Swig_restore(n);
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::constantWrapper()
   *
   * Used for wrapping constants declared by #define or %constant and also for
   * (primitive) static member constants initialised inline.
   *
   * If the %dconst(1) feature is used, the C/C++ constant value is used to
   * initialize a D »const«. If not, a »getter« method is generated which
   * retrieves the value via a call to the C wrapper. However, if there is a
   * %dconstvalue specified, it overrides all other settings.
   * --------------------------------------------------------------------------- */
  virtual int constantWrapper(Node *n) {
    String *symname = Getattr(n, "sym:name");
    if (!addSymbol(symname, n))
      return SWIG_ERROR;

    // The %dconst feature determines if a D const or a getter function is
    // created.
    if (GetFlag(n, "feature:d:const") != 1) {
      // Default constant handling will work with any type of C constant. It
      // generates a getter function (which is the same as a read only property
      // in D) which retrieves the value via by calling the C wrapper.
      // Note that this is only called for global constants, static member
      // constants are already handeled in staticmemberfunctionHandler().

      Swig_save("constantWrapper", n, "value", NIL);

      // Add the stripped quotes back in.
      String *old_value = Getattr(n, "value");
      SwigType *t = Getattr(n, "type");
      if (SwigType_type(t) == T_STRING) {
	Setattr(n, "value", NewStringf("\"%s\"", old_value));
	Delete(old_value);
      } else if (SwigType_type(t) == T_CHAR) {
	Setattr(n, "value", NewStringf("\'%s\'", old_value));
	Delete(old_value);
      }

      int result = globalvariableHandler(n);

      Swig_restore(n);
      return result;
    }

    String *constants_code = NewString("");
    SwigType *t = Getattr(n, "type");
    ParmList *l = Getattr(n, "parms");

    // Attach the non-standard typemaps to the parameter list.
    Swig_typemap_attach_parms("cstype", l, NULL);

    // Get D return type.
    String *return_type = NewString("");
    String *tm;
    if ((tm = Swig_typemap_lookup("cstype", n, "", 0))) {
      String *cstypeout = Getattr(n, "tmap:cstype:out");	// the type in the cstype typemap's out attribute overrides the type in the typemap
      if (cstypeout)
	tm = cstypeout;
      substituteClassname(t, tm);
      Printf(return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(t, 0));
    }

    const String *itemname = (proxy_flag && wrapping_member_flag) ? variable_name : symname;

    String *attributes = Getattr(n, "feature:d:methodmodifiers");
    if ( attributes ) {
      attributes = Copy( attributes );
    } else {
      attributes = Copy( is_public( n ) ? public_string : protected_string );
    }

    if ( static_flag ) {
      Printv( attributes, " static", NIL );
    }

    Printf(constants_code, "%s const %s %s = ", attributes, return_type, itemname);
    Delete(attributes);

    // Retrive the override value set via %dconstvalue, if any.
    String *override_value = Getattr(n, "feature:d:constvalue");
    if (override_value) {
      Printf(constants_code, "%s;\n", override_value);
    } else {
      // Just emit the C code and hope it compiles in D.
      String* value = Getattr(n, "wrappedasconstant") ?
	Getattr(n, "staticmembervariableHandler:value") : Getattr(n, "value");

      // Add the stripped quotes back in.
      if (SwigType_type(t) == T_STRING) {
	Printf(constants_code, "\"%s\";\n", value);
      } else if (SwigType_type(t) == T_CHAR) {
	Printf(constants_code, "\'%s\';\n", value);
      } else {
	Printf(constants_code, "%s;\n", value);
      }
    }

    // Emit the generated code to appropriate place.
    if (proxy_flag && wrapping_member_flag)
      Printv(proxy_class_code, "  ", constants_code, NIL);
    else
      Printv(module_class_constants_code, constants_code, NIL);

    // Cleanup.
    Delete(return_type);
    Delete(constants_code);

    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::insertDirective()
   * --------------------------------------------------------------------------- */
  virtual int insertDirective(Node *n) {
    String *code = Getattr(n, "code");
    Replaceall(code, "$proxydmodule", proxy_dmodule_fq_name);
    Replaceall(code, "$wrapdmodule", wrap_dmodule_fq_name);
    Replaceall(code, "$module", proxy_dmodule_name);
    return Language::insertDirective(n);
  }

  /* ---------------------------------------------------------------------------
   * D::pragmaDirective()
   *
   * Valid Pragmas:
   * wrapdmodulecode      - text (D code) is copied verbatim to the wrap module
   * wrapdmoduleimports   - import statements for the wrap module
   *
   * proxydmodulecode     - text (D code) is copied verbatim to the proxy module
   * proxydmoduleimports  - import statements for the proxy module
   * --------------------------------------------------------------------------- */
  virtual int pragmaDirective(Node *n) {
    if (!ImportMode) {
      String *lang = Getattr(n, "lang");
      String *code = Getattr(n, "name");
      String *value = Getattr(n, "value");

      if (Strcmp(lang, "d") == 0) {
	String *strvalue = NewString(value);
	Replaceall(strvalue, "\\\"", "\"");

	if (Strcmp(code, "wrapdmodulecode") == 0) {
	  Printf(wrap_dmodule_code, "%s\n", strvalue);
	} else if (Strcmp(code, "wrapdmoduleimports") == 0) {
	  Printv(wrap_dmodule_imports, strvalue, NIL);
	} else if (Strcmp(code, "proxydmodulecode") == 0) {
	  Printf(proxy_dmodule_code, "%s\n", strvalue);
	} else if (Strcmp(code, "proxydmoduleimports") == 0) {
	  Printv(proxy_dmodule_imports, strvalue, NIL);
	} else if (Strcmp(code, "wrapperloadercode") == 0) {
	  Delete(wrapper_loader_code);
	  wrapper_loader_code = Copy(strvalue);
	} else if (Strcmp(code, "wrapperloaderbindcommand") == 0) {
	  Delete(wrapper_loader_bind_command);
	  wrapper_loader_bind_command = Copy(strvalue);
	} else {
	  Printf(stderr, "%s : Line %d. Unrecognized pragma.\n", input_file, line_number);
	}
	Delete(strvalue);
      }
    }
    return Language::pragmaDirective(n);
  }

  /* ---------------------------------------------------------------------------
   * D::writeProxyClassAndUpcasts()
   * --------------------------------------------------------------------------- */
  void writeProxyClassAndUpcasts(Node *n) {
    String *c_classname = SwigType_namestr(Getattr(n, "name"));
    String *c_baseclass = NULL;
    String *baseclass = NULL;
    String *c_baseclassname = NULL;
    SwigType *typemap_lookup_type = Getattr(n, "classtypeobj");
    bool feature_director = Swig_directorclass(n) ? true : false;

    // Inheritance from pure D classes.
    Node *attributes = NewHash();
    const String *pure_baseclass = typemapLookup(n, "csbase", typemap_lookup_type, WARN_NONE, attributes);
    bool purebase_replace = GetFlag(attributes, "tmap:csbase:replace") ? true : false;
    bool purebase_notderived = GetFlag(attributes, "tmap:csbase:notderived") ? true : false;
    Delete(attributes);

    // C++ inheritance.
    if (!purebase_replace) {
      List *baselist = Getattr(n, "bases");
      if (baselist) {
        Iterator base = First(baselist);
        while (base.item && GetFlag(base.item, "feature:ignore")) {
          base = Next(base);
        }
        if (base.item) {
          c_baseclassname = Getattr(base.item, "name");
          baseclass = Copy(getProxyName(c_baseclassname));
          if (baseclass)
            c_baseclass = SwigType_namestr(Getattr(base.item, "name"));
          base = Next(base);
          /* Warn about multiple inheritance for additional base class(es) */
          while (base.item) {
            if (GetFlag(base.item, "feature:ignore")) {
              base = Next(base);
              continue;
            }
            String *proxyclassname = SwigType_str(Getattr(n, "classtypeobj"), 0);
            String *baseclassname = SwigType_str(Getattr(base.item, "name"), 0);
            Swig_warning(WARN_D_MULTIPLE_INHERITANCE, input_file, line_number,
	      "Base %s of class %s ignored: multiple inheritance is not supported in D.\n", baseclassname, proxyclassname);
            base = Next(base);
          }
        }
      }
    }

    bool derived = baseclass && getProxyName(c_baseclassname);
    if (derived && purebase_notderived)
      pure_baseclass = empty_string;
    const String *wanted_base = baseclass ? baseclass : pure_baseclass;

    if (purebase_replace) {
      wanted_base = pure_baseclass;
      derived = false;
      Delete(baseclass);
      baseclass = NULL;
      if (purebase_notderived)
        Swig_error(input_file, line_number, "The csbase typemap for proxy %s must contain just one of the 'replace' or 'notderived' attributes.\n", typemap_lookup_type);
    } else if (Len(pure_baseclass) > 0 && Len(baseclass) > 0) {
      Swig_warning(WARN_D_MULTIPLE_INHERITANCE, input_file, line_number,
	"Warning for %s proxy: Base class %s ignored. Multiple inheritance is not supported in D. "
	"Perhaps you need one of the 'replace' or 'notderived' attributes in the csbase typemap?\n", typemap_lookup_type, pure_baseclass);
    }


    /*
     * Emit the proxy D class.
     */

    // Import statements.
    Printv(proxy_dmodule_imports,
      typemapLookup(n, "dimports", typemap_lookup_type, WARN_NONE), "\n", NIL);

    // Class modifiers.
    const String *modifiers =
      typemapLookup(n, "dclassmodifiers", typemap_lookup_type, WARN_D_TYPEMAP_CLASSMOD_UNDEF);

    // User-defined interfaces.
    const String *interfaces =
      typemapLookup(n, derived ? "dinterfaces_derived" : "dinterfaces", typemap_lookup_type, WARN_NONE);

    // Default class body.
    const String *body;
    if (derived) {
      body = typemapLookup(n, "dbody_derived", typemap_lookup_type, WARN_D_TYPEMAP_DBODY_UNDEF);
    } else {
      body = typemapLookup(n, "dbody", typemap_lookup_type, WARN_D_TYPEMAP_DBODY_UNDEF);
    }

    Printv(proxy_class_def,
      modifiers,
      " $dclassname",
      (*Char(wanted_base) || *Char(interfaces)) ? " : " : "", wanted_base,
      (*Char(wanted_base) && *Char(interfaces)) ? ", " : "", interfaces, " {",
      body, NIL);

    // Destructor and dispose().
    // If the C++ destructor is accessible (public), it is wrapped by the
    // dispose() method which is also called by the emitted D constructor. If it
    // is not accessible, no D destructor is written and the generated dispose()
    // method throws an exception.
    // This enables C++ classes with protected or private destructors to be used
    // in D as it would be used in C++ (GC finalization is a no-op then because
    // of the empty D destructor) while preventing usage in »scope« variables.
    // The method name for the dispose() method is specified in a typemap
    // attribute called »methodname«.
    const String *tm = NULL;

    String *dispose_methodname;
    String *dispose_methodmodifiers;
    attributes = NewHash();
    if (derived) {
      tm = typemapLookup(n, "ddispose_derived", typemap_lookup_type, WARN_NONE, attributes);
      dispose_methodname = Getattr(attributes, "tmap:ddispose_derived:methodname");
      dispose_methodmodifiers = Getattr(attributes, "tmap:ddispose_derived:methodmodifiers");
    } else {
      tm = typemapLookup(n, "ddispose", typemap_lookup_type, WARN_NONE, attributes);
      dispose_methodname = Getattr(attributes, "tmap:ddispose:methodname");
      dispose_methodmodifiers = Getattr(attributes, "tmap:ddispose:methodmodifiers");
    }

    if (tm && *Char(tm)) {
      if (!dispose_methodname) {
	Swig_error(input_file, line_number,
	  "No methodname attribute defined in the ddispose%s typemap for %s\n",
	  (derived ? "_derived" : ""), proxy_class_name);
      }
      if (!dispose_methodmodifiers) {
	Swig_error(input_file, line_number,
	  "No methodmodifiers attribute defined in ddispose%s typemap for %s.\n",
	  (derived ? "_derived" : ""), proxy_class_name);
      }
    }

    if (tm) {
      // Write the destructor if the C++ one is accessible.
      if (*Char(destructor_call)) {
	Printv(proxy_class_def, typemapLookup(n, "ddestructor", typemap_lookup_type, WARN_NONE), NIL);
      }

      // Write the dispose() method.
      String *dispose_code = NewString("");
      Printv(dispose_code, tm, NIL);

      if (*Char(destructor_call)) {
	Replaceall(dispose_code, "$imcall", destructor_call);
      } else {
	Replaceall(dispose_code, "$imcall", "throw new Exception(\"C++ destructor does not have public access\")");
      }

      if (*Char(dispose_code)) {
	Printv(proxy_class_def, "\n  ", dispose_methodmodifiers,
	  (derived ? " override" : ""), " void ", dispose_methodname, "() ",
	  dispose_code, "\n", NIL);
      }
    }

    if (feature_director) {
      // Generate director connect method.
      // TODO: Put this into classDirectorEnd?
      Printf(proxy_class_code, "  private void __SwigDirectorConnect() {\n");

      int i;
      for (i = first_class_dmethod; i < curr_class_dmethod; ++i) {
	UpcallData *udata = Getitem(dmethods_seq, i);
	String *method = Getattr(udata, "method");
	String *overloaded_name = Getattr(udata, "overname");
	String *return_type = Getattr(udata, "return_type");
	String *param_list = Getattr(udata, "param_list");
	String *methid = Getattr(udata, "class_methodidx");
	Printf(proxy_class_code, "    %s.__SwigDirector_%s_Callback%s callback%s;\n", wrap_dmodule_fq_name, proxy_class_name, methid, methid);
	Printf(proxy_class_code, "    if ( __SwigIsMethodOverridden!( \"%s\", \"%s\", \"%s\" ) ) {\n", method, return_type, param_list);
	Printf(proxy_class_code, "      callback%s = &__SwigDirector_%s_%s;\n", methid, proxy_class_name, overloaded_name);
	Printf(proxy_class_code, "    }\n\n");
      }
      Printf(proxy_class_code, "    %s.%s_director_connect( __swig_cObject, cast( void* )this", wrap_dmodule_fq_name, proxy_class_name);
      for (i = first_class_dmethod; i < curr_class_dmethod; ++i) {
	UpcallData *udata = Getitem(dmethods_seq, i);
	String *methid = Getattr(udata, "class_methodidx");
	Printf(proxy_class_code, ", callback%s", methid);
      }
      Printf(proxy_class_code, " );\n");
      Printf(proxy_class_code, "  }\n");

      // Helper function to determine if a method has been overridden in a
      // subclass of the wrapped class. If not, we just pass null to the
      // director_connect_function since the method from the C++ class should
      // be called as usual (see above).
      // Only emit it if the proxy class has at least one method.
      if (first_class_dmethod < curr_class_dmethod) {
	Printf(proxy_class_code, "\n");
	Printf(proxy_class_code, "  private bool __SwigIsMethodOverridden( char[] methodName, char[] returnType, char[] parameterTypes )() {\n");
	Printf(proxy_class_code, "    auto vtblMethod = mixin( \"cast(\" ~ returnType ~ \" delegate(\" ~ parameterTypes ~ \"))&\" ~ methodName );\n" );
	Printf(proxy_class_code, "    void* baseMethod = mixin( \"__SwigAddressOf!(\" ~ methodName ~ \", \" ~ returnType ~ \" function(\" ~ parameterTypes ~ \"))\" );\n" );
	Printf(proxy_class_code, "    return ( cast(void*)vtblMethod.funcptr != baseMethod );\n");
	Printf(proxy_class_code, "  }\n");
	Printf(proxy_class_code, "\n");
	Printf(proxy_class_code, "  template __SwigAddressOf( alias fn, Type ) {\n");
	Printf(proxy_class_code, "    const __SwigAddressOf = cast(Type)&fn;\n");
	Printf(proxy_class_code, "  }\n");
      }

      if (Len(director_dcallbacks_code) > 0)
	Printv(proxy_class_epilogue_code, director_dcallbacks_code, NIL);
      if (Len(director_delegate_definitions) > 0)
	Printv(proxy_class_code, "\n", director_delegate_definitions, NIL);
      if (Len(director_delegate_instances) > 0)
	Printv(proxy_class_code, "\n", director_delegate_instances, NIL);
      if (Len(director_method_types) > 0)
	Printv(proxy_class_code, "\n", director_method_types, NIL);

      Delete(director_callback_typedefs);
      director_callback_typedefs = NULL;
      Delete(director_callbacks);
      director_callbacks = NULL;
      Delete(director_dcallbacks_code);
      director_dcallbacks_code = NULL;
      Delete(director_delegate_definitions);
      director_delegate_definitions = NULL;
      Delete(director_delegate_instances);
      director_delegate_instances = NULL;
      Delete(director_method_types);
      director_method_types = NULL;
      Delete(director_connect_parms);
      director_connect_parms = NULL;
    }

    // Write extra user D code to the class body.
    Printv(proxy_class_def,
      typemapLookup(n, "dcode", typemap_lookup_type, WARN_NONE), "\n", NIL);

    // Substitute various strings into the above template
    Replaceall(proxy_class_code, "$dclassname", proxy_class_name);
    Replaceall(proxy_class_def, "$dclassname", proxy_class_name);

    // RESEARCH: Is replacing $proxydmodule needed at all?
    Replaceall(proxy_class_def, "$proxydmodule", proxy_dmodule_fq_name);
    Replaceall(proxy_class_code, "$proxydmodule", proxy_dmodule_fq_name);

    Replaceall(proxy_class_def, "$wrapdmodule", wrap_dmodule_fq_name);
    Replaceall(proxy_class_code, "$wrapdmodule", wrap_dmodule_fq_name);

    Replaceall(proxy_class_def, "$module", proxy_dmodule_name);
    Replaceall(proxy_class_code, "$module", proxy_dmodule_name);

    // Add code to do C++ casting to base class (only for classes in an inheritance hierarchy)
    if (derived) {
      String *upcast_name = NewString( "" );
      Printv( upcast_name, proxy_class_name, "Upcast", NIL );
      String *upcast_wrapper_name = Swig_name_wrapper( upcast_name );

      writeWrapDModuleFunction( upcast_name, "void*",
	"(void* objectRef)", upcast_wrapper_name );

      Printv(upcasts_code,
	"SWIGEXPORT $cbaseclass * SWIGSTDCALL ", upcast_wrapper_name,
	"($cclass *objectRef) {\n", "    return ($cbaseclass *)objectRef;\n" "}\n", "\n", NIL);

      Replaceall(upcasts_code, "$cbaseclass", c_baseclass);
      Replaceall(upcasts_code, "$cclass", c_classname);

      Delete( upcast_name );
      Delete( upcast_wrapper_name );
    }
    Delete(baseclass);
  }

  /* ---------------------------------------------------------------------------
   * D::classHandler()
   * --------------------------------------------------------------------------- */
  virtual int classHandler(Node *n) {
    if (proxy_flag) {
      proxy_class_name = NewString(Getattr(n, "sym:name"));

      if (!addSymbol(proxy_class_name, n))
	return SWIG_ERROR;

      Clear(proxy_class_def);
      Clear(proxy_class_code);
      Clear(proxy_class_epilogue_code);

      destructor_call = NewString("");
      proxy_class_enums_code = NewString("");
    }

    Language::classHandler(n);

    if (proxy_flag) {
      writeProxyClassAndUpcasts(n);

      // RESEARCH: Is this needed at all (we are writing this to the proxy
      // module, why should it have to refer to itself)?
      Replaceall(proxy_class_def, "$proxydmodule", proxy_dmodule_fq_name);
      Replaceall(proxy_class_code, "$proxydmodule", proxy_dmodule_fq_name);
      Replaceall(proxy_class_enums_code, "$proxydmodule", proxy_dmodule_fq_name);
      Replaceall(proxy_class_epilogue_code, "$proxydmodule", proxy_dmodule_fq_name);

      Replaceall(proxy_class_def, "$wrapdmodule", wrap_dmodule_fq_name);
      Replaceall(proxy_class_code, "$wrapdmodule", wrap_dmodule_fq_name);
      Replaceall(proxy_class_enums_code, "$wrapdmodule", wrap_dmodule_fq_name);
      Replaceall(proxy_class_epilogue_code, "$wrapdmodule", wrap_dmodule_fq_name);

      Replaceall(proxy_class_def, "$module", proxy_dmodule_name);
      Replaceall(proxy_class_code, "$module", proxy_dmodule_name);
      Replaceall(proxy_class_enums_code, "$module", proxy_dmodule_name);
      Replaceall(proxy_class_epilogue_code, "$module", proxy_dmodule_name);

      // Write the proxy class definition (the header part).
      Printv(proxy_dmodule_code, proxy_class_def, NIL);

      // Write all constants and enumerations first to prevent forward reference
      // errors.
      if (Len(proxy_class_enums_code) != 0)
	Printv(proxy_dmodule_code, proxy_class_enums_code, NIL);

      // Write the class code and the curly bracket closing the class definition.
      Printv(proxy_dmodule_code, proxy_class_code, "}\n", NIL);

      // Write the epilogue code if there is any.
      if (Len(proxy_class_epilogue_code) != 0)
	Printv(proxy_dmodule_code, proxy_class_epilogue_code, NIL);

      Printv( proxy_dmodule_code, "\n", NIL );

      emitDirectorExtraMethods(n);

      Delete(proxy_class_name);
      proxy_class_name = NULL;
      Delete(destructor_call);
      destructor_call = NULL;
      Delete(proxy_class_enums_code);
      proxy_class_enums_code = NULL;
    }

    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::memberfunctionHandler()
   * --------------------------------------------------------------------------- */
  virtual int memberfunctionHandler(Node *n) {
    Language::memberfunctionHandler(n);

    if (proxy_flag) {
      String *overloaded_name = getOverloadedName(n);
      String *intermediary_function_name = Swig_name_member(proxy_class_name, overloaded_name);
      Setattr(n, "proxyfuncname", Getattr(n, "sym:name"));
      Setattr(n, "imfuncname", intermediary_function_name);
      writeProxyClassFunction(n);
      Delete(overloaded_name);
    }
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::staticmemberfunctionHandler()
   * --------------------------------------------------------------------------- */
  virtual int staticmemberfunctionHandler(Node *n) {

    static_flag = true;
    Language::staticmemberfunctionHandler(n);

    if (proxy_flag) {
      String *overloaded_name = getOverloadedName(n);
      String *intermediary_function_name = Swig_name_member(proxy_class_name, overloaded_name);
      Setattr(n, "proxyfuncname", Getattr(n, "sym:name"));
      Setattr(n, "imfuncname", intermediary_function_name);
      writeProxyClassFunction(n);
      Delete(overloaded_name);
    }
    static_flag = false;

    return SWIG_OK;
  }


  /* ---------------------------------------------------------------------------
   * D::writeProxyClassFunction()
   *
   * Creates a D proxy function for a C++ function in the wrapped class. Used
   * for both static and non-static C++ class functions.
   *
   * The Node must contain two extra attributes.
   *  - "proxyfuncname": The name of the D proxy function.
   *  - "imfuncname": The corresponding function in the wrap D module.
   * --------------------------------------------------------------------------- */
  void writeProxyClassFunction(Node *n) {
    SwigType *t = Getattr(n, "type");
    ParmList *l = Getattr(n, "parms");
    String *intermediary_function_name = Getattr(n, "imfuncname");
    String *proxy_function_name = Getattr(n, "proxyfuncname");
    String *tm;
    Parm *p;
    int i;
    String *imcall = NewString("");
    String *return_type = NewString("");
    String *function_code = NewString("");
    bool setter_flag = false;
    String *pre_code = NewString("");
    String *post_code = NewString("");
    String *terminator_code = NewString("");

    // RESEARCH: We shouldn't even get here then?
    if (!proxy_flag)
      return;

    // Wrappers not wanted for some methods where the parameters cannot be overloaded in C#
    if (Getattr(n, "overload:ignore"))
      return;

    // Don't generate proxy method for additional explicitcall method used in directors
    if (GetFlag(n, "explicitcall"))
      return;

    // RESEARCH: What is this good for?
    if (l) {
      if (SwigType_type(Getattr(l, "type")) == T_VOID) {
	l = nextSibling(l);
      }
    }

    /* Attach the non-standard typemaps to the parameter list */
    Swig_typemap_attach_parms("in", l, NULL);
    Swig_typemap_attach_parms("cstype", l, NULL);
    Swig_typemap_attach_parms("csin", l, NULL);

    /* Get return types */
    if ((tm = Swig_typemap_lookup("cstype", n, "", 0))) {
      // the type in the cstype typemap's out attribute overrides the type in the typemap
      String *cstypeout = Getattr(n, "tmap:cstype:out");
      if (cstypeout)
	tm = cstypeout;
      substituteClassname(t, tm);
      Printf(return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(t, 0));
    }

    if (wrapping_member_flag) {
      // Check if this is a setter method for a public member.
      setter_flag = (Cmp(Getattr(n, "sym:name"),
	Swig_name_set(Swig_name_member(proxy_class_name, variable_name))) == 0);
    }

    /* Start generating the proxy function */
    const String *outattributes = Getattr(n, "tmap:cstype:outattributes");
    if (outattributes)
      Printf(function_code, "  %s\n", outattributes);
    const String *methodmods = Getattr(n, "feature:d:methodmodifiers");
    if (methodmods) {
      if (is_smart_pointer()) {
	// Smart pointer classes do not mirror the inheritance hierarchy of the
	// underlying pointer type, so no override required.
	String *mmods = Copy(methodmods);
	Replaceall(mmods, "override", "");
	Chop(mmods);		// remove trailing whitespace
	Printf(function_code, "  %s ", mmods);
	Delete(mmods);
      } else {
	Printf(function_code, "  %s ", methodmods);
      }
    } else {
      methodmods = (is_public(n) ? public_string : protected_string);
      Printf(function_code, "  %s ", methodmods);
      if (!is_smart_pointer()) {
	// Smart pointer classes do not mirror the inheritance hierarchy of the
	// underlying pointer type, so no override required.
	if (Getattr(n, "override"))
	  Printf(function_code, "override ");
      }
    }
    if (static_flag)
      Printf(function_code, "static ");
    Printf(function_code, "%s %s(", return_type, proxy_function_name);

    Printv(imcall, wrap_dmodule_fq_name, ".$imfuncname(", NIL);
    if (!static_flag)
      Printf(imcall, "__swig_cObject");

    emit_mark_varargs(l);

    int gencomma = !static_flag;

    /* Output each parameter */
    for (i = 0, p = l; p; i++) {

      /* Ignored varargs */
      if (checkAttribute(p, "varargs:ignore", "1")) {
	p = nextSibling(p);
	continue;
      }

      /* Ignored parameters */
      if (checkAttribute(p, "tmap:in:numinputs", "0")) {
	p = Getattr(p, "tmap:in:next");
	continue;
      }

      /* Ignore the 'this' argument for variable wrappers */
      if (!(variable_wrapper_flag && i == 0)) {
	SwigType *pt = Getattr(p, "type");
	String *param_type = NewString("");

	/* Get the C# parameter type */
	if ((tm = Getattr(p, "tmap:cstype"))) {
	  substituteClassname(pt, tm);
	  const String *inattributes = Getattr(p, "tmap:cstype:inattributes");
	  Printf(param_type, "%s%s", inattributes ? inattributes : empty_string, tm);
	} else {
	  Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(pt, 0));
	}

	if (gencomma)
	  Printf(imcall, ", ");

	String *arg = makeParameterName(n, p, i, setter_flag);

	// Use typemaps to transform type used in C# wrapper function (in proxy class) to type used in PInvoke function (in intermediary class)
	if ((tm = Getattr(p, "tmap:csin"))) {
	  substituteClassname(pt, tm);
	  Replaceall(tm, "$csinput", arg);
          String *pre = Getattr(p, "tmap:csin:pre");
          if (pre) {
            substituteClassname(pt, pre);
            Replaceall(pre, "$csinput", arg);
            if (Len(pre_code) > 0)
              Printf(pre_code, "\n");
            Printv(pre_code, pre, NIL);
          }
          String *post = Getattr(p, "tmap:csin:post");
          if (post) {
            substituteClassname(pt, post);
            Replaceall(post, "$csinput", arg);
            if (Len(post_code) > 0)
              Printf(post_code, "\n");
            Printv(post_code, post, NIL);
          }
          String *terminator = Getattr(p, "tmap:csin:terminator");
          if (terminator) {
            substituteClassname(pt, terminator);
            Replaceall(terminator, "$csinput", arg);
            if (Len(terminator_code) > 0)
              Insert(terminator_code, 0, "\n");
            Insert(terminator_code, 0, terminator);
          }
	  Printv(imcall, tm, NIL);
	} else {
	  Swig_warning(WARN_CSHARP_TYPEMAP_CSIN_UNDEF, input_file, line_number, "No csin typemap defined for %s\n", SwigType_str(pt, 0));
	}

	/* Add parameter to proxy function */
	if (gencomma >= 2)
	  Printf(function_code, ", ");
	gencomma = 2;
	Printf(function_code, "%s %s", param_type, arg);

	Delete(arg);
	Delete(param_type);
      }
      p = Getattr(p, "tmap:in:next");
    }

    Printf(imcall, ")");
    Printf(function_code, ")");

    // Transform return type used in PInvoke function (in intermediary class) to type used in C# wrapper function (in proxy class)
    if ((tm = Swig_typemap_lookup("csout", n, "", 0))) {
      excodeSubstitute(n, tm, "csout", n);
      bool is_pre_code = Len(pre_code) > 0;
      bool is_post_code = Len(post_code) > 0;
      bool is_terminator_code = Len(terminator_code) > 0;
      if (is_pre_code || is_post_code || is_terminator_code) {
        Replaceall(tm, "\n ", "\n   "); // add extra indentation to code in typemap
        if (is_post_code) {
          Insert(tm, 0, "\n    try ");
          Printv(tm, " finally {\n", post_code, "\n    }", NIL);
        } else {
          Insert(tm, 0, "\n    ");
        }
        if (is_pre_code) {
          Insert(tm, 0, pre_code);
          Insert(tm, 0, "\n");
        }
        if (is_terminator_code) {
          Printv(tm, "\n", terminator_code, NIL);
        }
	Insert(tm, 0, "{");
	Printf(tm, "\n  }");
      }
      if (GetFlag(n, "feature:new"))
	Replaceall(tm, "$owner", "true");
      else
	Replaceall(tm, "$owner", "false");
      substituteClassname(t, tm);

      // For director methods: generate code to selectively make a normal
      // polymorphic call or an explicit method call. Needed to prevent infinite
      // recursion when calling director methods.
      Node *explicit_n = Getattr(n, "explicitcallnode");
      if (explicit_n) {
	String *ex_overloaded_name = getOverloadedName(explicit_n);
	String *ex_intermediary_function_name = Swig_name_member(proxy_class_name, ex_overloaded_name);

	String *ex_imcall = Copy(imcall);
	Replaceall(ex_imcall, "$imfuncname", ex_intermediary_function_name);
	Replaceall(imcall, "$imfuncname", intermediary_function_name);

	String *excode = NewString("");
	if (!Cmp(return_type, "void"))
	  Printf(excode, "if (this.classinfo == %s.classinfo) %s; else %s", proxy_class_name, imcall, ex_imcall);
	else
	  Printf(excode, "((this.classinfo == %s.classinfo) ? %s : %s)", proxy_class_name, imcall, ex_imcall);

	Clear(imcall);
	Printv(imcall, excode, NIL);
	Delete(ex_overloaded_name);
	Delete(excode);
      } else {
	Replaceall(imcall, "$imfuncname", intermediary_function_name);
      }
      Replaceall(tm, "$imcall", imcall);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, "No csout typemap defined for %s\n", SwigType_str(t, 0));
    }

    // The whole function code is now in stored tm (if there was a matching
    // type map, of course), so simply append it to the code buffer.
    Printv(function_code, tm, NIL);

    // Add extra indentation
    Replaceall(function_code, "\n", "\n  ");

    // Write function code buffer to the class code.
    Printv(proxy_class_code, function_code, "\n\n", NIL);

    Delete(pre_code);
    Delete(post_code);
    Delete(terminator_code);
    Delete(function_code);
    Delete(return_type);
    Delete(imcall);
  }

  /* ---------------------------------------------------------------------------
   * D::constructorHandler()
   * --------------------------------------------------------------------------- */
  virtual int constructorHandler(Node *n) {
    ParmList *l = Getattr(n, "parms");
    String *tm;
    Parm *p;
    int i;
    String *proxy_constructor_code = NewString("");
    String *helper_code = NewString(""); // Holds code for the constructor helper method generated only when the csin typemap has code in the pre or post attributes
    String *helper_args = NewString("");
    String *pre_code = NewString("");
    String *post_code = NewString("");
    String *terminator_code = NewString("");
    String *im_return_type = NewString("");
    bool feature_director = (parentNode(n) && Swig_directorclass(n));

    Language::constructorHandler(n);

    // Wrappers not wanted for some methods where the parameters cannot be overloaded in D.
    if (Getattr(n, "overload:ignore"))
      return SWIG_OK;

    if (proxy_flag) {
      String *overloaded_name = getOverloadedName(n);
      String *mangled_overname = Swig_name_construct(overloaded_name);
      String *imcall = NewString("");

      const String *methodmods = Getattr(n, "feature:d:methodmodifiers");
      methodmods = methodmods ? methodmods : (is_public(n) ? public_string : protected_string);

      tm = Getattr(n, "tmap:imtype"); // typemaps were attached earlier to the node
      String *imtypeout = Getattr(n, "tmap:imtype:out");	// the type in the imtype typemap's out attribute overrides the type in the typemap
      if (imtypeout)
	tm = imtypeout;
      Printf(im_return_type, "%s", tm);

      Printf(proxy_constructor_code, "  %s this(", methodmods);
      Printf(helper_code, "  static private %s SwigConstruct%s(", im_return_type, proxy_class_name);

      Printv(imcall, wrap_dmodule_fq_name, ".", mangled_overname, "(", NIL);

      /* Attach the non-standard typemaps to the parameter list */
      Swig_typemap_attach_parms("in", l, NULL);
      Swig_typemap_attach_parms("cstype", l, NULL);
      Swig_typemap_attach_parms("csin", l, NULL);

      emit_mark_varargs(l);

      int gencomma = 0;

      /* Output each parameter */
      for (i = 0, p = l; p; i++) {

	/* Ignored varargs */
	if (checkAttribute(p, "varargs:ignore", "1")) {
	  p = nextSibling(p);
	  continue;
	}

	/* Ignored parameters */
	if (checkAttribute(p, "tmap:in:numinputs", "0")) {
	  p = Getattr(p, "tmap:in:next");
	  continue;
	}

	SwigType *pt = Getattr(p, "type");
	String *param_type = NewString("");

	/* Get the C# parameter type */
	if ((tm = Getattr(p, "tmap:cstype"))) {
	  substituteClassname(pt, tm);
	  const String *inattributes = Getattr(p, "tmap:cstype:inattributes");
	  Printf(param_type, "%s%s", inattributes ? inattributes : empty_string, tm);
	} else {
	  Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(pt, 0));
	}

	if (gencomma)
	  Printf(imcall, ", ");

	String *arg = makeParameterName(n, p, i, false);
        String *cshin = 0;

	// Use typemaps to transform type used in C# wrapper function (in proxy class) to type used in PInvoke function (in intermediary class)
	if ((tm = Getattr(p, "tmap:csin"))) {
	  substituteClassname(pt, tm);
	  Replaceall(tm, "$csinput", arg);
          String *pre = Getattr(p, "tmap:csin:pre");
          if (pre) {
            substituteClassname(pt, pre);
            Replaceall(pre, "$csinput", arg);
            if (Len(pre_code) > 0)
              Printf(pre_code, "\n");
            Printv(pre_code, pre, NIL);
          }
          String *post = Getattr(p, "tmap:csin:post");
          if (post) {
            substituteClassname(pt, post);
            Replaceall(post, "$csinput", arg);
            if (Len(post_code) > 0)
              Printf(post_code, "\n");
            Printv(post_code, post, NIL);
          }
          String *terminator = Getattr(p, "tmap:csin:terminator");
          if (terminator) {
            substituteClassname(pt, terminator);
            Replaceall(terminator, "$csinput", arg);
            if (Len(terminator_code) > 0)
              Insert(terminator_code, 0, "\n");
            Insert(terminator_code, 0, terminator);
          }
          cshin = Getattr(p, "tmap:csin:cshin");
          if (cshin)
            Replaceall(cshin, "$csinput", arg);
	  Printv(imcall, tm, NIL);
	} else {
	  Swig_warning(WARN_CSHARP_TYPEMAP_CSIN_UNDEF, input_file, line_number, "No csin typemap defined for %s\n", SwigType_str(pt, 0));
	}

	/* Add parameter to proxy function */
	if (gencomma) {
	  Printf(proxy_constructor_code, ", ");
	  Printf(helper_code, ", ");
	  Printf(helper_args, ", ");
        }
	Printf(proxy_constructor_code, "%s %s", param_type, arg);
	Printf(helper_code, "%s %s", param_type, arg);
	Printf(helper_args, "%s", cshin ? cshin : arg);
	++gencomma;

	Delete(cshin);
	Delete(arg);
	Delete(param_type);
	p = Getattr(p, "tmap:in:next");
      }

      Printf(imcall, ")");

      Printf(proxy_constructor_code, ")");
      Printf(helper_code, ")");

      // Insert the dconstructor typemap (replacing $directorconnect as needed).
      Hash *attributes = NewHash();
      String *construct_tm = Copy( typemapLookup( n, "dconstructor",
	Getattr( n, "name" ), WARN_D_TYPEMAP_DCONSTRUCTOR_UNDEF, attributes ) );
      if (construct_tm) {
	if (!feature_director) {
	  Replaceall(construct_tm, "$directorconnect", "");
	} else {
	  String *connect_attr = Getattr(attributes, "tmap:dconstructor:directorconnect");

	  if (connect_attr) {
	    Replaceall(construct_tm, "$directorconnect", connect_attr);
	  } else {
	    Swig_warning(WARN_D_NO_DIRECTORCONNECT_ATTR, input_file, line_number, "\"directorconnect\" attribute missing in %s \"csconstruct\" typemap.\n",
			 Getattr(n, "name"));
	    Replaceall(construct_tm, "$directorconnect", "");
	  }
	}

	Printv(proxy_constructor_code, " ", construct_tm, NIL);
      }

      excodeSubstitute(n, proxy_constructor_code, "dconstructor", attributes);

      bool is_pre_code = Len(pre_code) > 0;
      bool is_post_code = Len(post_code) > 0;
      bool is_terminator_code = Len(terminator_code) > 0;
      if (is_pre_code || is_post_code || is_terminator_code) {
        Printf(helper_code, " {\n");
        if (is_pre_code) {
          Printv(helper_code, pre_code, "\n", NIL);
        }
        if (is_post_code) {
          Printf(helper_code, "    try {\n");
          Printv(helper_code, "      return ", imcall, ";\n", NIL);
          Printv(helper_code, "    } finally {\n", post_code, "\n    }", NIL);
        } else {
          Printv(helper_code, "    return ", imcall, ";", NIL);
        }
        if (is_terminator_code) {
          Printv(helper_code, "\n", terminator_code, NIL);
        }
        Printf(helper_code, "\n  }\n");
        String *helper_name = NewStringf("%s.SwigConstruct%s(%s)", proxy_class_name, proxy_class_name, helper_args);
        String *im_outattributes = Getattr(n, "tmap:imtype:outattributes");
        if (im_outattributes)
          Printf(proxy_class_code, "  %s\n", im_outattributes);
        Printv(proxy_class_code, helper_code, "\n", NIL);
        Replaceall(proxy_constructor_code, "$imcall", helper_name);
        Delete(helper_name);
      } else {
        Replaceall(proxy_constructor_code, "$imcall", imcall);
      }

      Printv(proxy_class_code, proxy_constructor_code, "\n", NIL);

      Delete(helper_args);
      Delete(im_return_type);
      Delete(pre_code);
      Delete(post_code);
      Delete(terminator_code);
      Delete(construct_tm);
      Delete(attributes);
      Delete(overloaded_name);
      Delete(imcall);
    }

    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::destructorHandler()
   * --------------------------------------------------------------------------- */
  virtual int destructorHandler(Node *n) {
    Language::destructorHandler(n);
    String *symname = Getattr(n, "sym:name");

    if (proxy_flag) {
      Printv(destructor_call, wrap_dmodule_fq_name, ".", Swig_name_destroy(symname), "(__swig_cObject)", NIL);
    }
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::membervariableHandler()
   * --------------------------------------------------------------------------- */
  virtual int membervariableHandler(Node *n) {
    variable_name = Getattr(n, "sym:name");
    wrapping_member_flag = true;
    variable_wrapper_flag = true;
    Language::membervariableHandler(n);
    wrapping_member_flag = false;
    variable_wrapper_flag = false;

    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::staticmembervariableHandler()
   * --------------------------------------------------------------------------- */
  virtual int staticmembervariableHandler(Node *n) {
    if ( GetFlag(n, "feature:d:const") != 1 ) {
      Delattr(n, "value");
    }

    variable_name = Getattr(n, "sym:name");
    wrapping_member_flag = true;
    static_flag = true;
    Language::staticmembervariableHandler(n);
    wrapping_member_flag = false;
    static_flag = false;

    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::memberconstantHandler()
   * --------------------------------------------------------------------------- */
  virtual int memberconstantHandler(Node *n) {
    variable_name = Getattr(n, "sym:name");
    wrapping_member_flag = true;
    Language::memberconstantHandler(n);
    wrapping_member_flag = false;
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::getOverloadedName()
   * --------------------------------------------------------------------------- */
  String *getOverloadedName(Node *n) {
    /* A C# HandleRef is used for all classes in the SWIG intermediary class.
     * The intermediary class methods are thus mangled when overloaded to give
     * a unique name. */
    String *overloaded_name = NewStringf("%s", Getattr(n, "sym:name"));

    if (Getattr(n, "sym:overloaded")) {
      Printv(overloaded_name, Getattr(n, "sym:overname"), NIL);
    }

    return overloaded_name;
  }

  /* ---------------------------------------------------------------------------
   * D::writeWrapDModuleFunction()
   *
   * Writes a function declaration for the given (C) wrapper function to the
   * wrap D module.
   *
   * d_name - The name the function in the D wrap module will get.
   * wrapper_function_name - The name of the exported function in the C wrapper
   *                         (usually d_name prefixed by »D_«).
   * --------------------------------------------------------------------------- */
  void writeWrapDModuleFunction( const_String_or_char_ptr d_name,
    const_String_or_char_ptr return_type, const_String_or_char_ptr parameters,
    const_String_or_char_ptr wrapper_function_name ) {

    // TODO: Add support for static linking here.
    Printf(wrap_dmodule_code, "extern( C ) %s function%s %s;\n", return_type,
      parameters, d_name);
    Printv(wrapper_loader_bind_code, wrapper_loader_bind_command, NIL);
    Replaceall(wrapper_loader_bind_code, "$function", d_name);
    Replaceall(wrapper_loader_bind_code, "$symbol", wrapper_function_name);
  }

  /* ---------------------------------------------------------------------------
   * D::writeProxyDModuleFunction()
   * --------------------------------------------------------------------------- */
  void writeProxyDModuleFunction(Node *n) {
    SwigType *t = Getattr(n, "type");
    ParmList *l = Getattr(n, "parms");
    String *tm;
    Parm *p;
    int i;
    String *imcall = NewString("");
    String *return_type = NewString("");
    String *function_code = NewString("");
    int num_arguments = 0;
    int num_required = 0;
    String *overloaded_name = getOverloadedName(n);
    String *func_name = NULL;
    String *pre_code = NewString("");
    String *post_code = NewString("");
    String *terminator_code = NewString("");

    // RESEARCH: What is this good for?
    if (l) {
      if (SwigType_type(Getattr(l, "type")) == T_VOID) {
	l = nextSibling(l);
      }
    }

    /* Attach the non-standard typemaps to the parameter list */
    Swig_typemap_attach_parms("cstype", l, NULL);
    Swig_typemap_attach_parms("csin", l, NULL);

    /* Get return types */
    if ((tm = Swig_typemap_lookup("cstype", n, "", 0))) {
      String *cstypeout = Getattr(n, "tmap:cstype:out");	// the type in the cstype typemap's out attribute overrides the type in the typemap
      if (cstypeout)
	tm = cstypeout;
      substituteClassname(t, tm);
      Printf(return_type, "%s", tm);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(t, 0));
    }

    /* Change function name for global variables */
    if (proxy_flag && global_variable_flag) {
      // RESEARCH: Is the Copy() needed here?
      func_name = Copy(variable_name);
    } else {
      func_name = Copy(Getattr(n, "sym:name"));
    }

    /* Start generating the function */
    const String *outattributes = Getattr(n, "tmap:cstype:outattributes");
    if (outattributes)
      Printf(function_code, "  %s\n", outattributes);

    const String *methodmods = Getattr(n, "feature:d:methodmodifiers");
    // TODO: Check if is_public(n) could possibly make any sense here
    // (private global functions would be useless anyway?).
    methodmods = methodmods ? methodmods : empty_string;

    Printf(function_code, "%s%s %s(", methodmods, return_type, func_name);
    Printv(imcall, wrap_dmodule_fq_name, ".", overloaded_name, "(", NIL);

    /* Get number of required and total arguments */
    num_arguments = emit_num_arguments(l);
    num_required = emit_num_required(l);

    int gencomma = 0;

    /* Output each parameter */
    for (i = 0, p = l; i < num_arguments; i++) {

      /* Ignored parameters */
      while (checkAttribute(p, "tmap:in:numinputs", "0")) {
	p = Getattr(p, "tmap:in:next");
      }

      SwigType *pt = Getattr(p, "type");
      String *param_type = NewString("");

      /* Get the C# parameter type */
      if ((tm = Getattr(p, "tmap:cstype"))) {
	substituteClassname(pt, tm);
	const String *inattributes = Getattr(p, "tmap:cstype:inattributes");
	Printf(param_type, "%s%s", inattributes ? inattributes : empty_string, tm);
      } else {
	Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(pt, 0));
      }

      if (gencomma)
	Printf(imcall, ", ");

      const bool generating_setter = global_variable_flag || wrapping_member_flag;
      String *arg = makeParameterName(n, p, i, generating_setter);

      // Use typemaps to transform type used in C# wrapper function (in proxy class) to type used in PInvoke function (in intermediary class)
      if ((tm = Getattr(p, "tmap:csin"))) {
	substituteClassname(pt, tm);
	Replaceall(tm, "$csinput", arg);
	String *pre = Getattr(p, "tmap:csin:pre");
	if (pre) {
	  substituteClassname(pt, pre);
	  Replaceall(pre, "$csinput", arg);
          if (Len(pre_code) > 0)
            Printf(pre_code, "\n");
	  Printv(pre_code, pre, NIL);
	}
	String *post = Getattr(p, "tmap:csin:post");
	if (post) {
	  substituteClassname(pt, post);
	  Replaceall(post, "$csinput", arg);
          if (Len(post_code) > 0)
            Printf(post_code, "\n");
	  Printv(post_code, post, NIL);
	}
        String *terminator = Getattr(p, "tmap:csin:terminator");
        if (terminator) {
          substituteClassname(pt, terminator);
          Replaceall(terminator, "$csinput", arg);
          if (Len(terminator_code) > 0)
            Insert(terminator_code, 0, "\n");
          Insert(terminator_code, 0, terminator);
        }
	Printv(imcall, tm, NIL);
      } else {
	Swig_warning(WARN_CSHARP_TYPEMAP_CSIN_UNDEF, input_file, line_number, "No csin typemap defined for %s\n", SwigType_str(pt, 0));
      }

      /* Add parameter to module class function */
      if (gencomma >= 2)
	Printf(function_code, ", ");
      gencomma = 2;
      Printf(function_code, "%s %s", param_type, arg);

      p = Getattr(p, "tmap:in:next");
      Delete(arg);
      Delete(param_type);
    }

    Printf(imcall, ")");
    Printf(function_code, ")");

    // Transform return type used in PInvoke function (in intermediary class) to type used in C# wrapper function (in module class)
    if ((tm = Swig_typemap_lookup("csout", n, "", 0))) {
      excodeSubstitute(n, tm, "csout", n);
      bool is_pre_code = Len(pre_code) > 0;
      bool is_post_code = Len(post_code) > 0;
      bool is_terminator_code = Len(terminator_code) > 0;
      if (is_pre_code || is_post_code || is_terminator_code) {
        if (is_post_code) {
          Insert(tm, 0, "\n  try ");
          Printv(tm, " finally {\n", post_code, "\n  }", NIL);
        } else {
          Insert(tm, 0, "\n  ");
        }
        if (is_pre_code) {
          Insert(tm, 0, pre_code);
          Insert(tm, 0, "\n");
        }
        if (is_terminator_code) {
          Printv(tm, "\n", terminator_code, NIL);
        }
	Insert(tm, 0, " {");
	Printf(tm, "\n}");
      }
      if (GetFlag(n, "feature:new"))
	Replaceall(tm, "$owner", "true");
      else
	Replaceall(tm, "$owner", "false");
      substituteClassname(t, tm);
      Replaceall(tm, "$imcall", imcall);
    } else {
      Swig_warning(WARN_CSHARP_TYPEMAP_CSOUT_UNDEF, input_file, line_number, "No csout typemap defined for %s\n", SwigType_str(t, 0));
    }

    // The whole function code is now in stored tm (if there was a matching
    // type map, of course), so simply append it to the code buffer.
    Printf(function_code, " %s\n\n", tm ? (const String *) tm : empty_string);
    Printv(proxy_dmodule_code, function_code, NIL);

    Delete(pre_code);
    Delete(post_code);
    Delete(terminator_code);
    Delete(function_code);
    Delete(return_type);
    Delete(imcall);
    Delete(func_name);
  }

  /* ---------------------------------------------------------------------------
   * D::replaceSpecialVariables()
   * --------------------------------------------------------------------------- */
  virtual void replaceSpecialVariables(String *method, String *tm, Parm *parm) {
    (void)method;
    SwigType *type = Getattr(parm, "type");
    substituteClassname(type, tm);
  }

  /* ---------------------------------------------------------------------------
   * D::enumValue()
   *
   * This method will return a string with an enum value to use in C# generated
   * code. If the %csconst feature is not used, the string will contain the intermediary
   * class call to obtain the enum value. The intermediary class and PINVOKE methods to obtain
   * the enum value will be generated. Otherwise the C/C++ enum value will be used if there
   * is one and hopefully it will compile as C# code - e.g. 20 as in: enum E{e=20};
   * The %dconstvalue feature overrides all other ways to generate the constant value.
   * The caller must delete memory allocated for the returned string.
   * --------------------------------------------------------------------------- */
  String *enumValue(Node *n) {
    String *symname = Getattr(n, "sym:name");

    // Check for the %dconstvalue feature
    String *value = Getattr(n, "feature:d:constvalue");

    if (!value) {
      // The %csconst feature determines how the constant value is obtained
      int const_feature_flag = GetFlag(n, "feature:d:const");

      if (const_feature_flag) {
	// Use the C syntax to make a true C# constant and hope that it compiles as C# code
	value = Getattr(n, "enumvalue") ? Copy(Getattr(n, "enumvalue")) : Copy(Getattr(n, "enumvalueex"));
      } else {
	// Get the enumvalue from a PINVOKE call
	if (!getCurrentClass() || !cparse_cplusplus || !proxy_flag) {
	  // Strange hack to change the name
	  Setattr(n, "name", Getattr(n, "value"));	/* for wrapping of enums in a namespace when emit_action is used */
	  constantWrapper(n);
	  value = NewStringf("%s.%s()", wrap_dmodule_fq_name, Swig_name_get(symname));
	} else {
	  memberconstantHandler(n);
	  value = NewStringf("%s.%s()", wrap_dmodule_fq_name, Swig_name_get(Swig_name_member(proxy_class_name, symname)));
	}
      }
    }
    return value;
  }

  /* ---------------------------------------------------------------------------
   * D::getEnumName()
   * --------------------------------------------------------------------------- */
  String *getEnumName(SwigType *t) {
    Node *enum_name = NULL;
    Node *n = enumLookup(t);
    if (n) {
      String *symname = Getattr(n, "sym:name");
      if (symname) {
	// Add in class scope when referencing enum if not a global enum
	String *scopename_prefix = Swig_scopename_prefix(Getattr(n, "name"));
	String *proxyname = 0;
	if (scopename_prefix) {
	  proxyname = getProxyName(scopename_prefix);
	}
	if (proxyname)
	  enum_name = NewStringf("%s.%s", proxyname, symname);
	else
	  enum_name = NewStringf("%s", symname);
	Delete(scopename_prefix);
      }
    }

    return enum_name;
  }

  /* ---------------------------------------------------------------------------
   * D::substituteClassname()
   *
   * Substitute the special variable $dclassname with the proxy class name for classes/structs/unions
   * that SWIG knows about. Also substitutes enums with enum name.
   * Otherwise use the $descriptor name for the D class name. Note that the $&dclassname substitution
   * is the same as a $&descriptor substitution, ie one pointer added to descriptor name.
   * Inputs:
   *   pt - parameter type
   *   tm - typemap contents that might contain the special variable to be replaced
   * Outputs:
   *   tm - typemap contents complete with the special variable substitution
   * Return:
   *   substitution_performed - flag indicating if a substitution was performed
   * --------------------------------------------------------------------------- */
  bool substituteClassname(SwigType *pt, String *tm) {
    bool substitution_performed = false;
    SwigType *type = Copy(SwigType_typedef_resolve_all(pt));
    SwigType *strippedtype = SwigType_strip_qualifiers(type);

    if (Strstr(tm, "$dclassname")) {
      SwigType *classnametype = Copy(strippedtype);
      substituteClassnameSpecialVariable(classnametype, tm, "$dclassname");
      substitution_performed = true;
      Delete(classnametype);
    }
    if (Strstr(tm, "$*dclassname")) {
      SwigType *classnametype = Copy(strippedtype);
      Delete(SwigType_pop(classnametype));
      substituteClassnameSpecialVariable(classnametype, tm, "$*dclassname");
      substitution_performed = true;
      Delete(classnametype);
    }
    if (Strstr(tm, "$&dclassname")) {
      SwigType *classnametype = Copy(strippedtype);
      SwigType_add_pointer(classnametype);
      substituteClassnameSpecialVariable(classnametype, tm, "$&dclassname");
      substitution_performed = true;
      Delete(classnametype);
    }

    Delete(strippedtype);
    Delete(type);

    return substitution_performed;
  }

  /* ---------------------------------------------------------------------------
   * D::substituteClassnameSpecialVariable()
   * --------------------------------------------------------------------------- */
  void substituteClassnameSpecialVariable(SwigType *classnametype, String *tm, const char *classnamespecialvariable) {
    if (SwigType_isenum(classnametype)) {
      String *enumname = getEnumName(classnametype);
      if (enumname)
	Replaceall(tm, classnamespecialvariable, enumname);
      else
	Replaceall(tm, classnamespecialvariable, NewStringf("int"));
    } else {
      String *classname = getProxyName(classnametype);
      if (classname) {
	Replaceall(tm, classnamespecialvariable, classname);	// getProxyName() works for pointers to classes too
      } else {			// use $descriptor if SWIG does not know anything about this type. Note that any typedefs are resolved.
	String *descriptor = NewStringf("SWIGTYPE%s", SwigType_manglestr(classnametype));
	Replaceall(tm, classnamespecialvariable, descriptor);

	// Add to hash table so that the type wrapper classes can be created later
	Setattr(swig_types_hash, descriptor, classnametype);
	Delete(descriptor);
      }
    }
  }

  /* ---------------------------------------------------------------------------
   * D::makeParameterName()
   *
   * Inputs:
   *   n - Node
   *   p - parameter node
   *   arg_num - parameter argument number
   *   setter  - set this flag when wrapping variables
   * Return:
   *   arg - a unique parameter name
   * --------------------------------------------------------------------------- */
  String *makeParameterName(Node *n, Parm *p, int arg_num, bool setter) {

    String *arg = 0;
    String *pn = Getattr(p, "name");

    // Use C parameter name unless it is a duplicate or an empty parameter name
    int count = 0;
    ParmList *plist = Getattr(n, "parms");
    while (plist) {
      if ((Cmp(pn, Getattr(plist, "name")) == 0))
        count++;
      plist = nextSibling(plist);
    }
    String *wrn = pn ? Swig_name_warning(p, 0, pn, 0) : 0;
    arg = (!pn || (count > 1) || wrn) ? NewStringf("arg%d", arg_num) : Copy(pn);

    if (setter && Cmp(arg, "self") != 0) {
      // In theory, we could use the normal parameter name for setter functions.
      // Unfortunately, it is set to "Class::VariableName" for static public
      // members by the parser, which is not legal D syntax. Thus, we just force
      // it to "value".
      Delete(arg);
      arg = NewString("value");
    }

    return arg;
  }

  /* ---------------------------------------------------------------------------
   * D::writeTypeWrapperClass()
   * --------------------------------------------------------------------------- */
  void writeTypeWrapperClass(String *classname, SwigType *type) {
    Node *n = NewHash();
    Setfile(n, input_file);
    Setline(n, line_number);

    // Import statements.
    Printv(wrap_dmodule_imports, typemapLookup(n, "dimports", type, WARN_NONE), NIL);

    // Pure C# baseclass and interfaces
    const String *pure_baseclass = typemapLookup(n, "csbase", type, WARN_NONE);
    const String *pure_interfaces = typemapLookup(n, "dinterfaces", type, WARN_NONE);

    // Emit the class.
    Printv(proxy_dmodule_code,
      typemapLookup(n, "dclassmodifiers", type, WARN_CSHARP_TYPEMAP_CLASSMOD_UNDEF),	// Class modifiers
      " $dclassname",	// Class name and base class
      (*Char(pure_baseclass) || *Char(pure_interfaces)) ? " : " : "", pure_baseclass,
      ((*Char(pure_baseclass)) && *Char(pure_interfaces)) ? ", " : "", pure_interfaces, // Interfaces
      " {",
      typemapLookup(n, "dbody", type, WARN_D_TYPEMAP_DBODY_UNDEF),	// main body of class
      typemapLookup(n, "cscode", type, WARN_NONE),	// extra D code
      "}\n\n", NIL);

    Replaceall(proxy_dmodule_code, "$dclassname", classname);
    Replaceall(proxy_dmodule_code, "$proxydmodule", proxy_dmodule_fq_name);
    Replaceall(proxy_dmodule_code, "$wrapdmodule", wrap_dmodule_fq_name);
    Replaceall(proxy_dmodule_code, "$module", proxy_dmodule_name);

    Delete(n);
  }

  /* ---------------------------------------------------------------------------
   * D::typemapLookup()
   *
   * n - for input only and must contain info for Getfile(n) and Getline(n) to work
   * tmap_method - typemap method name
   * type - typemap type to lookup
   * warning - warning number to issue if no typemaps found
   * typemap_attributes - the typemap attributes are attached to this node and will
   *   also be used for temporary storage if non null
   * return is never NULL, unlike Swig_typemap_lookup()
   * --------------------------------------------------------------------------- */
  const String *typemapLookup(Node *n, const_String_or_char_ptr tmap_method, SwigType *type, int warning, Node *typemap_attributes = 0) {
    Node *node = !typemap_attributes ? NewHash() : typemap_attributes;
    Setattr(node, "type", type);
    Setfile(node, Getfile(n));
    Setline(node, Getline(n));
    const String *tm = Swig_typemap_lookup(tmap_method, node, "", 0);
    if (!tm) {
      tm = empty_string;
      if (warning != WARN_NONE)
	Swig_warning(warning, Getfile(n), Getline(n), "No %s typemap defined for %s\n", tmap_method, SwigType_str(type, 0));
    }
    if (!typemap_attributes)
      Delete(node);
    return tm;
  }

  /* ---------------------------------------------------------------------------
   * D::canThrow()
   *
   * Determine whether the code in the typemap can throw a C# exception.
   * If so, note it for later when excodeSubstitute() is called.
   * --------------------------------------------------------------------------- */
  void canThrow(Node *n, const String *typemap, Node *parameter) {
    String *canthrow_attribute = NewStringf("tmap:%s:canthrow", typemap);
    String *canthrow = Getattr(parameter, canthrow_attribute);
    if (canthrow)
      Setattr(n, "csharp:canthrow", "1");
    Delete(canthrow_attribute);
  }

  /* ---------------------------------------------------------------------------
   * D::excodeSubstitute()
   *
   * If a method can throw a C# exception, additional exception code is added to
   * check for the pending exception so that it can then throw the exception. The
   * $excode special variable is replaced by the exception code in the excode
   * typemap attribute.
   * --------------------------------------------------------------------------- */

  void excodeSubstitute(Node *n, String *code, const String *typemap, Node *parameter) {
    String *excode_attribute = NewStringf("tmap:%s:excode", typemap);
    String *excode = Getattr(parameter, excode_attribute);
    if (Getattr(n, "csharp:canthrow")) {
      int count = Replaceall(code, "$excode", excode);
      if (count < 1 || !excode) {
	Swig_warning(WARN_CSHARP_EXCODE, input_file, line_number,
		     "C# exception may not be thrown - no $excode or excode attribute in '%s' typemap.\n", typemap);
      }
    } else {
      Replaceall(code, "$excode", empty_string);
    }
    Delete(excode_attribute);
  }


  /*
   * Start of director methods
   */

  /* ---------------------------------------------------------------------------
   * D::emitDirectorExtraMethods()
   *
   * This is where the director connect method is generated.
   * --------------------------------------------------------------------------- */
  void emitDirectorExtraMethods(Node *n) {
    if (!Swig_directorclass(n))
      return;

    // Output the director connect method.
    String *norm_name = SwigType_namestr(Getattr(n, "name"));
    String *connect_name = NewStringf("%s_director_connect", proxy_class_name);
    String *sym_name = Getattr(n, "sym:name");
    Wrapper *code_wrap;

    Printv(wrapper_loader_bind_code, wrapper_loader_bind_command, NIL);
    Replaceall(wrapper_loader_bind_code, "$function", connect_name);
    Replaceall(wrapper_loader_bind_code, "$symbol", Swig_name_wrapper( connect_name ));

    Printf(wrap_dmodule_code, "extern( C ) void function(void* cObject, void* dObject");

    code_wrap = NewWrapper();
    Printf(code_wrap->def, "SWIGEXPORT void SWIGSTDCALL D_%s(void *objarg, void *dobj", connect_name);

    Printf(code_wrap->code, "  %s *obj = (%s *)objarg;\n", norm_name, norm_name);
    Printf(code_wrap->code, "  SwigDirector_%s *director = dynamic_cast<SwigDirector_%s *>(obj);\n", sym_name, sym_name);
    // TODO: if statement not needed?? - Java too
    Printf(code_wrap->code, "  if (director) {\n");
    Printf(code_wrap->code, "    director->swig_connect_director(dobj");

    for (int i = first_class_dmethod; i < curr_class_dmethod; ++i) {
      UpcallData *udata = Getitem(dmethods_seq, i);
      String *methid = Getattr(udata, "class_methodidx");

      Printf(code_wrap->def, ", SwigDirector_%s::SWIG_Callback%s_t callback%s", sym_name, methid, methid);
      Printf(code_wrap->code, ", callback%s", methid);
      Printf(wrap_dmodule_code, ", __SwigDirector_%s_Callback%s callback%s", sym_name, methid, methid);
    }

    Printf(code_wrap->def, ") {\n");
    Printf(code_wrap->code, ");\n");
    Printf(wrap_dmodule_code, ") %s;\n", connect_name);
    Printf(code_wrap->code, "  }\n");
    Printf(code_wrap->code, "}\n");

    Wrapper_print(code_wrap, f_wrappers);
    DelWrapper(code_wrap);

    Delete(connect_name);
  }

  /* ---------------------------------------------------------------------------
   * D::classDirectorMethod()
   *
   * Emit a virtual director method to pass a method call on to the
   * underlying D object.
   * --------------------------------------------------------------------------- */
  int classDirectorMethod(Node *n, Node *parent, String *super) {
    String *empty_str = NewString("");
    String *classname = Getattr(parent, "sym:name");
    String *c_classname = Getattr(parent, "name");
    String *name = Getattr(n, "name");
    String *symname = Getattr(n, "sym:name");
    SwigType *type = Getattr(n, "type");
    SwigType *returntype = Getattr(n, "returntype");
    String *overloaded_name = getOverloadedName(n);
    String *storage = Getattr(n, "storage");
    String *value = Getattr(n, "value");
    String *decl = Getattr(n, "decl");
    String *declaration = NewString("");
    String *tm;
    Parm *p;
    int i;
    Wrapper *w = NewWrapper();
    ParmList *l = Getattr(n, "parms");
    bool is_void = !(Cmp(returntype, "void"));
    String *qualified_return = NewString("");
    bool pure_virtual = (!(Cmp(storage, "virtual")) && !(Cmp(value, "0")));
    int status = SWIG_OK;
    bool output_director = true;
    String *dirclassname = directorClassName(parent);
    String *qualified_name = NewStringf("%s::%s", dirclassname, name);
    SwigType *c_ret_type = NULL;
    String *dcallback_call_args = NewString("");
    String *imclass_dmethod;
    String *callback_typedef_parms = NewString("");
    String *delegate_parms = NewString("");
    String *proxy_method_param_list = NewString("");
    String *proxy_callback_return_type = NewString("");
    String *callback_def = NewString("");
    String *callback_code = NewString("");
    String *imcall_args = NewString("");
    int gencomma = 0;
    bool ignored_method = GetFlag(n, "feature:ignore") ? true : false;

    // Kludge Alert: functionWrapper sets sym:overload properly, but it
    // isn't at this point, so we have to manufacture it ourselves. At least
    // we're consistent with the sym:overload name in functionWrapper. (?? when
    // does the overloaded method name get set?)

    imclass_dmethod = NewStringf("SwigDirector_%s", Swig_name_member(classname, overloaded_name));

    if (returntype) {
      qualified_return = SwigType_rcaststr(returntype, "c_result");

      if (!is_void && !ignored_method) {
	if (!SwigType_isclass(returntype)) {
	  if (!(SwigType_ispointer(returntype) || SwigType_isreference(returntype))) {
            String *construct_result = NewStringf("= SwigValueInit< %s >()", SwigType_lstr(returntype, 0));
	    Wrapper_add_localv(w, "c_result", SwigType_lstr(returntype, "c_result"), construct_result, NIL);
            Delete(construct_result);
	  } else {
	    String *base_typename = SwigType_base(returntype);
	    String *resolved_typename = SwigType_typedef_resolve_all(base_typename);
	    Symtab *symtab = Getattr(n, "sym:symtab");
	    Node *typenode = Swig_symbol_clookup(resolved_typename, symtab);

	    if (SwigType_ispointer(returntype) || (typenode && Getattr(typenode, "abstract"))) {
	      /* initialize pointers to something sane. Same for abstract
	         classes when a reference is returned. */
	      Wrapper_add_localv(w, "c_result", SwigType_lstr(returntype, "c_result"), "= 0", NIL);
	    } else {
	      /* If returning a reference, initialize the pointer to a sane
	         default - if a C# exception occurs, then the pointer returns
	         something other than a NULL-initialized reference. */
	      String *non_ref_type = Copy(returntype);

	      /* Remove reference and const qualifiers */
	      Replaceall(non_ref_type, "r.", "");
	      Replaceall(non_ref_type, "q(const).", "");
	      Wrapper_add_localv(w, "result_default", "static", SwigType_str(non_ref_type, "result_default"), "=", SwigType_str(non_ref_type, "()"), NIL);
	      Wrapper_add_localv(w, "c_result", SwigType_lstr(returntype, "c_result"), "= &result_default", NIL);

	      Delete(non_ref_type);
	    }

	    Delete(base_typename);
	    Delete(resolved_typename);
	  }
	} else {
	  SwigType *vt;

	  vt = cplus_value_type(returntype);
	  if (!vt) {
	    Wrapper_add_localv(w, "c_result", SwigType_lstr(returntype, "c_result"), NIL);
	  } else {
	    Wrapper_add_localv(w, "c_result", SwigType_lstr(vt, "c_result"), NIL);
	    Delete(vt);
	  }
	}
      }

      /* Create the intermediate class wrapper */
      Parm *tp = NewParmFromNode(returntype, empty_str, n);

      tm = Swig_typemap_lookup("imtype", tp, "", 0);
      if (tm) {
	String *imtypeout = Getattr(tp, "tmap:imtype:out");	// the type in the imtype typemap's out attribute overrides the type in the typemap
	if (imtypeout)
	  tm = imtypeout;
	Printf(callback_def, "extern(C) %s __SwigDirector_%s_%s(void* dObject", tm, classname, overloaded_name);
	Printv( proxy_callback_return_type, tm, NIL );
      } else {
	Swig_warning(WARN_D_TYPEMAP_CSTYPE_UNDEF, input_file, line_number, "No imtype typemap defined for %s\n", SwigType_str(returntype, 0));
      }

      Parm *retpm = NewParmFromNode(returntype, empty_str, n);

      if ((c_ret_type = Swig_typemap_lookup("ctype", retpm, "", 0))) {

	if (!is_void && !ignored_method) {
	  String *jretval_decl = NewStringf("%s jresult", c_ret_type);
	  Wrapper_add_localv(w, "jresult", jretval_decl, "= 0", NIL);
	  Delete(jretval_decl);
	}
      } else {
	Swig_warning(WARN_CSHARP_TYPEMAP_CTYPE_UNDEF, input_file, line_number, "No ctype typemap defined for %s for use in %s::%s (skipping director method)\n",
	    SwigType_str(returntype, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	output_director = false;
      }

      Delete(retpm);
    }

    /* Go through argument list, attach lnames for arguments */
    for (i = 0, p = l; p; p = nextSibling(p), ++i) {
      String *arg = Getattr(p, "name");
      String *lname = NewString("");

      if (!arg && Cmp(Getattr(p, "type"), "void")) {
	lname = NewStringf("arg%d", i);
	Setattr(p, "name", lname);
      } else
	lname = arg;

      Setattr(p, "lname", lname);
    }

    // Attach the standard typemaps.
    Swig_typemap_attach_parms("out", l, 0);
    Swig_typemap_attach_parms("ctype", l, 0);
    Swig_typemap_attach_parms("imtype", l, 0);
    Swig_typemap_attach_parms("cstype", l, 0);
    Swig_typemap_attach_parms("directorin", l, 0);
    Swig_typemap_attach_parms("csdirectorin", l, 0);

    // Preamble code.
    if (!ignored_method)
      Printf(w->code, "if (!swig_callback_%s) {\n", overloaded_name);

    if (!pure_virtual) {
      String *super_call = Swig_method_call(super, l);
      if (is_void) {
	Printf(w->code, "%s;\n", super_call);
	if (!ignored_method)
	  Printf(w->code, "return;\n");
      } else {
	Printf(w->code, "return %s;\n", super_call);
      }
      Delete(super_call);
    } else {
      Printf(w->code, " throw Swig::DirectorPureVirtualException(\"%s::%s\");\n", SwigType_namestr(c_classname), SwigType_namestr(name));
    }

    if (!ignored_method)
      Printf(w->code, "} else {\n");

    // Go through argument list.
    for (p = l; p; /* empty */ ) {
      /* Is this superfluous? */
      while (checkAttribute(p, "tmap:directorin:numinputs", "0")) {
	p = Getattr(p, "tmap:directorin:next");
      }

      SwigType *pt = Getattr(p, "type");
      String *ln = Copy(Getattr(p, "name"));
      String *c_param_type = NULL;
      String *c_decl = NewString("");
      String *arg = NewString("");

      Printf(arg, "j%s", ln);

      // Add each parameter to the D callback invocation arguments.
      Printf(dcallback_call_args, ", %s", arg);

      /* Get parameter's intermediary C type */
      if ((c_param_type = Getattr(p, "tmap:ctype"))) {
	String *ctypeout = Getattr(p, "tmap:ctype:out");	// the type in the ctype typemap's out attribute overrides the type in the typemap
	if (ctypeout)
	  c_param_type = ctypeout;

	Parm *tp = NewParmFromNode(c_param_type, empty_str, n);
	String *desc_tm = NULL;

	/* Add to local variables */
	Printf(c_decl, "%s %s", c_param_type, arg);
	if (!ignored_method)
	  Wrapper_add_localv(w, arg, c_decl, (!(SwigType_ispointer(pt) || SwigType_isreference(pt)) ? "" : "= 0"), NIL);

	/* Add input marshalling code */
	if ((desc_tm = Swig_typemap_lookup("directorin", tp, "", 0))
	    && (tm = Getattr(p, "tmap:directorin"))) {

	  Replaceall(tm, "$input", arg);
	  Replaceall(tm, "$owner", "0");

	  if (Len(tm))
	    if (!ignored_method)
	      Printf(w->code, "%s\n", tm);

	  Delete(tm);

	  // Add parameter type to the C typedef for the D callback function.
	  Printf(callback_typedef_parms, ", %s", c_param_type);

	  /* Add parameter to the intermediate class code if generating the
	   * intermediate's upcall code */
	  if ((tm = Getattr(p, "tmap:imtype"))) {

	    String *imtypeout = Getattr(p, "tmap:imtype:out");	// the type in the imtype typemap's out attribute overrides the type in the typemap
	    if (imtypeout)
	      tm = imtypeout;
            const String *im_directorinattributes = Getattr(p, "tmap:imtype:directorinattributes");

	    String *din = Copy(Getattr(p, "tmap:csdirectorin"));

	    if (din) {
	      Replaceall(din, "$proxydmodule", proxy_dmodule_fq_name);
	      Replaceall(din, "$wrapdmodule", wrap_dmodule_fq_name);
	      Replaceall(din, "$module", proxy_dmodule_name);
	      substituteClassname(pt, din);
	      Replaceall(din, "$iminput", ln);

	      Printf(delegate_parms, ", ");
	      if (gencomma > 0) {
		Printf(proxy_method_param_list, ", ");
		Printf(imcall_args, ", ");
	      }
	      Printf(delegate_parms, "%s%s %s", im_directorinattributes ? im_directorinattributes : empty_string, tm, ln);

	      if (Cmp(din, ln)) {
		Printv(imcall_args, din, NIL);
	      } else {
		Printv(imcall_args, ln, NIL);
	      }

	      // Get the parameter type in the proxy D class.
	      if ((tm = Getattr(p, "tmap:cstype"))) {
		substituteClassname(pt, tm);
		Printf(proxy_method_param_list, "%s", tm);
	      } else {
		Swig_warning(WARN_D_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(pt, 0));
	      }
	    } else {
	      Swig_warning(WARN_D_TYPEMAP_CSDIRECTORIN_UNDEF, input_file, line_number, "No csdirectorin typemap defined for %s for use in %s::%s (skipping director method)\n",
		  SwigType_str(pt, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	      output_director = false;
	    }
	  } else {
	    Swig_warning(WARN_D_TYPEMAP_CSTYPE_UNDEF, input_file, line_number, "No imtype typemap defined for %s for use in %s::%s (skipping director method)\n",
		SwigType_str(pt, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	    output_director = false;
	  }

	  p = Getattr(p, "tmap:directorin:next");

	  Delete(desc_tm);
	} else {
	  if (!desc_tm) {
	    Swig_warning(WARN_D_TYPEMAP_CSDIRECTORIN_UNDEF, input_file, line_number,
			 "No or improper directorin typemap defined for %s for use in %s::%s (skipping director method)\n",
			 SwigType_str(c_param_type, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	    p = nextSibling(p);
	  } else if (!tm) {
	    Swig_warning(WARN_D_TYPEMAP_CSDIRECTORIN_UNDEF, input_file, line_number,
			 "No or improper directorin typemap defined for argument %s for use in %s::%s (skipping director method)\n",
			 SwigType_str(pt, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	    p = nextSibling(p);
	  }

	  output_director = false;
	}

	Delete(tp);
      } else {
	Swig_warning(WARN_D_TYPEMAP_CTYPE_UNDEF, input_file, line_number, "No ctype typemap defined for %s for use in %s::%s (skipping director method)\n",
	    SwigType_str(pt, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	output_director = false;
	p = nextSibling(p);
      }

      gencomma++;
      Delete(arg);
      Delete(c_decl);
      Delete(c_param_type);
    }

    /* header declaration, start wrapper definition */
    String *target;
    SwigType *rtype = Getattr(n, "conversion_operator") ? 0 : type;
    target = Swig_method_decl(rtype, decl, qualified_name, l, 0, 0);
    Printf(w->def, "%s", target);
    Delete(qualified_name);
    Delete(target);
    target = Swig_method_decl(rtype, decl, name, l, 0, 1);
    Printf(declaration, "    virtual %s", target);
    Delete(target);

    // Add any exception specifications to the methods in the director class
    ParmList *throw_parm_list = NULL;
    if ((throw_parm_list = Getattr(n, "throws")) || Getattr(n, "throw")) {
      int gencomma = 0;

      Append(w->def, " throw(");
      Append(declaration, " throw(");

      if (throw_parm_list)
	Swig_typemap_attach_parms("throws", throw_parm_list, 0);
      for (p = throw_parm_list; p; p = nextSibling(p)) {
	if ((tm = Getattr(p, "tmap:throws"))) {
	  if (gencomma++) {
	    Append(w->def, ", ");
	    Append(declaration, ", ");
	  }
	  Printf(w->def, "%s", SwigType_str(Getattr(p, "type"), 0));
	  Printf(declaration, "%s", SwigType_str(Getattr(p, "type"), 0));
	}
      }

      Append(w->def, ")");
      Append(declaration, ")");
    }

    Append(w->def, " {");
    Append(declaration, ";\n");

    // Finish the callback function declaraction.
    Printf(callback_def, "%s)", delegate_parms);
    Printf(callback_def, " {\n");

    /* Emit the intermediate class's upcall to the actual class */

    String *upcall = NewStringf("( cast(%s)dObject ).%s(%s)", classname, symname, imcall_args);

    if (!is_void) {
      Parm *tp = NewParmFromNode(returntype, empty_str, n);

      if ((tm = Swig_typemap_lookup("csdirectorout", tp, "", 0))) {
	substituteClassname(returntype, tm);
	Replaceall(tm, "$cscall", upcall);

	Printf(callback_code, "  return %s;\n", tm);
      }

      Delete(tm);
      Delete(tp);
    } else
      Printf(callback_code, "  %s;\n", upcall);

    Printf(callback_code, "}\n");
    Delete(upcall);

    if (!ignored_method) {
      if (!is_void)
	Printf(w->code, "jresult = (%s) ", c_ret_type);

      Printf(w->code, "swig_callback_%s(d_object%s);\n", overloaded_name, dcallback_call_args);

      if (!is_void) {
	String *jresult_str = NewString("jresult");
	String *result_str = NewString("c_result");
	Parm *tp = NewParmFromNode(returntype, result_str, n);

	/* Copy jresult into c_result... */
	if ((tm = Swig_typemap_lookup("directorout", tp, result_str, w))) {
	  Replaceall(tm, "$input", jresult_str);
	  Replaceall(tm, "$result", result_str);
	  Printf(w->code, "%s\n", tm);
	} else {
	  Swig_warning(WARN_TYPEMAP_DIRECTOROUT_UNDEF, input_file, line_number,
		       "Unable to use return type %s used in %s::%s (skipping director method)\n",
		       SwigType_str(returntype, 0), SwigType_namestr(c_classname), SwigType_namestr(name));
	  output_director = false;
	}

	Delete(tp);
	Delete(jresult_str);
	Delete(result_str);
      }

      /* Terminate wrapper code */
      Printf(w->code, "}\n");
      if (!is_void)
	Printf(w->code, "return %s;", qualified_return);
    }

    Printf(w->code, "}");

    // We expose virtual protected methods via an extra public inline method which makes a straight call to the wrapped class' method
    String *inline_extra_method = NewString("");
    if (dirprot_mode() && !is_public(n) && !pure_virtual) {
      Printv(inline_extra_method, declaration, NIL);
      String *extra_method_name = NewStringf("%sSwigPublic", name);
      Replaceall(inline_extra_method, name, extra_method_name);
      Replaceall(inline_extra_method, ";\n", " {\n      ");
      if (!is_void)
	Printf(inline_extra_method, "return ");
      String *methodcall = Swig_method_call(super, l);
      Printv(inline_extra_method, methodcall, ";\n    }\n", NIL);
      Delete(methodcall);
      Delete(extra_method_name);
    }

    /* emit code */
    if (status == SWIG_OK && output_director) {
      if (!is_void) {
	Replaceall(w->code, "$null", qualified_return);
      } else {
	Replaceall(w->code, "$null", "");
      }
      if (!ignored_method)
	Printv(director_dcallbacks_code, callback_def, callback_code, NIL);
      if (!Getattr(n, "defaultargs")) {
	Wrapper_print(w, f_directors);
	Printv(f_directors_h, declaration, NIL);
	Printv(f_directors_h, inline_extra_method, NIL);
      }
    }

    if (!ignored_method) {
      // Get D return types.
      String *return_type = NewString("");
      String *tm;
      if ((tm = Swig_typemap_lookup("cstype", n, "", 0))) {
	String *cstypeout = Getattr(n, "tmap:cstype:out");	// the type in the cstype typemap's out attribute overrides the type in the typemap
	if (cstypeout)
	  tm = cstypeout;
	substituteClassname(type, tm);
	Printf(return_type, "%s", tm);
      } else {
	Swig_warning(WARN_CSHARP_TYPEMAP_CSWTYPE_UNDEF, input_file, line_number, "No cstype typemap defined for %s\n", SwigType_str(type, 0));
      }

      // Write the callback typedef/member to the C++ director class.
      UpcallData *udata = addUpcallMethod(imclass_dmethod, symname, decl, overloaded_name, return_type, proxy_method_param_list);
      Delete(return_type);
      String *methid = Getattr(udata, "class_methodidx");

      Printf(director_callback_typedefs, "    typedef %s (SWIGSTDCALL* SWIG_Callback%s_t)", c_ret_type, methid);
      Printf(director_callback_typedefs, "(void *dobj%s);\n", callback_typedef_parms);
      Printf(director_callbacks, "    SWIG_Callback%s_t swig_callback_%s;\n", methid, overloaded_name);

      // Write the type alias for the callback to the wrap D module.
      String* proxy_callback_type = NewString("");
      Printf(proxy_callback_type, "__SwigDirector_%s_Callback%s", classname, methid);
      Printf(wrap_dmodule_code, "alias extern(C) %s function(void*%s) %s;\n", proxy_callback_return_type, delegate_parms, proxy_callback_type);
      Delete(proxy_callback_type);
    }

    Delete(qualified_return);
    Delete(c_ret_type);
    Delete(declaration);
    Delete(callback_typedef_parms);
    Delete(delegate_parms);
    Delete(proxy_method_param_list);
    Delete(callback_def);
    Delete(callback_code);
    DelWrapper(w);

    return status;
  }

  /* ---------------------------------------------------------------------------
   * D::classDirectorConstructor()
   * --------------------------------------------------------------------------- */
  int classDirectorConstructor(Node *n) {
    Node *parent = parentNode(n);
    String *decl = Getattr(n, "decl");;
    String *supername = Swig_class_name(parent);
    String *classname = directorClassName(parent);
    String *sub = NewString("");
    Parm *p;
    ParmList *superparms = Getattr(n, "parms");
    ParmList *parms;
    int argidx = 0;

    /* Assign arguments to superclass's parameters, if not already done */
    for (p = superparms; p; p = nextSibling(p)) {
      String *pname = Getattr(p, "name");

      if (!pname) {
	pname = NewStringf("arg%d", argidx++);
	Setattr(p, "name", pname);
      }
    }

    // TODO: Is this copy needed?
    parms = CopyParmList(superparms);

    if (!Getattr(n, "defaultargs")) {
      /* constructor */
      {
	String *basetype = Getattr(parent, "classtype");
	String *target = Swig_method_decl(0, decl, classname, parms, 0, 0);
	String *call = Swig_csuperclass_call(0, basetype, superparms);
	String *classtype = SwigType_namestr(Getattr(n, "name"));

	Printf(f_directors, "%s::%s : %s, %s {\n", classname, target, call, Getattr(parent, "director:ctor"));
	Printf(f_directors, "  swig_init_callbacks();\n");
	Printf(f_directors, "}\n\n");

	Delete(classtype);
	Delete(target);
	Delete(call);
      }

      /* constructor header */
      {
	String *target = Swig_method_decl(0, decl, classname, parms, 0, 1);
	Printf(f_directors_h, "    %s;\n", target);
	Delete(target);
      }
    }

    Delete(sub);
    Delete(supername);
    Delete(parms);
    return Language::classDirectorConstructor(n);
  }

  /* ---------------------------------------------------------------------------
   * D::classDirectorDefaultConstructor()
   * --------------------------------------------------------------------------- */
  int classDirectorDefaultConstructor(Node *n) {
    String *classname = Swig_class_name(n);
    String *classtype = SwigType_namestr(Getattr(n, "name"));
    Wrapper *w = NewWrapper();

    Printf(w->def, "SwigDirector_%s::SwigDirector_%s() : %s {", classname, classname, Getattr(n, "director:ctor"));
    Printf(w->code, "}\n");
    Wrapper_print(w, f_directors);

    Printf(f_directors_h, "    SwigDirector_%s();\n", classname);
    DelWrapper(w);
    Delete(classtype);
    Delete(classname);
    return Language::classDirectorDefaultConstructor(n);
  }


  /* ---------------------------------------------------------------------------
   * D::classDirectorInit()
   * --------------------------------------------------------------------------- */
  int classDirectorInit(Node *n) {
    Delete(none_comparison);
    none_comparison = NewString("");	// not used

    Delete(director_ctor_code);
    director_ctor_code = NewString("$director_new");

    D_director_declaration(n);

    Printf(f_directors_h, "%s {\n", Getattr(n, "director:decl"));
    Printf(f_directors_h, "\npublic:\n");

    /* Keep track of the director methods for this class */
    first_class_dmethod = curr_class_dmethod = n_dmethods;

    director_callback_typedefs = NewString("");
    director_callbacks = NewString("");
    director_dcallbacks_code = NewString("");
    director_delegate_definitions = NewString("");
    director_delegate_instances = NewString("");
    director_method_types = NewString("");
    director_connect_parms = NewString("");

    return Language::classDirectorInit(n);
  }

  /* ---------------------------------------------------------------------------
   * D::classDirectorDestructor()
   * --------------------------------------------------------------------------- */
  int classDirectorDestructor(Node *n) {
    Node *current_class = getCurrentClass();
    String *full_classname = Getattr(current_class, "name");
    String *classname = Swig_class_name(current_class);
    Wrapper *w = NewWrapper();

    if (Getattr(n, "throw")) {
      Printf(f_directors_h, "    virtual ~SwigDirector_%s() throw ();\n", classname);
      Printf(w->def, "SwigDirector_%s::~SwigDirector_%s() throw () {\n", classname, classname);
    } else {
      Printf(f_directors_h, "    virtual ~SwigDirector_%s();\n", classname);
      Printf(w->def, "SwigDirector_%s::~SwigDirector_%s() {\n", classname, classname);
    }

    /* Ensure that correct directordisconnect typemap's method name is called
     * here: */

    const String *disconn_tm = NULL;
    Node *disconn_attr = NewHash();
    String *disconn_methodname = NULL;

    disconn_tm = typemapLookup(n, "directordisconnect", full_classname, WARN_NONE, disconn_attr);
    disconn_methodname = Getattr(disconn_attr, "tmap:directordisconnect:methodname");

    Printv(w->code, "}\n", NIL);

    Wrapper_print(w, f_directors);

    DelWrapper(w);
    Delete(disconn_attr);
    Delete(classname);
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::classDirectorEnd()
   * --------------------------------------------------------------------------- */
  int classDirectorEnd(Node *n) {
    int i;
    String *director_classname = directorClassName(n);

    Wrapper *w = NewWrapper();

    if (Len(director_callback_typedefs) > 0) {
      Printf(f_directors_h, "\n%s", director_callback_typedefs);
    }

    Printf(f_directors_h, "    void swig_connect_director( void* dobj");

    Printf(w->def, "void %s::swig_connect_director( void* dobj", director_classname);
    Printf(w->code, "d_object = dobj;");

    for (i = first_class_dmethod; i < curr_class_dmethod; ++i) {
      UpcallData *udata = Getitem(dmethods_seq, i);
      String *methid = Getattr(udata, "class_methodidx");
      String *overname = Getattr(udata, "overname");

      Printf(f_directors_h, ", SWIG_Callback%s_t callback%s", methid, overname);
      Printf(w->def, ", SWIG_Callback%s_t callback_%s", methid, overname);
      Printf(w->code, "swig_callback_%s = callback_%s;\n", overname, overname);
    }

    Printf(f_directors_h, ");\n");
    Printf(w->def, ") {");

    Printf(f_directors_h, "\nprivate:\n" );
    Printf(f_directors_h, "    void swig_init_callbacks();\n");
    Printf(f_directors_h, "    void *d_object;\n");
    if (Len(director_callback_typedefs) > 0) {
      Printf(f_directors_h, "%s", director_callbacks);
    }
    Printf(f_directors_h, "};\n\n");
    Printf(w->code, "}\n\n");

    Printf(w->code, "void %s::swig_init_callbacks() {\n", director_classname);
    for (i = first_class_dmethod; i < curr_class_dmethod; ++i) {
      UpcallData *udata = Getitem(dmethods_seq, i);
      String *overname = Getattr(udata, "overname");
      Printf(w->code, "swig_callback_%s = 0;\n", overname);
    }
    Printf(w->code, "}");

    Wrapper_print(w, f_directors);

    DelWrapper(w);

    return Language::classDirectorEnd(n);
  }

  /* ---------------------------------------------------------------------------
   * D::classDirectorDisown()
   * --------------------------------------------------------------------------- */
  virtual int classDirectorDisown(Node *n) {
    (void) n;
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------------
   * D::extraDirectorProtectedCPPMethodsRequired()
   * --------------------------------------------------------------------------- */
  bool extraDirectorProtectedCPPMethodsRequired() const {
    return false;
  }

  /* ---------------------------------------------------------------------------
   * D::D_director_declaration()
   *
   * Generate the director class's declaration
   * e.g. "class SwigDirector_myclass : public myclass, public Swig::Director {"
   * --------------------------------------------------------------------------- */
  void D_director_declaration(Node *n) {
    String *base = Getattr(n, "classtype");
    String *class_ctor = NewString("Swig::Director()");

    String *classname = Swig_class_name(n);
    String *directorname = NewStringf("SwigDirector_%s", classname);
    String *declaration = Swig_class_declaration(n, directorname);

    Printf(declaration, " : public %s, public Swig::Director", base);

    // Stash stuff for later.
    Setattr(n, "director:decl", declaration);
    Setattr(n, "director:ctor", class_ctor);
  }
};

static Language *new_swig_d() {
  return new D();
}

/* -----------------------------------------------------------------------------
 * swig_d()    - Instantiate module
 * ----------------------------------------------------------------------------- */
extern "C" Language *swig_d(void) {
  return new_swig_d();
}

/* -----------------------------------------------------------------------------
 * Usage information displayed at the command line.
 * ----------------------------------------------------------------------------- */
const char *D::usage = (char *) "\
D Options (available with -d)\n\
     -wrapperlibrary <wl> - Sets the name of the wrapper library to <wl>\n\
     -package <pkg>       - Write generated D modules into package <pkg>\n\
     -noproxy             - Generate the low-level functional interface instead\n\
                            of proxy classes\n\
\n";
