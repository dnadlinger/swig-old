char cvsroot_allegrocl_cxx[] = "$Header$";

#include "swigmod.h"

class ALLEGROCL : public Language {
public:

  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  virtual int functionWrapper(Node *n); 
  virtual int constantWrapper(Node *n);
  virtual int classDeclaration(Node *n);

};

static File *f_cl=0;
static File *f_null=0;

char *identifier_converter="identifier-convert-null";

int any_varargs(ParmList *pl) {
  Parm *p;
  
  for(p=pl; p; p=nextSibling(p)) {
    if (SwigType_isvarargs(Getattr(p, "type")))
      return 1;
  }

  return 0;
}

  
/* utilities */
/* returns new string w/ parens stripped */
String *strip_parens(String *string) {
	char *s=Char(string), *p;
	int len=Len(string);
	String *res;
	
	if (len==0 || s[0] != '(' || s[len-1] != ')') {
		return NewString(string);
	}
	
	p=(char *)malloc(len-2+1);
	if (!p) {
		Printf(stderr, "Malloc failed\n");
		SWIG_exit(EXIT_FAILURE);
	}
	
	strncpy(p, s+1, len-1);
	p[len-2]=0; /* null terminate */
	
	res=NewString(p);
	free(p);
	
	return res;
}


String *convert_literal(String *num_param, String *type) {
	String *num=strip_parens(num_param), *res;
	char *s=Char(num);
	
	/* Make sure doubles use 'd' instead of 'e' */
	if (!Strcmp(type, "double")) {
		String *updated=Copy(num);
		if (Replace(updated, "e", "d", DOH_REPLACE_ANY) > 1) {
			Printf(stderr, "Weird!! number %s looks invalid.\n", num);
			SWIG_exit(EXIT_FAILURE);
		}
		Delete(num);
		return updated;
	}

	if (SwigType_type(type) == T_CHAR) {
		/* Use CL syntax for character literals */
		return NewStringf("#\\%s", num_param);
	}
	else if (SwigType_type(type) == T_STRING) {
		/* Use CL syntax for string literals */
		return NewStringf("\"%s\"", num_param);
	}
	
	if (Len(num) < 2 || s[0] != '0') {
		return num;
	}
	
	/* octal or hex */
	
	res=NewStringf("#%c%s", 
		       s[1] == 'x' ? 'x' : 'o', 
		       s+2);
	Delete(num);

	return res;
}

typedef struct type_mapping {
  char *c_type, *lisp_type;
} type_mapping;

/* Need a way for the user to specify where a char is really a character
   or where it is an 8-bit number.  Similarly, is a char * a string pointer
   or a pointer to an array of 8-bit numbers */
type_mapping type_mappings[]={
  { "void" , ":void" },
  { "char", ":char"}, 
  { "signed char", ":char" },
  { "unsigned char", ":unsigned-char" },
  { "short", ":short" },
  { "unsigned short", ":unsigned-short" },
  { "int", ":int" },
  { "unsigned int", ":unsigned-int" },
  { "unsigned long", ":unsigned-int" },
  { "float", ":float" },
  { "double", ":double" },
  { NULL, NULL }
};

struct {
  int count;
  String **entries;
} defined_foreign_types;

void add_defined_foreign_type(String *type) {
  if (!defined_foreign_types.count) {
    /* Make fresh */
    defined_foreign_types.count=1;
    defined_foreign_types.entries=(String **)malloc(sizeof(String *));
  } else {
    /* make room */
    defined_foreign_types.count++;
    defined_foreign_types.entries=(String **)
      realloc(defined_foreign_types.entries,
	      defined_foreign_types.count*sizeof(String *));
  }

  if (!defined_foreign_types.entries) {
    Printf(stderr, "Out of memory\n");
    SWIG_exit(EXIT_FAILURE);
  }

  /* Fill in the new data */
  defined_foreign_types.entries[defined_foreign_types.count-1]=
    Copy(type);
  
}


String *convert_type(SwigType *ty) {
  SwigType *tr=SwigType_typedef_resolve_all(ty);
  char *type_reduced=Char(tr);
  int i;

  //Printf(stdout,"convert_type %s\n", ty);
  if (SwigType_isconst(tr)) {
	  SwigType_pop(tr);
	  type_reduced=Char(tr);
  }


  if (SwigType_ispointer(type_reduced) || SwigType_isarray(ty) ||
      !strncmp(type_reduced, "p.f", 3)) {
#if 1
	  return NewString("(* :void)");
#else
	  return NewString(":foreign-address");
#endif
  }
  
  for(i=0; type_mappings[i].c_type; i++) {
    if (!strcmp(type_reduced, type_mappings[i].c_type)) {
      return NewString(type_mappings[i].lisp_type);
    }
  }
  
  for(i=0; i<defined_foreign_types.count; i++) {
    if (!Strcmp(ty, defined_foreign_types.entries[i])) {
      return NewStringf("#.(%s \"%s\" :type :type)",
			identifier_converter, 
			ty);
    }
  }
  
  if (!Strncmp(type_reduced, "enum ", 5)) {
    return NewString(":int");
  }

  Printf(stderr, "Unsupported data type: %s (was: %s)\n", type_reduced, ty);
  SWIG_exit(EXIT_FAILURE);

  return NewString("this should never happen");

}


void ALLEGROCL :: main(int argc, char *argv[]) {
  int i;

  SWIG_library_directory("allegrocl"); 
  SWIG_config_file("allegrocl.swg");


  for(i=1; i<argc; i++) {
    if (!strcmp(argv[i], "-identifier-converter")) {
      char *conv=argv[i+1];
      
      if (!conv)
	Swig_arg_error();

      Swig_mark_arg(i);
      Swig_mark_arg(i+1);
      i++;

      /* check for built-ins */
      if (!strcmp(conv, "lispify")) {
	identifier_converter="identifier-convert-lispify";
      } else if (!strcmp(conv, "null")) {
	identifier_converter="identifier-convert-null";
      } else {
	/* Must be user defined */
        identifier_converter = (char *)malloc(strlen(conv)+1);
        strcpy(identifier_converter, conv);
      }
    }

    if (!strcmp(argv[i], "-help")) {
      fprintf(stderr, "Allegro CL Options:\n");
      fprintf(stderr, 
	      "    -identifier-converter <type or funcname>\n"
	      "\tSpecifies the type of conversion to do on C identifiers to convert\n"
	      "\tthem to symbols.  There are two built-in converters:  'null' and\n"
	      "\t 'lispify'.  The default is 'null'.  If you supply a name other\n"
	      "\tthan one of the built-ins, then a function by that name will be\n"
	      "\tcalled to convert identifiers to symbols.\n");

    }
      
  }
      
  
}

int ALLEGROCL :: top(Node *n) {
  String *module=Getattr(n, "name");
  String *output_filename=NewString("");
  String *devnull=NewString("/dev/null");

  f_null=NewFile(devnull, "w+");
  if (!f_null) {
	  perror("Failed to open /dev/null");
	  SWIG_exit(EXIT_FAILURE);
  }
  Delete(devnull);


  Printf(output_filename, "%s%s.cl", SWIG_output_directory(), module);


  f_cl=NewFile(output_filename, "w");
  if (!f_cl) {
    Printf(stderr, "Unable to open %s for writing\n", output_filename);
    SWIG_exit(EXIT_FAILURE);
  }

  Swig_register_filebyname("header",f_null);
  Swig_register_filebyname("wrapper", f_cl);

  Printf(f_cl, ";; This is an automatically generated file.  Make changes in\n;; the definition file, not here.\n\n(defpackage :%s\n  (:use :common-lisp :ff :excl))\n\n(in-package :%s)\n", module, module);
  Printf(f_cl, "(eval-when (compile load eval)\n  (defparameter *swig-identifier-converter* '%s))\n", identifier_converter);
  
  Language::top(n);

  Close(f_cl);
  Delete(f_cl); // Delete the handle, not the file
  Close(f_null);
  Delete(f_null);
  
  return SWIG_OK;
}

int ALLEGROCL :: functionWrapper(Node *n) {
  String *funcname=Getattr(n, "sym:name");
  ParmList *pl=Getattr(n, "parms");
  Parm *p;
  int argnum=0, first=1, varargs=0;
  
  //Language::functionWrapper(n);

  Printf(f_cl, "(swig-defun \"%s\"\n", funcname);
  Printf(f_cl, "  (");

  /* Special cases */
  
  if (ParmList_len(pl) == 0) {
    Printf(f_cl, ":void");
  } else if (any_varargs(pl)) {
    Printf(f_cl, "#| varargs |#");
    varargs=1;
  } else {
    for (p=pl; p; p=nextSibling(p), argnum++) {
      String *argname=Getattr(p, "sym:name");
      SwigType *argtype=Getattr(p, "type");
      String *lisptype=convert_type(argtype);
      int tempargname=0;
      
      if (!argname) {
	argname=NewStringf("arg%d", argnum);
	tempargname=1;
      }
      
      if (!first) {
	Printf(f_cl, "\n   ");
      }
      Printf(f_cl, "(%s %s)", argname, lisptype);
      first=0;
      
      Delete(lisptype);
      if (tempargname) 
	Delete(argname);
      
    }
  }
  Printf(f_cl, ")\n"); /* finish arg list */
  Printf(f_cl, "  :returning (%s)\n  :strings-convert t\n  :call-direct %s\n  :optimize-for-space t)\n", 
	 convert_type(Getattr(n, "type")),
	 varargs ? "nil"  : "t");

  
  return SWIG_OK;
}

int ALLEGROCL :: constantWrapper(Node *n) {
  String *type=Getattr(n, "type");
  String *converted_value=convert_literal(Getattr(n, "value"), type);
  String *name=Getattr(n, "sym:name");

#if 0
  Printf(stdout, "constant %s is of type %s. value: %s\n",
	 name, type, converted_value);
#endif

  Printf(f_cl, "(swig-defconstant \"%s\" %s)\n",
	 name, converted_value);

  Delete(converted_value);
 
  return SWIG_OK;
}

// Includes structs
int ALLEGROCL :: classDeclaration(Node *n) {
  String *name=Getattr(n, "sym:name");
  String *kind = Getattr(n,"kind");
  Node *c;
  
  if (Strcmp(kind, "struct")) {
    Printf(stderr, "Don't know how to deal with %s kind of class yet.\n",
	   kind);
    Printf(stderr, " (name: %s)\n", name);
    SWIG_exit(EXIT_FAILURE);
  }

  Printf(f_cl, 
	 "(swig-def-foreign-type \"%s\"\n (:struct\n", 
	 name);
  
  for (c=firstChild(n); c; c=nextSibling(c)) {
    SwigType *type=Getattr(c, "type");
    String *lisp_type;

    if (Strcmp(nodeType(c), "cdecl")) {
      Printf(stderr, "Structure %s has a slot that we can't deal with.\n",
	     name);
      Printf(stderr, "nodeType: %s, name: %s, type: %s\n", 
	     nodeType(c),
	     Getattr(c, "name"),
	     Getattr(c, "type"));
      SWIG_exit(EXIT_FAILURE);
    }

    
    /* Printf(stdout, "Converting %s in %s\n", type, name); */
    lisp_type=convert_type(type);

    Printf(f_cl, 
	   "  (#.(%s \"%s\" :type :slot) %s)\n", 
	   identifier_converter,
	   Getattr(c, "sym:name"), 
	   lisp_type);

    Delete(lisp_type);
  }
  
  Printf(f_cl, " ))\n");

  /* Add this structure to the known lisp types */
  //Printf(stdout, "Adding %s foreign type\n", name);
  add_defined_foreign_type(name);
  
  return SWIG_OK;
}

extern "C" Language *swig_allegrocl(void) {
  return new ALLEGROCL();
}



  
