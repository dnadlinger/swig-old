/* -----------------------------------------------------------------------------
 * cpp.c
 *
 *     An implementation of a C preprocessor plus some support for additional
 *     SWIG directives.
 *
 *         - SWIG directives such as %include, %extern, and %import are handled
 *         - A new macro %define ... %enddef can be used for multiline macros
 *         - No preprocessing is performed in %{ ... %} blocks
 *         - Lines beginning with %# are stripped down to #... and passed through.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 * ----------------------------------------------------------------------------- */

char cvsroot_cpp_c[] = "$Header$";

#include "swig.h"
#include "preprocessor.h"
#include <ctype.h>

static Hash     *cpp = 0;                /* C preprocessor data */
static int       include_all = 0;        /* Follow all includes */
static int       ignore_missing = 0; 
static int       import_all = 0;         /* Follow all includes, but as %import statements */
static int       imported_depth = 0;	 /* Depth of %imported files */
static int       single_include = 1;     /* Only include each file once */
static int       replace_defined = 0;
static Hash     *included_files = 0;
static List     *dependencies = 0;

/* Test a character to see if it starts an identifier */
static int
isidentifier(int c) {
  if ((isalpha(c)) || (c == '_') || (c == '$')) return 1;
  else return 0;
}

/* Test a character to see if it valid in an identifier (after the first letter) */
static int
isidchar(int c) {
  if ((isalnum(c)) || (c == '_') || (c == '$')) return 1;
  else return 0;
}

/* Skip whitespace */
static void
skip_whitespace(File *s, File *out) {
  int c;
  while ((c = Getc(s)) != EOF) {
    if (!isspace(c)) {
      Ungetc(c,s);
      break;
    } else if (out) Putc(c,out);
  }
}

/* Skip to a specified character taking line breaks into account */
static int
skip_tochar(File *s, int ch, File *out) {
  int c;
  while ((c = Getc(s)) != EOF) {
    if (out) Putc(c,out);
    if (c == ch) break;
    if (c == '\\') {
      c = Getc(s);
      if ((c != EOF) && (out)) Putc(c,out);
    }
  }
  if (c == EOF) return -1;
  return 0;
}

static void
copy_location(DOH *s1, DOH *s2) {
  Setfile(s2,Getfile(s1));
  Setline(s2,Getline(s1));
}

static String *cpp_include(String_or_char *fn, int sysfile) {
  String *s;
  s = sysfile ? Swig_include_sys(fn) : Swig_include(fn);
  if (s && single_include) {
    String *file = Getfile(s);
    if (Getattr(included_files,file)) {
      Delete(s);
      return 0;
    }
    Setattr(included_files,file,file);
  }
  if (!s) {
    Seek(fn,0,SEEK_SET);
    if (ignore_missing) {
      Swig_warning(WARN_PP_MISSING_FILE,Getfile(fn),Getline(fn),"Unable to find '%s'\n", fn);
    } else {
      Swig_error(Getfile(fn),Getline(fn),"Unable to find '%s'\n", fn);
    }
  } else {
    Seek(s,0,SEEK_SET);
    if (!dependencies) {
      dependencies = NewList();
    }
    Append(dependencies, Copy(Swig_last_file()));
  }
  return s;
}

List *Preprocessor_depend(void) {
  return dependencies;
}

/* -----------------------------------------------------------------------------
 * void Preprocessor_cpp_init() - Initialize the preprocessor
 * ----------------------------------------------------------------------------- */
void Preprocessor_init(void) {
  Hash *s;
  cpp = NewHash();
  s =   NewHash();
  Setattr(cpp,"symbols",s);
  Preprocessor_expr_init();            /* Initialize the expression evaluator */
  included_files = NewHash();
}
/* -----------------------------------------------------------------------------
 * void Preprocessor_include_all() - Instruct preprocessor to include all files
 * ----------------------------------------------------------------------------- */
void Preprocessor_include_all(int a) {
  include_all = a;
}

void Preprocessor_import_all(int a) {
  import_all = a;
}

void Preprocessor_ignore_missing(int a) {
  ignore_missing = a;
}


/* -----------------------------------------------------------------------------
 * Preprocessor_define()
 *
 * Defines a new C preprocessor symbol.   swigmacro specifies whether or not the macro has
 * SWIG macro semantics.
 * ----------------------------------------------------------------------------- */

 
String_or_char *Macro_vararg_name(String_or_char *str,
                                  String_or_char *line)
{
  String_or_char *argname, *varargname;
  char *s, *dots;

  argname = Copy(str);
  s = Char(argname);
  dots = strchr(s, '.');
  if (!dots) return NULL;
  if (strcmp(dots, "...") != 0) {
    Swig_error(Getfile(line), Getline(line),
               "Illegal macro argument name '%s'\n", str);  
    return NULL;
  }
  if (dots == s) {
      varargname = NewString("__VA_ARGS__");
  } else {
    *dots = '\0';
    varargname = NewStringf(argname);
  }
  Delete(argname);
  return varargname;
}

Hash *Preprocessor_define(const String_or_char *_str, int swigmacro)
{
  String *macroname = 0, *argstr = 0, *macrovalue = 0, *file = 0, *s = 0;
  Hash   *macro = 0, *symbols = 0, *m1;
  List   *arglist = 0;
  int c, line;
  int    varargs = 0;
  String_or_char *str = (String_or_char *) _str;

  assert(cpp);
  assert(str);

  /* First make sure that string is actually a string */
  if (DohCheck(str)) {
    s = NewString(str);
    copy_location(str,s);
    str = s;
  } else {
    str = NewString((char *) str);
  }
  Seek(str,0,SEEK_SET);
  line = Getline(str);
  file = Getfile(str);

  /* Skip over any leading whitespace */
  skip_whitespace(str,0);

  /* Now look for a macro name */
  macroname = NewString("");
  while ((c = Getc(str)) != EOF) {
    if (c == '(') {
      argstr = NewString("");
      copy_location(str,argstr);
      /* It is a macro.  Go extract it's argument string */
      while ((c = Getc(str)) != EOF) {
	if (c == ')') break;
	else Putc(c,argstr);
      }
      if (c != ')') {
	Swig_error(Getfile(str),Getline(str), "Missing \')\' in macro parameters\n");
	goto macro_error;
      }
      break;
    } else if (isidchar(c) || (c == '%')) {
      Putc(c,macroname);
    } else if (isspace(c)) {
      break;
    } else if (c == '\\') {
      c = Getc(str);
      if (c != '\n') {
	Ungetc(c,str);
	Ungetc('\\',str);
	break;
      }
    } else {
      /*Swig_error(Getfile(str),Getline(str),"Illegal character in macro name\n");
	goto macro_error; */
      Ungetc(c,str);
      break;
    }
  }
  if (!swigmacro)
    skip_whitespace(str,0);
  macrovalue = NewString("");
  while ((c = Getc(str)) != EOF) {
    Putc(c,macrovalue);
  }

  /* If there are any macro arguments, convert into a list */
  if (argstr) {
    String *argname, *varargname;
    arglist = NewList();
    Seek(argstr,0,SEEK_SET);
    argname = NewString("");
    while ((c = Getc(argstr)) != EOF) {
      if (c == ',') {
	varargname = Macro_vararg_name(argname, str);
	if (varargname) {
          Swig_error(Getfile(str),Getline(str),"Variable-length macro argument must be last parameter\n");	  
	} else {
	  Append(arglist,argname);
	}
	Delete(argname);
	argname = NewString("");
      } else if (isidchar(c) || (c == '.')) {
	Putc(c,argname);
      } else if (!(isspace(c) || (c == '\\'))) {
	Swig_error(Getfile(str),Getline(str),"Illegal character in macro argument name\n");
	goto macro_error;
      }
    }
    if (Len(argname)) {
      /* Check for varargs */
      varargname = Macro_vararg_name(argname, str);
      if (varargname) {
	Append(arglist,varargname);
	Delete(varargname);
	varargs = 1;
      } else {
	Append(arglist,argname);
      }
      Delete(argname);
    }
  }

  if (!swigmacro) {
    Replace(macrovalue,"\\\n"," ", DOH_REPLACE_NOQUOTE);
  }

  /* Look for special # substitutions.   We only consider # that appears
     outside of quotes and comments */

  {
    int state = 0;
    char *cc = Char(macrovalue);
    while (*cc) {
      switch(state) {
      case 0:
	if (*cc == '#') *cc = '\001';
	else if (*cc == '/') state = 10;
	else if (*cc == '\'') state = 20;
	else if (*cc == '\"') state = 30;
	break;
      case 10:
	if (*cc == '*') state = 11;
	else if (*cc == '/') state = 15;
	else {
	  state = 0;
	  cc--;
	}
	break;
      case 11:
	if (*cc == '*') state = 12;
	break;
      case 12:
	if (*cc == '/') state = 0;
	else if (*cc != '*') state = 11;
	break;
      case 15:
	if (*cc == '\n') state = 0;
	break;
      case 20:
	if (*cc == '\'') state = 0;
	if (*cc == '\\') state = 21;
	break;
      case 21:
	state = 20;
	break;
      case 30:
	if (*cc == '\"') state = 0;
	if (*cc == '\\') state = 31;
	break;
      case 31:
	state = 30;
	break;
      default:
	break;
      }
      cc++;
    }
  }

  /* Get rid of whitespace surrounding # */
  /*  Replace(macrovalue,"#","\001",DOH_REPLACE_NOQUOTE); */
  while(strstr(Char(macrovalue),"\001 ")) {
    Replace(macrovalue,"\001 ","\001", DOH_REPLACE_ANY);
  }
  while(strstr(Char(macrovalue)," \001")) {
    Replace(macrovalue," \001","\001", DOH_REPLACE_ANY);
  }
  /* Replace '##' with a special token */
  Replace(macrovalue,"\001\001","\002", DOH_REPLACE_ANY);
  /* Replace '#@' with a special token */
  Replace(macrovalue,"\001@","\004", DOH_REPLACE_ANY);
  /* Replace '##@' with a special token */
  Replace(macrovalue,"\002@","\005", DOH_REPLACE_ANY);  

  /* Go create the macro */
  macro = NewHash();
  Setattr(macro,"name", macroname);
  if (arglist) {
    Setattr(macro,"args",arglist);
    Delete(arglist);
    if (varargs) {
      Setattr(macro,"varargs","1");
    }
  }
  Setattr(macro,"value",macrovalue);
  Delete(macrovalue);
  Setline(macro,line);
  Setfile(macro,file);
  if (swigmacro) {
    Setattr(macro,"swigmacro","1");
  }
  symbols = Getattr(cpp,"symbols");
  if ((m1 = Getattr(symbols,macroname))) {
    if (Cmp(Getattr(m1,"value"),macrovalue)) {
      Swig_error(Getfile(str),Getline(str),"Macro '%s' redefined,\n",macroname);    
      Swig_error(Getfile(m1),Getline(m1),"previous definition of '%s'.\n",macroname);
      goto macro_error;
    }
  }
  Setattr(symbols,macroname,macro);
  
  Delete(str);
  Delete(argstr);
  Delete(macroname);
  return macro;

 macro_error:
  Delete(str);
  Delete(argstr);
  Delete(macroname);
  return 0;
}

/* -----------------------------------------------------------------------------
 * Preprocessor_undef()
 *
 * Undefines a macro.
 * ----------------------------------------------------------------------------- */
void Preprocessor_undef(const String_or_char *str)
{
  Hash *symbols;
  assert(cpp);
  symbols = Getattr(cpp,"symbols");
  Delattr(symbols,str);
}

/* -----------------------------------------------------------------------------
 * find_args()
 *
 * Isolates macro arguments and returns them in a list.   For each argument,
 * leading and trailing whitespace is stripped (ala K&R, pg. 230).
 * ----------------------------------------------------------------------------- */
static List *
find_args(String *s)
{
  List   *args;
  String *str;
  int   c, level;
  long  pos;

  /* Create a new list */
  args = NewList();
  copy_location(s,args);

  /* First look for a '(' */
  pos = Tell(s);
  skip_whitespace(s,0);

  /* Now see if the next character is a '(' */
  c = Getc(s);
  if (c != '(') {
    /* Not a macro, bail out now! */
    Seek(s,pos, SEEK_SET);
    Delete(args);
    return 0;
  }
  c = Getc(s);
  /* Okay.  This appears to be a macro so we will start isolating arguments */
  while (c != EOF) {
    if (isspace(c)) {
      skip_whitespace(s,0);                    /* Skip leading whitespace */
      c = Getc(s);
    }
    str = NewString("");
    copy_location(s,str);
    level = 0;
    while (c != EOF) {
      if (c == '\"') {
	Putc(c,str);
	skip_tochar(s,'\"',str);
	c = Getc(s);
	continue;
      } else if (c == '\'') {
	Putc(c,str);
	skip_tochar(s,'\'',str);
	c = Getc(s);
	continue;
      }
      if ((c == ',') && (level == 0)) break;
      if ((c == ')') && (level == 0)) break;
      Putc(c,str);
      if (c == '(') level++;
      if (c == ')') level--;
      c = Getc(s);
    }
    if (level > 0) {
      goto unterm;
    }
    Chop(str);
    if (Len(args) || Len(str))
      Append(args,str);
    Delete(str);

    /*    if (Len(str) && (c != ')'))
	  Append(args,str); */

    if (c == ')') return args;
    c = Getc(s);
  }
 unterm:
  Swig_error(Getfile(args),Getline(args),"Unterminated macro call.\n");
  return args;
}

/* -----------------------------------------------------------------------------
 * DOH *get_filename(DOH *str)
 *
 * Read a filename from str.   A filename can be enclose in quotes, angle brackets,
 * or bare.
 * ----------------------------------------------------------------------------- */

static String *
get_filename(String *str, int* sysfile) {
  String *fn;
  int  c;

  skip_whitespace(str,0);
  fn = NewString("");
  copy_location(str,fn);
  c = Getc(str);
  *sysfile = 0;
  if (c == '\"') {
    while (((c = Getc(str)) != EOF) && (c != '\"')) Putc(c,fn);
  } else if (c == '<') {
    *sysfile = 1;
    while (((c = Getc(str)) != EOF) && (c != '>'))  Putc(c,fn);
  } else {
    Putc(c,fn);
    while (((c = Getc(str)) != EOF) && (!isspace(c))) Putc(c,fn);
    if (isspace(c)) Ungetc(c,str);
  }
#if defined(_WIN32) || defined(MACSWIG)
  /* accept Unix path separator on non-Unix systems */
  Replaceall(fn, "/", SWIG_FILE_DELIMETER);
#endif
#if defined(__CYGWIN__)
  /* accept Windows path separator in addition to Unix path separator */
  Replaceall(fn, "\\", SWIG_FILE_DELIMETER);
#endif
  Seek(fn,0,SEEK_SET);
  return fn;
}

static String *
get_options(String *str) {

  int  c;
  skip_whitespace(str,0);
  c = Getc(str);
  if (c == '(') {
    String *opt;
    int     level = 1;
    opt = NewString("(");
    while (((c = Getc(str)) != EOF)) {
      Putc(c,opt);
      if (c == ')') {
	level--;
	if (!level) return opt;
      }
      if (c == '(') level++;
    }
    Delete(opt);
    return 0;
  } else {
    Ungetc(c,str);
    return 0;
  }
}
/* -----------------------------------------------------------------------------
 * expand_macro()
 *
 * Perform macro expansion and return a new string.  Returns NULL if some sort
 * of error occurred.
 * ----------------------------------------------------------------------------- */

static String *
expand_macro(String_or_char *name, List *args)
{
  DOH *symbols, *ns, *macro, *margs, *mvalue, *temp, *tempa, *e;
  DOH *Preprocessor_replace(DOH *);
  int i, l;
  int isvarargs = 0;

  symbols = Getattr(cpp,"symbols");
  if (!symbols) return 0;

  /* See if the name is actually defined */
  macro = Getattr(symbols,name);
  if (!macro) return 0;
  if (Getattr(macro,"*expanded*")) {
    ns = NewString("");
    Printf(ns,"%s",name);
    if (args) {
      if (Len(args))
	Putc('(',ns);
      for (i = 0; i < Len(args); i++) {
	Printf(ns,"%s",Getitem(args,i));
	if (i < (Len(args) -1)) Putc(',',ns);
      }
      if (i)
	Putc(')',ns);
    }
    return ns;
  }

  /* Get macro arguments and value */
  mvalue = Getattr(macro,"value");
  assert(mvalue);
  margs = Getattr(macro,"args");

  if (args && Getattr(macro,"varargs")) {
    isvarargs = 1;
    /* Variable length argument macro.  We need to collect all of the extra arguments into a single argument */
    if (Len(args) >= (Len(margs)-1)) {
      int i;
      int vi, na;
      String *vararg = NewString("");
      vi = Len(margs)-1;
      na = Len(args);
      for (i = vi; i < na; i++) {
	Append(vararg,Getitem(args,i));
	if ((i+1) < na) {
	  Append(vararg,",");
	}
      }
      /* Remove arguments */
      for (i = vi; i < na; i++) {
	Delitem(args,vi);
      }
      Append(args,vararg);
      Delete(vararg);
    }
  }
  /* If there are arguments, see if they match what we were given */
  if (args && (margs) && (Len(margs) != Len(args))) {
    if (Len(margs) > (1+isvarargs))
      Swig_error(Getfile(args),Getline(args),"Macro '%s' expects %d arguments\n", name, Len(margs)-isvarargs);
    else if (Len(margs) == (1+isvarargs))
      Swig_error(Getfile(args),Getline(args),"Macro '%s' expects 1 argument\n", name);
    else
      Swig_error(Getfile(args),Getline(args),"Macro '%s' expects no arguments\n", name);
    return 0;
  }

  /* If the macro expects arguments, but none were supplied, we leave it in place */
  if (!args && (margs)) {
    return NewString(name);
  }

  /* Copy the macro value */
  ns = Copy(mvalue);
  copy_location(mvalue,ns);

  /* Tag the macro as being expanded.   This is to avoid recursion in
     macro expansion */

  temp = NewString("");
  tempa = NewString("");
  if (args && margs) {
    l = Len(margs);
    for (i = 0; i < l; i++) {
      DOH *arg, *aname;
      String *reparg;
      arg = Getitem(args,i);           /* Get an argument value */
      reparg = Preprocessor_replace(arg);
      aname = Getitem(margs,i);        /* Get macro argument name */
      if (strstr(Char(ns),"\001")) {
	/* Try to replace a quoted version of the argument */
	Clear(temp);
	Clear(tempa);
	Printf(temp,"\001%s", aname);
	Printf(tempa,"\"%s\"",arg);
	Replace(ns, temp, tempa, DOH_REPLACE_ID_END);
      }
      if (strstr(Char(ns),"\002")) {
	/* Look for concatenation tokens */
	Clear(temp);
	Clear(tempa);
	Printf(temp,"\002%s",aname);
	Append(tempa,"\002\003");
	Replace(ns, temp, tempa, DOH_REPLACE_ID_END);
	Clear(temp);
	Clear(tempa);
	Printf(temp,"%s\002",aname);
	Append(tempa,"\003\002");
	Replace(ns,temp,tempa, DOH_REPLACE_ID_BEGIN);
      }

      /* Non-standard macro expansion.   The value `x` is replaced by a quoted
	 version of the argument except that if the argument is already quoted
	 nothing happens */

      if (strstr(Char(ns),"`")) {
	String *rep;
	char *c;
	Clear(temp);
	Printf(temp,"`%s`",aname);
	c = Char(arg);
	if (*c == '\"') {
	  rep = arg;
	} else {
	  Clear(tempa);
	  Printf(tempa,"\"%s\"",arg);
	  rep = tempa;
	}
	Replace(ns,temp,rep, DOH_REPLACE_ANY);
      }

      /* Non-standard mangle expansions.  
	 The #@Name is replaced by mangle_arg(Name). */
      if (strstr(Char(ns),"\004")) {
	String* marg = Swig_string_mangle(arg);
	Clear(temp);
	Printf(temp,"\004%s", aname);
	Replace(ns, temp, marg, DOH_REPLACE_ID_END);
	Delete(marg);
      }
      if (strstr(Char(ns),"\005")) {
	String* marg = Swig_string_mangle(arg);
	Clear(temp);
	Clear(tempa);
	Printf(temp,"\005%s", aname);
	Printf(tempa,"\"%s\"", marg);
	Replace(ns, temp, tempa, DOH_REPLACE_ID_END);
	Delete(marg);
      }

      if (isvarargs && i == l-1 && Len(arg) == 0) {
	/* Zero length varargs macro argument.   We search for commas that might appear before and nuke them */
	char *a, *s, *t, *name;
        int namelen;
	s = Char(ns);
        name = Char(aname);
        namelen = Len(aname);
	a = strstr(s,name);
	while (a) {
          if (!isidchar(a[namelen+1])) {
          /* Matched the entire vararg name, not just a prefix */
	    t = a-1;
	    if (*t == '\002') {
	      t--;
	      while (t >= s) {
		if (isspace((int) *t)) t--;
		else if (*t == ',') {
		  *t = ' ';
		} else break;
	      }
	    }
	  }
	  a = strstr(a+namelen,name);
	}
      }
      /*      Replace(ns, aname, arg, DOH_REPLACE_ID); */
      Replace(ns, aname, reparg, DOH_REPLACE_ID);   /* Replace expanded args */
      Replace(ns, "\003", arg, DOH_REPLACE_ANY);    /* Replace unexpanded arg */
      Delete(reparg);
    }
  }
  Replace(ns,"\002","",DOH_REPLACE_ANY);    /* Get rid of concatenation tokens */
  Replace(ns,"\001","#",DOH_REPLACE_ANY);   /* Put # back (non-standard C) */
  Replace(ns,"\004","#@",DOH_REPLACE_ANY);   /* Put # back (non-standard C) */

  /* Expand this macro even further */
  Setattr(macro,"*expanded*","1"); 

  e = Preprocessor_replace(ns);

  Delattr(macro,"*expanded*");
  Delete(ns);

  if (Getattr(macro,"swigmacro")) {
    String *g;
    String *f = NewString("");
    Seek(e,0,SEEK_SET);
    copy_location(macro,e);
    g = Preprocessor_parse(e);

    /* Drop the macro in place, but with a marker around it */
    Printf(f,"/*@%s,%d,%s@*/%s/*@@*/", Getfile(macro), Getline(macro), name, g);

    /*    Printf(f," }\n"); */
    Delete(g);
    Delete(e);
    e = f;
  }
  Delete(temp);
  Delete(tempa);
  return e;
}

/* -----------------------------------------------------------------------------
 * evaluate_args()
 *
 * Evaluate the arguments of a macro 
 * ----------------------------------------------------------------------------- */

List *evaluate_args(List *x) {
  Iterator i;
  String *Preprocessor_replace(String *);
  List *nl = NewList();

  for (i = First(x); i.item; i = Next(i)) {
    Append(nl,Preprocessor_replace(i.item));
  }
  return nl;
}

/* -----------------------------------------------------------------------------
 * DOH *Preprocessor_replace(DOH *s)
 *
 * Performs a macro substitution on a string s.  Returns a new string with
 * substitutions applied.   This function works by walking down s and looking
 * for identifiers.   When found, a check is made to see if they are macros
 * which are then expanded.
 * ----------------------------------------------------------------------------- */

DOH *
Preprocessor_replace(DOH *s)
{
  DOH   *ns, *id, *symbols, *m;
  int   c, i, state = 0;

  assert(cpp);
  symbols = Getattr(cpp,"symbols");

  ns = NewString("");
  copy_location(s,ns);
  Seek(s,0,SEEK_SET);
  id = NewString("");

  /* Try to locate identifiers in s and replace them with macro replacements */
  while ((c = Getc(s)) != EOF) {
    switch (state) {
    case 0:
      if (isidentifier(c) || (c == '%')) {
	Clear(id);
	copy_location(s,id);
	Putc(c,id);
	state = 1;
      } else if (c == '\"') {
	Putc(c,ns);
	skip_tochar(s,'\"',ns);
      } else if (c == '\'') {
	Putc(c,ns);
	skip_tochar(s,'\'',ns);
      } else if (c == '/') {
	Putc(c,ns);
	state = 10;
      } else {
	Putc(c,ns);
      }
      break;
    case 1:  /* An identifier */
      if (isidchar(c)) {
	Putc(c,id);
	state = 1;
      } else {
	/* We found the end of a valid identifier */
	Ungetc(c,s);
	/* See if this is the special "defined" macro */
	if (replace_defined && Cmp(id,"defined") == 0) {
	  DOH *args;
	  /* See whether or not a paranthesis has been used */
	  skip_whitespace(s,0);
	  c = Getc(s);
	  if (c == '(') {
	    Seek(s,-1,SEEK_CUR);
	    args = find_args(s);
	  } else {
	    DOH *arg = 0;
	    args = NewList();
	    arg = NewString("");
	    if (isidchar(c)) Putc(c,arg);
	    while ((c = Getc(s)) != EOF) {
	      if (!isidchar(c)) {
		Seek(s,-1,SEEK_CUR);
		break;
	      }
	      Putc(c,arg);
	    }
	    Append(args,arg);
	    Delete(arg);
	  }
	  if ((!args) || (!Len(args))) {
	    Swig_error(Getfile(id),Getline(id),"No arguments given to defined()\n");
	    state = 0;
	    break;
	  }
	  for (i = 0; i < Len(args); i++) {
	    DOH *o = Getitem(args,i);
	    if (!Getattr(symbols,o)) {
	      break;
	    }
	  }
	  if (i < Len(args)) Putc('0',ns);
	  else Putc('1',ns);
	  Delete(args);
	  state = 0;
	  break;
	}
	if (Cmp(id,"__LINE__") == 0) {
	  Printf(ns,"%d",Getline(s));
	  state = 0;
	  break;
	}
	if (Cmp(id,"__FILE__") == 0) {
	  String *fn = Copy(Getfile(s));
	  Replaceall(fn,"\\","\\\\");
	  Printf(ns,"\"%s\"",fn);
	  Delete(fn);
	  state = 0;
	  break;
	}
	/* See if the macro is defined in the preprocessor symbol table */
	if ((m = Getattr(symbols,id))) {
	  DOH *args = 0;
	  DOH *e;
	  /* See if the macro expects arguments */
	  if (Getattr(m,"args")) {
	    /* Yep.  We need to go find the arguments and do a substitution */
	    args = find_args(s);
	    if (!Len(args)) {
	      Delete(args);
	      args = 0;
	    }
	  } else {
	    args = 0;
	  }
	  e = expand_macro(id,args);
	  if (e) {
	    Printf(ns,"%s",e);
	  }
	  Delete(e);
	  Delete(args);
	} else {
	  Printf(ns,"%s",id);
	}
	state = 0;
      }
      break;
    case 10:
      if (c == '/') state = 11;
      else if (c == '*') state = 12;
      else {
	Ungetc(c,s);
	state = 0;
	break;
      }
      Putc(c,ns);
      break;
    case 11:
      Putc(c,ns);
      if (c == '\n') state = 0;
      break;
    case 12:
      Putc(c,ns);
      if (c == '*') state = 13;
      break;
    case 13:
      Putc(c,ns);
      if (c == '/') state = 0;
      else if (c != '*') state = 12;
      break;
    default:
      state = 0;
      break;
    }
  }

  /* Identifier at the end */
  if (state == 1) {
    /* See if this is the special "defined" macro */
    if (Cmp(id,"defined") == 0) {
      Swig_error(Getfile(id),Getline(id),"No arguments given to defined()\n");
    } else if ((m = Getattr(symbols,id))) {
	DOH *e;
	/* Yes.  There is a macro here */
	/* See if the macro expects arguments */
	/*	if (Getattr(m,"args")) {
	  Swig_error(Getfile(id),Getline(id),"Macro arguments expected.\n");
	  } */
	e = expand_macro(id,0);
	Printf(ns,"%s",e);
	Delete(e);
    } else {
      Printf(ns,"%s",id);
    }
  }
  Delete(id);
  return ns;
}


/* -----------------------------------------------------------------------------
 * int check_id(DOH *s)
 *
 * Checks the string s to see if it contains any unresolved identifiers.  This
 * function contains the heuristic that determines whether or not a macro
 * definition passes through the preprocessor as a constant declaration.
 * ----------------------------------------------------------------------------- */
static int
check_id(DOH *s)
{
  static SwigScanner *scan = 0;
  int c;
  int hastok = 0;

  Seek(s,0,SEEK_SET);

  if (!scan) {
    scan = NewSwigScanner();
  }

  SwigScanner_clear(scan);
  s = Copy(s);
  Seek(s,SEEK_SET,0);
  SwigScanner_push(scan,s);
  while ((c = SwigScanner_token(scan))) {
    hastok = 1;
    if ((c == SWIG_TOKEN_ID) || (c == SWIG_TOKEN_LBRACE) || (c == SWIG_TOKEN_RBRACE)) return 1;
  }
  if (!hastok) return 1;
  return 0;
}

/* addline().  Utility function for adding lines to a chunk */
static void
addline(DOH *s1, DOH *s2, int allow)
{
  if (allow) {
    Append(s1,s2);
  } else {
    char *c = Char(s2);
    while (*c) {
      if (*c == '\n') Putc('\n',s1);
      c++;
    }
  }
}

static void add_chunk(DOH *ns, DOH *chunk, int allow) {
  DOH *echunk;
  Seek(chunk,0,SEEK_SET);
  if (allow) {
    echunk = Preprocessor_replace(chunk);
    addline(ns,echunk,allow);
    Delete(echunk);
  } else {
    addline(ns,chunk,0);
  }
  Clear(chunk);
}

/*
  push/pop_imported(): helper functions for defining and undefining
  SWIGIMPORTED (when %importing a file).
 */
static void
push_imported() {
  if (imported_depth == 0) {
    Preprocessor_define("SWIGIMPORTED 1", 0);
  }
  ++imported_depth;
}

static void
pop_imported() {
  --imported_depth;
  if (imported_depth == 0) {
    Preprocessor_undef("SWIGIMPORTED");
  }
}

/* -----------------------------------------------------------------------------
 * Preprocessor_parse()
 *
 * Parses the string s.  Returns a new string containing the preprocessed version.
 *
 * Parsing rules :
 *       1.  Lines starting with # are C preprocessor directives
 *       2.  Macro expansion inside strings is not allowed
 *       3.  All code inside false conditionals is changed to blank lines
 *       4.  Code in %{, %} is not parsed because it may need to be
 *           included inline (with all preprocessor directives included).
 * ----------------------------------------------------------------------------- */

String *
Preprocessor_parse(String *s)
{
  String  *ns;             /* New string containing the preprocessed text */
  String  *chunk, *sval, *decl;
  Hash    *symbols;
  String  *id = 0, *value = 0, *comment = 0;
  int    i, state, val, e, c;
  int    start_line = 0;
  int    allow = 1;
  int    level = 0;
  int    mask = 0;
  int    start_level = 0;
  int    cpp_lines = 0;
  int    cond_lines[256];

  /* Blow away all carriage returns */
  Replace(s,"\015","",DOH_REPLACE_ANY); 

  ns = NewString("");        /* Return result */

  decl = NewString("");
  id = NewString("");
  value = NewString("");
  comment = NewString("");
  chunk = NewString("");
  copy_location(s,chunk);
  copy_location(s,ns);
  symbols = Getattr(cpp,"symbols");

  state = 0;
  while ((c = Getc(s)) != EOF) {
    switch(state) {
    case 0:        /* Initial state - in first column */
      /* Look for C preprocessor directives.   Otherwise, go directly to state 1 */
      if (c == '#') {
	replace_defined = 1;
	add_chunk(ns,chunk,allow);
	replace_defined = 0;
	copy_location(s,chunk);
	cpp_lines = 1;
	state = 40;
      } else if (isspace(c)) {
	Putc(c,chunk);
	skip_whitespace(s,chunk);
      } else {
	state = 1;
	Ungetc(c,s);
      }
      break;
    case 1:       /* Non-preprocessor directive */
      /* Look for SWIG directives */
      if (c == '%') {
	state = 100;
	break;
      }
      Putc(c,chunk);
      if (c == '\n') state = 0;
      else if (c == '\"') {
	start_line = Getline(s);
	if (skip_tochar(s,'\"',chunk) < 0) {
	  Swig_error(Getfile(s),-1,"Unterminated string constant starting at line %d\n",start_line);
	}
      } else if (c == '\'') {
	start_line = Getline(s);
	if (skip_tochar(s,'\'',chunk) < 0) {
	  Swig_error(Getfile(s),-1,"Unterminated character constant starting at line %d\n",start_line);
	}
      }
      else if (c == '/') state = 30;  /* Comment */
      break;

    case 30:      /* Possibly a comment string of some sort */
      start_line = Getline(s);
      Putc(c,chunk);
      if (c == '/') state = 31;
      else if (c == '*') state = 32;
      else state = 1;
      break;
    case 31:
      Putc(c,chunk);
      if (c == '\n') state = 0;
      break;
    case 32:
      Putc(c,chunk);
      if (c == '*') state = 33;
      break;
    case 33:
      Putc(c,chunk);
      if (c == '/') state = 1;
      else if (c != '*') state = 32;
      break;

    case 40:   /* Start of a C preprocessor directive */
      if (c == '\n') {
	Putc('\n',chunk);
	state = 0;
      } else if (isspace(c)) {
	state = 40;
      } else {
	/* Got the start of a preprocessor directive */
	Ungetc(c,s);
	Clear(id);
	copy_location(s,id);
	state = 41;
      }
      break;

    case 41:  /* Build up the name of the preprocessor directive */
      if ((isspace(c) || (!isalpha(c)))) {
	Clear(value);
	Clear(comment);
	if (c == '\n') {
	  Ungetc(c,s);
	  state = 50;
	} else {
	  state = 42;
	  if (!isspace(c)) {
	    Ungetc(c,s);
	  }
	}

	copy_location(s,value);
	break;
      }
      Putc(c,id);
      break;

    case 42: /* Strip any leading space before preprocessor value */
      if (isspace(c)) {
	if (c == '\n') {
	  Ungetc(c,s);
	  state = 50;
	}
	break;
      }
      state = 43;
      /* no break intended here */

    case 43:
      /* Get preprocessor value */
      if (c == '\n') {
	Ungetc(c,s);
	state = 50;
      } else if (c == '/') {
	state = 45;
      } else if (c == '\"') {
	Putc(c,value);
	skip_tochar(s,'\"',value);
      } else if (c == '\'') {
	Putc(c,value);
	skip_tochar(s,'\'',value);
      } else {
	Putc(c,value);
	if (c == '\\') state = 44;
      }
      break;

    case 44:
      if (c == '\n') {
	Putc(c,value);
	cpp_lines++;
      } else {
	Ungetc(c,s);
      }
      state = 43;
      break;

      /* States 45-48 are used to remove, but retain comments from macro values.  The comments
         will be placed in the output in an alternative form */

    case 45:
      if (c == '/') state = 46;
      else if (c == '*') state = 47;
      else if (c == '\n') {
	Putc('/',value);
	Ungetc(c,s);
	cpp_lines++;
	state = 50;
      } else {
	Putc('/',value);
	Putc(c,value);
	state = 43;
      }
      break;
    case 46:
      if (c == '\n') {
	Ungetc(c,s);
	cpp_lines++;
	state = 50;
      } else Putc(c,comment);
      break;
    case 47:
      if (c == '*') state = 48;
      else Putc(c,comment);
      break;
    case 48:
      if (c == '/') state = 43;
      else if (c == '*') Putc(c,comment);
      else {
	Putc('*',comment);
	Putc(c,comment);
	state = 47;
      }
      break;
    case 50:
      /* Check for various preprocessor directives */
      Chop(value);
      if (Cmp(id,"define") == 0) {
	if (allow) {
	  DOH *m, *v, *v1;
	  Seek(value,0,SEEK_SET);
	  m = Preprocessor_define(value,0);
	  if ((m) && !(Getattr(m,"args"))) {
	    v = Copy(Getattr(m,"value"));
	    if (Len(v)) {
	      Swig_error_silent(1);
	      v1 = Preprocessor_replace(v);
	      Swig_error_silent(0);
	      /*	      Printf(stdout,"checking '%s'\n", v1); */
	      if (!check_id(v1)) {
		if (Len(comment) == 0)
		  Printf(ns,"%%constant %s = %s;\n", Getattr(m,"name"), v1);
		else
		  Printf(ns,"%%constant %s = %s; /*%s*/\n", Getattr(m,"name"),v1,comment);
		cpp_lines--;
	      }
	      Delete(v1);
	    }
	    Delete(v);
	  }
	  Delete(m);
	}
      } else if (Cmp(id,"undef") == 0) {
	if (allow) Preprocessor_undef(value);
      } else if (Cmp(id,"ifdef") == 0) {
	cond_lines[level] = Getline(id);
	level++;
	if (allow) {
	  start_level = level;
	  /* See if the identifier is in the hash table */
	  if (!Getattr(symbols,value)) allow = 0;
	  mask = 1;
	}
      } else if (Cmp(id,"ifndef") == 0) {
	cond_lines[level] = Getline(id);
	level++;
	if (allow) {
	  start_level = level;
	  /* See if the identifier is in the hash table */
	  if (Getattr(symbols,value)) allow = 0;
	  mask = 1;
	}
      } else if (Cmp(id,"else") == 0) {
	if (level <= 0) {
	  Swig_error(Getfile(s),Getline(id),"Misplaced #else.\n");
	} else {
	  cond_lines[level-1] = Getline(id);
	  if (allow) {
	    allow = 0;
	    mask = 0;
	  } else if (level == start_level) {
	    allow = 1*mask;
	  }
	}
      } else if (Cmp(id,"endif") == 0) {
	level--;
	if (level < 0) {
	  Swig_error(Getfile(id),Getline(id),"Extraneous #endif.\n");
	  level = 0;
	} else {
	  if (level < start_level) {
	    allow = 1;
	    start_level--;
	  }
	}
      } else if (Cmp(id,"if") == 0) {
	cond_lines[level] = Getline(id);
	level++;
	if (allow) {
	  start_level = level;
	  sval = Preprocessor_replace(value);
	  Seek(sval,0,SEEK_SET);
	  /*	  Printf(stdout,"Evaluating '%s'\n", sval); */
  	  val = Preprocessor_expr(sval,&e);
  	  if (e) {
	    char * msg = Preprocessor_expr_error();
  	    Seek(value,0,SEEK_SET);
	    Swig_warning(WARN_PP_EVALUATION,Getfile(value),Getline(value),"Could not evaluate '%s'\n", value);
	    if (msg)
	      Swig_warning(WARN_PP_EVALUATION,Getfile(value),Getline(value),"Error: '%s'\n", msg);
  	    allow = 0;
  	  } else {
  	    if (val == 0)
  	      allow = 0;
  	  }
  	  mask = 1;
  	}
      } else if (Cmp(id,"elif") == 0) {
  	if (level == 0) {
  	  Swig_error(Getfile(s),Getline(id),"Misplaced #elif.\n");
  	} else {
  	  cond_lines[level-1] = Getline(id);
  	  if (allow) {
  	    allow = 0;
  	    mask = 0;
  	  } else if (level == start_level) {
  	    sval = Preprocessor_replace(value);
  	    Seek(sval,0,SEEK_SET);
  	    val = Preprocessor_expr(sval,&e);
  	    if (e) {
	      char * msg = Preprocessor_expr_error();
  	      Seek(value,0,SEEK_SET);
  	      Swig_warning(WARN_PP_EVALUATION,Getfile(value),Getline(value),"Could not evaluate '%s'\n", value);
	      if (msg)
		Swig_warning(WARN_PP_EVALUATION,Getfile(value),Getline(value),"Error: '%s'\n", msg);
  	      allow = 0;
  	    } else {
  	      if (val)
  		allow = 1*mask;
  	      else
  		allow = 0;
  	    }
  	  }
  	}
      } else if (Cmp(id,"line") == 0) {
      } else if (Cmp(id,"include") == 0) {
  	if (((include_all) || (import_all)) && (allow)) {
  	  String *s1, *s2, *fn;
	  char *dirname; int sysfile = 0;
	  if (include_all && import_all) {
	    Swig_warning(WARN_PP_INCLUDEALL_IMPORTALL,Getfile(s),Getline(id),"Both includeall and importall are defined: using includeall");
	    import_all = 0;
	  }
  	  Seek(value,0,SEEK_SET);
  	  fn = get_filename(value, &sysfile);
	  s1 = cpp_include(fn, sysfile);
	  if (s1) {
	    if (include_all) 
	      Printf(ns,"%%includefile \"%s\" [\n", Swig_last_file());
	    else if (import_all) {
	      Printf(ns,"%%importfile \"%s\" [\n", Swig_last_file());
	      push_imported();
	    }

	    /* See if the filename has a directory component */
	    dirname = Swig_file_dirname(Swig_last_file());
	    if (sysfile || !strlen(dirname)) dirname = 0;
	    if (dirname) {
	      dirname[strlen(dirname)-1] = 0;   /* Kill trailing directory delimeter */
	      Swig_push_directory(dirname);
	    }
  	    s2 = Preprocessor_parse(s1);
  	    addline(ns,s2,allow);
  	    Printf(ns,"\n]");
	    if (dirname) {
	      Swig_pop_directory();
	    }
	    if (import_all) {
	      pop_imported();
	    }
	    Delete(s2);
  	  }
	  Delete(s1);
 	  Delete(fn);
  	}
      } else if (Cmp(id,"pragma") == 0) {
	if (Strncmp(value,"SWIG ",5) == 0) {
	  char *c = Char(value)+5;
	  while (*c && (isspace((int)*c))) c++;
	  if (*c) {
	    if (Strncmp(c,"nowarn=",7) == 0) {
	      Swig_warnfilter(c+7,1);
	    }
	  }
	}
      } else if (Cmp(id,"level") == 0) {
	Swig_error(Getfile(s),Getline(id),"cpp debug: level = %d, startlevel = %d\n", level, start_level);
      }
      for (i = 0; i < cpp_lines; i++)
  	Putc('\n',ns);
      state = 0;
      break;

        /* Swig directives  */
    case 100:
      /* %{,%} block  */
      if (c == '{') {
  	start_line = Getline(s);
  	add_chunk(ns,chunk,allow);
  	copy_location(s,chunk);
  	Putc('%',chunk);
  	Putc(c,chunk);
  	state = 105;
      }
      /* %#cpp -  an embedded C preprocessor directive (we strip off the %)  */
      else if (c == '#') {
	replace_defined = 1;
	add_chunk(ns,chunk,allow);
	replace_defined = 0;
  	Putc(c,chunk);
  	state = 107;
      } else if (isidentifier(c)) {
  	Clear(decl);
  	Putc('%',decl);
  	Putc(c,decl);
  	state = 110;
      } else {
	Putc('%',chunk);
  	Putc(c,chunk);
  	state = 1;
      }
      break;

    case 105:
      Putc(c,chunk);
      if (c == '%')
  	state = 106;
      break;

    case 106:
      Putc(c,chunk);
      if (c == '}') {
  	state = 1;
	addline(ns,chunk,allow);
	Clear(chunk);
	copy_location(s,chunk);
      } else {
  	state = 105;
      }
      break;
      
    case 107:
      Putc(c,chunk);
      if (c == '\n') {
	addline(ns,chunk,allow);
	Clear(chunk);
	state = 0;
      } else if (c == '\\') {
	state = 108;
      }
      break;

    case 108:
      Putc(c,chunk);
      state = 107;
      break;

    case 110:
      if (!isidchar(c)) {
  	Ungetc(c,s);
  	/* Look for common Swig directives  */
  	if ((Cmp(decl,"%include") == 0) || (Cmp(decl,"%import") == 0) || (Cmp(decl,"%extern") == 0)) {
  	  /* Got some kind of file inclusion directive  */
  	  if (allow) {
  	    DOH *s1, *s2, *fn, *opt; int sysfile = 0;

	    if (Cmp(decl,"%extern") == 0) {
	      Swig_warning(WARN_DEPRECATED_EXTERN, Getfile(s),Getline(s),"%%extern is deprecated. Use %%import instead.\n");
	      Clear(decl);
	      Printf(decl,"%%import");
	    }
	    opt = get_options(s);
  	    fn = get_filename(s, &sysfile);
	    s1 = cpp_include(fn, sysfile);
	    if (s1) {
	      char *dirname;
  	      add_chunk(ns,chunk,allow);
  	      copy_location(s,chunk);
  	      Printf(ns,"%sfile%s \"%s\" [\n", decl, opt, Swig_last_file());
	      if (Cmp(decl,"%import") == 0) {
		push_imported();
	      }
	      dirname = Swig_file_dirname(Swig_last_file());
	      if (sysfile || !strlen(dirname)) dirname = 0;
	      if (dirname) {
		dirname[strlen(dirname)-1] = 0;   /* Kill trailing directory delimeter */
		Swig_push_directory(dirname);
	      }
  	      s2 = Preprocessor_parse(s1);
	      if (dirname) {
		Swig_pop_directory();
	      }
	      if (Cmp(decl,"%import") == 0) {
		pop_imported();
	      }
  	      addline(ns,s2,allow);
  	      Printf(ns,"\n]");
	      Delete(s2);
	      Delete(s1);
  	    }
	    Delete(fn);
  	  }
  	  state = 1;
  	} else if (Cmp(decl,"%line") == 0) {
  	  /* Got a line directive  */
  	  state = 1;
  	} else if (Cmp(decl,"%define") == 0) {
  	  /* Got a define directive  */
  	  add_chunk(ns,chunk,allow);
  	  copy_location(s,chunk);
  	  Clear(value);
  	  copy_location(s,value);
  	  state = 150;
  	} else {
  	  Printf(chunk,"%s", decl);
  	  state = 1;
  	}
      } else {
  	Putc(c,decl);
      }
      break;

        /* Searching for the end of a %define statement  */
    case 150:
      Putc(c,value);
      if (c == '%') {
  	int i = 0;
  	char *d = "enddef";
  	for (i = 0; i < 6; i++) {
  	  c = Getc(s);
  	  Putc(c,value);
  	  if (c != d[i]) break;
  	}
	c = Getc(s);
	Ungetc(c,s);
  	if ((i == 6) && (isspace(c))) {
  	  /* Got the macro  */
  	  for (i = 0; i < 7; i++) {
  	    Delitem(value,DOH_END);
  	  }
  	  if (allow) {
  	    Seek(value,0,SEEK_SET);
  	    Preprocessor_define(value,1);
  	  }
  	  Putc('\n',ns);
  	  addline(ns,value,0);
  	  state = 0;
  	}
      }
      break;
    default :
      Printf(stderr,"cpp: Invalid parser state %d\n", state);
      abort();
      break;
    }
  }
  while (level > 0) {
    Swig_error(Getfile(s),-1,"Missing #endif for conditional starting on line %d\n", cond_lines[level-1]);
    level--;
  }
  if (state == 150) {
    Seek(value,0,SEEK_SET);
    Swig_error(Getfile(s),-1,"Missing %%enddef for macro starting on line %d\n",Getline(value));
  }
  if ((state >= 105) && (state < 107)) {
    Swig_error(Getfile(s),-1,"Unterminated %%{ ... %%} block starting on line %d\n", start_line);
  }
  if ((state >= 30) && (state < 40)) {
    Swig_error(Getfile(s),-1,"Unterminated comment starting on line %d\n", start_line);
  }
  add_chunk(ns,chunk,allow);
  copy_location(s,chunk);

  /*  DelScope(scp); */
  Delete(decl);
  Delete(id);
  Delete(value);
  Delete(comment);
  Delete(chunk);

  /*  fprintf(stderr,"cpp: %d\n", Len(Getattr(cpp,"symbols"))); */
  return ns;
}




