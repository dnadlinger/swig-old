/* -----------------------------------------------------------------------------
 * emit.cxx
 *
 *     Useful functions for emitting various pieces of code.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1998-2000.  The University of Chicago
 * Copyright (C) 1995-1998.  The University of Utah and The Regents of the
 *                           University of California.
 *
 * See the file LICENSE for information on usage and redistribution.
 * ----------------------------------------------------------------------------- */

#include "swigmod.h"

char cvsroot_emit_cxx[] = "$Header$";

extern SwigType *cplus_value_type(SwigType *t);

/* -----------------------------------------------------------------------------
 * emit_args()
 *
 * Creates a list of variable declarations for both the return value
 * and function parameters.
 *
 * The return value is always called result and arguments arg0, arg1, arg2, etc...
 * Returns the number of parameters associated with a function.
 * ----------------------------------------------------------------------------- */

void emit_args(SwigType *rt, ParmList *l, Wrapper *f) {

  Parm *p;
  String *tm;

  /* Emit function arguments */
  Swig_cargs(f, l);

  /* Handle return type */
  if (rt && (SwigType_type(rt) != T_VOID)) {
    if (!CPlusPlus || (CPlusPlus && !SwigType_isclass(rt))) {
      Wrapper_add_local(f,"result", SwigType_lstr(rt,"result"));
    } else {
      SwigType *vt = 0;
      vt = cplus_value_type(rt);
      if (!vt) {
	Wrapper_add_local(f,"result", SwigType_lstr(rt,"result"));
      } else {
	Wrapper_add_local(f,"result", SwigType_lstr(vt,"result"));
	Delete(vt);
      }
    }
  }
  
  /* Attach typemaps to parameters */
  /*  Swig_typemap_attach_parms("ignore",l,f); */

  Swig_typemap_attach_parms("default",l,f);
  Swig_typemap_attach_parms("arginit",l,f);

  /* Apply the arginit and default */
  p = l;
  while (p) {
    tm = Getattr(p,"tmap:arginit");
    if (tm) {
      Replace(tm,"$target", Getattr(p,"lname"), DOH_REPLACE_ANY);
      Printv(f->code,tm,"\n",NIL);
      p = Getattr(p,"tmap:arginit:next");
    } else {
      p = nextSibling(p);
    }
  }

  /* Apply the default typemap */
  p = l;
  while (p) {
    tm = Getattr(p,"tmap:default");
    if (tm) {
      Replace(tm,"$target", Getattr(p,"lname"), DOH_REPLACE_ANY);
      Printv(f->code,tm,"\n",NIL);
      p = Getattr(p,"tmap:default:next");
    } else {
      p = nextSibling(p);
    }
  }

#ifdef DEPRECATED  
  /* Apply the ignore typemap */
  p = l;
  while (p) {
    tm = Getattr(p,"tmap:ignore");
    if (tm) {
      Parm *np;
      Replace(tm,"$target", Getattr(p,"lname"), DOH_REPLACE_ANY);
      Printv(f->code,tm,"\n",NIL);
      np = Getattr(p,"tmap:ignore:next");

      /* Deprecate this part later */
      while (p && (p != np)) {
	Setattr(p,"ignore","1");
	p = nextSibling(p);
      }
      /* -- end deprecate */

    } else {
      p = nextSibling(p);
    }
  }
#endif
  return;
}

/* -----------------------------------------------------------------------------
 * emit_attach_parmmaps()
 *
 * Attach the standard parameter related typemaps.
 * ----------------------------------------------------------------------------- */

void emit_attach_parmmaps(ParmList *l, Wrapper *f) {
  Swig_typemap_attach_parms("in",l,f);
  Swig_typemap_attach_parms("typecheck",l,0);
  Swig_typemap_attach_parms("argout",l,f);
  Swig_typemap_attach_parms("check",l,f);
  Swig_typemap_attach_parms("freearg",l,f);

  {
    /* This is compatibility code to deal with the deprecated "ignore" typemap */
    Parm *p = l;
    Parm *np;
    String *tm;
    while (p) {
      tm = Getattr(p,"tmap:in");
      if (tm && checkAttribute(p,"tmap:in:numinputs","0")) {
	Replaceall(tm,"$target", Getattr(p,"lname"));
	Printv(f->code,tm,"\n",NIL);
	np = Getattr(p,"tmap:in:next");
	while (p && (p != np)) {
	  Setattr(p,"ignore","1");
	  p = nextSibling(p);
	}
      } else if (tm) {
	p = Getattr(p,"tmap:in:next");
      } else {
	p = nextSibling(p);
      }
    }
  }

  /* Perform a sanity check on "in" and "freearg" typemaps.  These
     must exactly match to avoid chaos.  If a mismatch occurs, we
     nuke the freearg typemap */

  {
    Parm *p = l;
    Parm *npin, *npfreearg;
    while (p) {
      npin = Getattr(p,"tmap:in:next");
      
      /*
      if (Getattr(p,"tmap:ignore")) {
	npin = Getattr(p,"tmap:ignore:next");
      } else if (Getattr(p,"tmap:in")) {
	npin = Getattr(p,"tmap:in:next");
      }
      */

      if (Getattr(p,"tmap:freearg")) {
	npfreearg = Getattr(p,"tmap:freearg:next");
	if (npin != npfreearg) {
	  while (p != npin) {
	    Delattr(p,"tmap:freearg");
	    Delattr(p,"tmap:freearg:next");
	    p = nextSibling(p);
	  }
	}
      }
      p = npin;
    }
  }
      
  /* Check for variable length arguments with no input typemap.
     If no input is defined, we set this to ignore and print a
     message.
   */
  {
    Parm *p = l;
    Parm *lp = 0;
    while (p) {
      if (!checkAttribute(p,"tmap:in:numinputs","0")) {
	lp = p;
	p = Getattr(p,"tmap:in:next");
	continue;
      }
      if (SwigType_isvarargs(Getattr(p,"type"))) {
	Swig_warning(WARN_LANG_VARARGS,input_file,line_number,"Variable length arguments discarded.\n");
	Setattr(p,"tmap:in","");
      }
      lp = 0;
      p = nextSibling(p);
    }
    
    /* Check if last input argument is variable length argument */
    if (lp) {
      p = lp;
      while (p) {
	if (SwigType_isvarargs(Getattr(p,"type"))) {
	  Setattr(l,"emit:varargs",lp);
	  break;
	}
	p = nextSibling(p);
      }
    }
  }
}

/* -----------------------------------------------------------------------------
 * emit_num_arguments()                                         ** new in 1.3.10
 *
 * Calculate the total number of arguments.   This function is safe for use
 * with multi-valued typemaps which may change the number of arguments in
 * strange ways.
 * ----------------------------------------------------------------------------- */

int emit_num_arguments(ParmList *parms) {
  Parm *p = parms;
  int   nargs = 0;

  while (p) {
    if (Getattr(p,"tmap:in")) {
      nargs += GetInt(p,"tmap:in:numinputs");
      p = Getattr(p,"tmap:in:next");
    } else {
      p = nextSibling(p);
    }
  }

#ifdef DEPRECATED
  while (p) {
    /* Ignored arguments */
    if (Getattr(p,"tmap:ignore")) {
      p = Getattr(p,"tmap:ignore:next");
    } else {
      /* Marshalled arguments */
      nargs++;
      if (Getattr(p,"tmap:in")) {
	p = Getattr(p,"tmap:in:next");
      } else {
	p = nextSibling(p);
      }
    }
  }
#endif

  /* DB 04/02/2003: Not sure this is necessary with tmap:in:numinputs */
  /*
  if (parms && (p = Getattr(parms,"emit:varargs"))) {
    if (!nextSibling(p)) {
      nargs--;
    }
  }
  */
  return nargs;
}

/* -----------------------------------------------------------------------------
 * emit_num_required()                                          ** new in 1.3.10
 *
 * Computes the number of required arguments.  This is function is safe for
 * use with multi-valued typemaps and knows how to skip over everything
 * properly.
 * ----------------------------------------------------------------------------- */

int emit_num_required(ParmList *parms) {
  Parm *p = parms;
  int   nargs = 0;

  while (p) {
    if (Getattr(p,"tmap:in") && checkAttribute(p,"tmap:in:numinputs","0")) {
      p = Getattr(p,"tmap:in:next");
    } else {
      if (Getattr(p,"value")) break;
      if (Getattr(p,"tmap:default")) break;
      nargs+= GetInt(p,"tmap:in:numinputs");
      if (Getattr(p,"tmap:in")) {
	p = Getattr(p,"tmap:in:next");
      } else {
	p = nextSibling(p);
      }
    }
  }

  /* Print message for non-default arguments */
  while (p) {
    if (Getattr(p,"tmap:in") && checkAttribute(p,"tmap:in:numinputs","0")) {
      p = Getattr(p,"tmap:in:next");
    } else {
      if (!Getattr(p,"value") && (!Getattr(p,"tmap:default"))) {
	Swig_error(Getfile(p),Getline(p),"Error. Non-optional argument '%s' follows an optional argument.\n",Getattr(p,"name"));
      }
      if (Getattr(p,"tmap:in")) {
	p = Getattr(p,"tmap:in:next");
      } else {
	p = nextSibling(p);
      }
    }
  }
  /* DB 04/02/2003: Not sure this is necessary with tmap:in:numinputs */
  /*
  if (parms && (p = Getattr(parms,"emit:varargs"))) {
    if (!nextSibling(p)) {
      nargs--;
    }
  }
  */
  return nargs;
}

/* -----------------------------------------------------------------------------
 * emit_isvarargs()
 *
 * Checks if a function is a varargs function
 * ----------------------------------------------------------------------------- */

int
emit_isvarargs(ParmList *p) {
  if (!p) return 0;
  if (Getattr(p,"emit:varargs")) return 1;
  return 0;
}

/* -----------------------------------------------------------------------------
 * replace_args()
 * ----------------------------------------------------------------------------- */

static
void replace_args(Parm *p, String *s) {
  while (p) {
    String *n = Getattr(p,"name");
    if (n) {
      Replace(s,n,Getattr(p,"lname"), DOH_REPLACE_ID);
    }
    p = nextSibling(p);
  }
}
 
/* -----------------------------------------------------------------------------
 * int emit_action()
 *
 * Emits action code for a wrapper and checks for exception handling
 * ----------------------------------------------------------------------------- */

void emit_action(Node *n, Wrapper *f) {
  String *tm;
  String *action;
  String *wrap;
  Parm   *p;
  SwigType *rt;
  ParmList *throws = Getattr(n,"throws");

  /* Look for fragments */
  {
    String *f;
    f = Getattr(n,"feature:fragment");
    if (f) {
      char  *c, *tok;
      String *t = Copy(f);
      c = Char(t);
      tok = strtok(c,",");
      while (tok) {
	Swig_fragment_emit(tok);
	tok = strtok(NULL,",");
      }
      Delete(t);
    }
  }

  /* Emit wrapper code (if any) */
  wrap = Getattr(n,"wrap:code");
  if (wrap && Swig_filebyname("header")!=Getattr(n,"wrap:code:done") ) {
    File *f_code = Swig_filebyname("header");
    if (f_code) {
      Printv(f_code,wrap,NIL);
    }
    Setattr(n,"wrap:code:done",f_code);
  }
  action = Getattr(n,"feature:action");
  if (!action)
    action = Getattr(n,"wrap:action");
  assert(action != 0);

  /* Get the return type */

  rt = Getattr(n,"type");

  /* Preassert -- EXPERIMENTAL */
  tm = Getattr(n,"feature:preassert");
  if (tm) {
    p = Getattr(n,"parms");
    replace_args(p,tm);
    Printv(f->code,tm,"\n",NIL);
  }

  /* Exception handling code */

  /* If we are in C++ mode and there is a throw specifier. We're going to
     enclose the block in a try block */

  if (throws) {
    Printf(f->code,"try {\n");
  }

  /* Look for except typemap (Deprecated) */
  tm = Swig_typemap_lookup_new("except",n,"result",0);

  /* Look for except feature */
  if (!tm) {
    tm = Getattr(n,"feature:except");
    if (tm) tm = Copy(tm);
  }
  if ((tm) && Len(tm) && (Strcmp(tm,"1") != 0)) {
    Replaceall(tm,"$name",Getattr(n,"name"));
    Replaceall(tm,"$symname", Getattr(n,"sym:name"));
    Replaceall(tm,"$function", action);
    Replaceall(tm,"$action", action);
    Printv(f->code,tm,"\n", NIL);
    Delete(tm);
  } else {
    Printv(f->code, action, "\n",NIL);
  }

  if (throws) {
    Printf(f->code,"}\n");
    for (Parm *ep = throws; ep; ep = nextSibling(ep)) {
      String *em = Swig_typemap_lookup_new("throws",ep,"_e",0);
      if (em) {
	Printf(f->code,"catch(%s) {\n", SwigType_str(Getattr(ep,"type"),"&_e"));
	Printv(f->code,em,"\n",NIL);
	Printf(f->code,"}\n");
      } else {
	Swig_warning(WARN_TYPEMAP_THROW, Getfile(n), Getline(n),
		     "No 'throw' typemap defined for exception type '%s'\n", SwigType_str(Getattr(ep,"type"),0));
      }
    }
    Printf(f->code,"catch(...) { throw; }\n");
  }

  /* Postassert - EXPERIMENTAL */
  tm = Getattr(n,"feature:postassert");
  if (tm) {
    p = Getattr(n,"parms");
    replace_args(p,tm);
    Printv(f->code,tm,"\n",NIL);
  }
}




