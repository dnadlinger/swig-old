/* ----------------------------------------------------------------------------- 
 * overload.cxx
 *
 *     This file is used to analyze overloaded functions and methods.
 *     It looks at signatures and tries to gather information for
 *     building a dispatch function.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

char cvsroot_overload_cxx[] = "$Header$";

#include "swigmod.h"

#define MAX_OVERLOAD 256

/* Overload "argc" and "argv" */
String *argv_template_string;
String *argc_template_string;

struct Overloaded {
  Node      *n;          /* Node                               */
  int        argc;       /* Argument count                     */
  ParmList  *parms;      /* Parameters used for overload check */
  int        error;      /* Ambiguity error                    */
};

/* -----------------------------------------------------------------------------
 * Swig_overload_rank()
 *
 * This function takes an overloaded declaration and creates a list that ranks
 * all overloaded methods in an order that can be used to generate a dispatch 
 * function.
 * Slight difference in the way this function is used by scripting languages and
 * statically typed languages. The script languages call this method via 
 * Swig_overload_dispatch() - where wrappers for all overloaded methods are generated,
 * however sometimes the code can never be executed. The non-scripting languages
 * call this method via Swig_overload_check() for each overloaded method in order
 * to determine whether or not the method should be wrapped. Note the slight
 * difference when overloading methods that differ by const only. The
 * scripting languages will ignore the const method, whereas the non-scripting
 * languages ignore the first method parsed.
 * ----------------------------------------------------------------------------- */

static List *
Swig_overload_rank(Node *n, bool script_lang_wrapping) {
  Overloaded  nodes[MAX_OVERLOAD];
  int         nnodes = 0;
  Node *o = Getattr(n,"sym:overloaded");
  Node *c;

  if (!o) return 0;

  c = o;
  while (c) {
    if (Getattr(c,"error")) {
      c = Getattr(c,"sym:nextSibling");
      continue;
    }
    /*    if (SmartPointer && Getattr(c,"cplus:staticbase")) {
      c = Getattr(c,"sym:nextSibling");
      continue;
      } */

    /* Make a list of all the declarations (methods) that are overloaded with
     * this one particular method name */
    if (Getattr(c,"wrap:name")) {
      nodes[nnodes].n = c;
      nodes[nnodes].parms = Getattr(c,"wrap:parms");
      nodes[nnodes].argc = emit_num_required(nodes[nnodes].parms);
      nodes[nnodes].error = 0;
      nnodes++;
    }
    c = Getattr(c,"sym:nextSibling");
  }
  
  /* Sort the declarations by required argument count */
  {
    int i,j;
    for (i = 0; i < nnodes; i++) {
      for (j = i+1; j < nnodes; j++) {
	if (nodes[i].argc > nodes[j].argc) {
	  Overloaded t = nodes[i];
	  nodes[i] = nodes[j];
	  nodes[j] = t;
	}
      }
    }
  }

  /* Sort the declarations by argument types */
  {
    int i,j;
    for (i = 0; i < nnodes-1; i++) {
      if (nodes[i].argc == nodes[i+1].argc) {
	for (j = i+1; (j < nnodes) && (nodes[j].argc == nodes[i].argc); j++) {
	  Parm *p1 = nodes[i].parms;
	  Parm *p2 = nodes[j].parms;
	  int   differ = 0;
	  int   num_checked = 0;
	  while (p1 && p2 && (num_checked < nodes[i].argc)) {
	    //	  Printf(stdout,"p1 = '%s', p2 = '%s'\n", Getattr(p1,"type"), Getattr(p2,"type"));
	    if (checkAttribute(p1,"tmap:in:numinputs","0")) {
	      p1 = Getattr(p1,"tmap:in:next");
	      continue;
	    }
	    if (checkAttribute(p2,"tmap:in:numinputs","0")) {
	      p2 = Getattr(p2,"tmap:in:next");
	      continue;
	    }
	    String *t1 = Getattr(p1,"tmap:typecheck:precedence");
	    String *t2 = Getattr(p2,"tmap:typecheck:precedence");
	    if ((!t1) && (!nodes[i].error)) {
	      Swig_warning(WARN_TYPEMAP_TYPECHECK, Getfile(nodes[i].n), Getline(nodes[i].n),
			   "Overloaded %s(%s) not supported (no type checking rule for '%s').\n", 
			   Getattr(nodes[i].n,"name"),ParmList_str_defaultargs(Getattr(nodes[i].n,"parms")),
			   SwigType_str(Getattr(p1,"type"),0));
	      nodes[i].error = 1;
	    } else if ((!t2) && (!nodes[j].error)) {
	      Swig_warning(WARN_TYPEMAP_TYPECHECK, Getfile(nodes[j].n), Getline(nodes[j].n),
			   "Overloaded %s(%s) not supported (no type checking rule for '%s').\n", 
			   Getattr(nodes[j].n,"name"),ParmList_str_defaultargs(Getattr(nodes[j].n,"parms")),
			   SwigType_str(Getattr(p2,"type"),0));
	      nodes[j].error = 1;
	    }
	    if (t1 && t2) {
	      int t1v, t2v;
	      t1v = atoi(Char(t1));
	      t2v = atoi(Char(t2));
	      differ = t1v-t2v;
	    }
	    else if (!t1 && t2) differ = 1;
	    else if (t2 && !t1) differ = -1;
	    else if (!t1 && !t2) differ = -1;
	    num_checked++;
	    if (differ > 0) {
	      Overloaded t = nodes[i];
	      nodes[i] = nodes[j];
	      nodes[j] = t;
	      break;
	    } else if ((differ == 0) && (Strcmp(t1,"0") == 0)) {
	      t1 = Getattr(p1,"ltype");
	      if (!t1) {
		t1 = SwigType_ltype(Getattr(p1,"type"));
		if (Getattr(p1,"tmap:typecheck:SWIGTYPE")) {
		  SwigType_add_pointer(t1);
		}
		Setattr(p1,"ltype",t1);
	      }
	      t2 = Getattr(p2,"ltype");
	      if (!t2) {
		t2 = SwigType_ltype(Getattr(p2,"type"));
		if (Getattr(p2,"tmap:typecheck:SWIGTYPE")) {
		  SwigType_add_pointer(t2);
		}
		Setattr(p2,"ltype",t2);
	      }

	      /* Need subtype check here.  If t2 is a subtype of t1, then we need to change the
                 order */

	      if (SwigType_issubtype(t2,t1)) {
		Overloaded t = nodes[i];
		nodes[i] = nodes[j];
		nodes[j] = t;
	      }

	      if (Strcmp(t1,t2) != 0) {
		differ = 1;
		break;
	      }
	    } else if (differ) {
	      break;
	    }
	    if (Getattr(p1,"tmap:in:next")) {
	      p1 = Getattr(p1,"tmap:in:next");
	    } else {
	      p1 = nextSibling(p1);
	    }
	    if (Getattr(p2,"tmap:in:next")) {
	      p2 = Getattr(p2,"tmap:in:next");
	    } else {
	      p2 = nextSibling(p2);
	    }
	  }
	  if (!differ) {
	    /* See if declarations differ by const only */
	    String *d1 = Getattr(nodes[i].n,"decl");
	    String *d2 = Getattr(nodes[j].n,"decl");
	    if (d1 && d2) {
	      String *dq1 = Copy(d1);
	      String *dq2 = Copy(d2);
	      if (SwigType_isconst(d1)) {
		SwigType_pop(dq1);
	      }
	      if (SwigType_isconst(d2)) {
		SwigType_pop(dq2);
	      }
	      if (Strcmp(dq1,dq2) == 0) {
		
		if (SwigType_isconst(d1) && !SwigType_isconst(d2)) {
                  if (script_lang_wrapping) {
                    // Swap nodes so that the const method gets ignored (shadowed by the non-const method)
                    Overloaded t = nodes[i];
                    nodes[i] = nodes[j];
                    nodes[j] = t;
                  }
		  differ = 1;
		  if (!nodes[j].error) {
                    if (script_lang_wrapping) {
		      Swig_warning(WARN_LANG_OVERLOAD_CONST, Getfile(nodes[j].n), Getline(nodes[j].n),
				   "Overloaded %s(%s) const ignored. Non-const method at %s:%d used.\n",
				   Getattr(nodes[j].n,"name"), ParmList_protostr(nodes[j].parms),
				   Getfile(nodes[i].n), Getline(nodes[i].n));
                    } else {
                      if (!Getattr(nodes[j].n, "overload:ignore"))
		        Swig_warning(WARN_LANG_OVERLOAD_IGNORED, Getfile(nodes[j].n), Getline(nodes[j].n),
				     "Overloaded method %s(%s) ignored. Method %s(%s) const at %s:%d used.\n",
				     Getattr(nodes[j].n,"name"), ParmList_protostr(nodes[j].parms),
			             Getattr(nodes[i].n,"name"), ParmList_protostr(nodes[i].parms),
				     Getfile(nodes[i].n), Getline(nodes[i].n));
                    }
		  }
		  nodes[j].error = 1;
		} else if (!SwigType_isconst(d1) && SwigType_isconst(d2)) {
		  differ = 1;
		  if (!nodes[j].error) {
                    if (script_lang_wrapping) {
		      Swig_warning(WARN_LANG_OVERLOAD_CONST, Getfile(nodes[j].n), Getline(nodes[j].n),
				   "Overloaded %s(%s) const ignored. Non-const method at %s:%d used.\n",
				   Getattr(nodes[j].n,"name"), ParmList_protostr(nodes[j].parms),
				   Getfile(nodes[i].n), Getline(nodes[i].n));
                    } else {
                      if (!Getattr(nodes[j].n, "overload:ignore"))
		        Swig_warning(WARN_LANG_OVERLOAD_IGNORED, Getfile(nodes[j].n), Getline(nodes[j].n),
				     "Overloaded method %s(%s) const ignored. Method %s(%s) at %s:%d used.\n",
				     Getattr(nodes[j].n,"name"), ParmList_protostr(nodes[j].parms),
			             Getattr(nodes[i].n,"name"), ParmList_protostr(nodes[i].parms),
				     Getfile(nodes[i].n), Getline(nodes[i].n));
                    }
                  }
		  nodes[j].error = 1;
		}
	      }
	      Delete(dq1);
	      Delete(dq2);
	    }
	  }
	  if (!differ) {
	    if (!nodes[j].error) {
              if (script_lang_wrapping) {
	        Swig_warning(WARN_LANG_OVERLOAD_SHADOW, Getfile(nodes[j].n), Getline(nodes[j].n),
			     "Overloaded %s(%s)%s is shadowed by %s(%s)%s at %s:%d.\n",
			     Getattr(nodes[j].n,"name"), ParmList_protostr(nodes[j].parms),
			     SwigType_isconst(Getattr(nodes[j].n,"decl")) ? " const" : "", 
			     Getattr(nodes[i].n,"name"), ParmList_protostr(nodes[i].parms),
			     SwigType_isconst(Getattr(nodes[i].n,"decl")) ? " const" : "", 
			     Getfile(nodes[i].n),Getline(nodes[i].n));
              } else {
                if (!Getattr(nodes[j].n, "overload:ignore"))
	          Swig_warning(WARN_LANG_OVERLOAD_IGNORED, Getfile(nodes[j].n), Getline(nodes[j].n),
			       "Overloaded method %s(%s)%s ignored. Method %s(%s)%s at %s:%d used.\n",
			       Getattr(nodes[j].n,"name"), ParmList_protostr(nodes[j].parms),
			       SwigType_isconst(Getattr(nodes[j].n,"decl")) ? " const" : "", 
                               Getattr(nodes[i].n,"name"), ParmList_protostr(nodes[i].parms),
			       SwigType_isconst(Getattr(nodes[i].n,"decl")) ? " const" : "", 
			       Getfile(nodes[i].n),Getline(nodes[i].n));
              }
	      nodes[j].error = 1;
	    }
	  }
	}
      }
    }
  }
  List *result = NewList();
  {
    int i;
    for (i = 0; i < nnodes; i++) {
      if (nodes[i].error)
        Setattr(nodes[i].n, "overload:ignore", "1");
      Append(result,nodes[i].n);
      //      Printf(stdout,"[ %d ] %s\n", i, ParmList_protostr(nodes[i].parms));
      //      Swig_print_node(nodes[i].n);
    }
  }
  return result;
}

/* -----------------------------------------------------------------------------
 * print_typecheck()
 * ----------------------------------------------------------------------------- */

static bool
print_typecheck(String *f, int j, Parm *pj) {
  char tmp[256];
  sprintf(tmp,Char(argv_template_string),j);
  String *tm = Getattr(pj,"tmap:typecheck");
  if (tm) {
    Replaceid(tm,Getattr(pj,"lname"),"_v");
    Replaceall(tm,"$input", tmp);
    Printv(f,tm,"\n",NIL);
    return true;
  }
  else
    return false;
}

/* -----------------------------------------------------------------------------
 * ReplaceFormat()
 * ----------------------------------------------------------------------------- */

static String *
ReplaceFormat (const String_or_char *fmt, int j) {
  String *lfmt = NewString (fmt);
  char buf[50];
  sprintf (buf, "%d", j);
  Replaceall (lfmt, "$numargs", buf);
  int i;
  String *commaargs = NewString ("");
  for (i=0; i < j; i++) {
    Printv (commaargs, ", ", NIL);
    Printf (commaargs, Char(argv_template_string), i);
  }
  Replaceall (lfmt, "$commaargs", commaargs);
  return lfmt;
}

/* -----------------------------------------------------------------------------
 * Swig_overload_dispatch()
 *
 * Generate a dispatch function.  argc is assumed to hold the argument count.
 * argv is the argument vector.
 *
 * Note that for C++ class member functions, Swig_overload_dispatch() assumes
 * that argc includes the "self" argument and that the first element of argv[]
 * is the "self" argument. So for a member function:
 *
 *     Foo::bar(int x, int y, int z);
 *
 * the argc should be 4 (not 3!) and the first element of argv[] would be
 * the appropriate scripting language reference to "self". For regular
 * functions (and static class functions) the argc and argv only include
 * the regular function arguments.
 * ----------------------------------------------------------------------------- */

String *
Swig_overload_dispatch(Node *n, const String_or_char *fmt, int *maxargs) {
  int i,j;
  
  *maxargs = 1;

  String *f = NewString("");

  /* Get a list of methods ranked by precedence values and argument count */
  List *dispatch = Swig_overload_rank(n, true);
  int   nfunc = Len(dispatch);

  /* Loop over the functions */

  for (i = 0; i < nfunc; i++) {
    Node *ni = Getitem(dispatch,i);
    Parm *pi = Getattr(ni,"wrap:parms");
    int num_required = emit_num_required(pi);
    int num_arguments = emit_num_arguments(pi);
    if (num_arguments > *maxargs) *maxargs = num_arguments;
    int varargs = emit_isvarargs(pi);    
    
    if (!varargs) {
      if (num_required == num_arguments) {
	Printf(f,"if (%s == %d) {\n", argc_template_string, num_required);
      } else {
	Printf(f,"if ((%s >= %d) && (%s <= %d)) {\n", 
	       argc_template_string, num_required, 
	       argc_template_string, num_arguments);
      }
    } else {
      Printf(f,"if (%s >= %d) {\n", argc_template_string, num_required);
    }

    if (num_arguments) {
      Printf(f,"int _v;\n");
    }
    
    int num_braces = 0;
    j = 0;
    Parm *pj = pi;
    while (pj) {
      if (checkAttribute(pj,"tmap:in:numinputs","0")) {
	pj = Getattr(pj,"tmap:in:next");
	continue;
      }
      if (j >= num_required) {
	String *lfmt = ReplaceFormat (fmt, num_arguments);
	Printf(f, "if (%s <= %d) {\n", argc_template_string, j);
	Printf(f, Char(lfmt),Getattr(ni,"wrap:name"));
	Printf(f, "}\n");
	Delete (lfmt);
      }
      if (print_typecheck(f, j, pj)) {
	Printf(f, "if (_v) {\n");
	num_braces++;
      }
      Parm *pk = Getattr(pj,"tmap:in:next");
      if (pk) pj = pk;
      else pj = nextSibling(pj);
      j++;
    }
    String *lfmt = ReplaceFormat (fmt, num_arguments);
    Printf(f, Char(lfmt),Getattr(ni,"wrap:name"));
    Delete (lfmt);
    /* close braces */
    for (/* empty */; num_braces > 0; num_braces--)
      Printf(f, "}\n");
    Printf(f,"}\n"); /* braces closes "if" for this method */
  }
  Delete(dispatch);
  return f;
}

/* -----------------------------------------------------------------------------
 * Swig_overload_check()
 * ----------------------------------------------------------------------------- */
void Swig_overload_check(Node *n) {
    Swig_overload_rank(n, false);
}

