/* ----------------------------------------------------------------------------- 
 * util.c
 *
 *     Parsing utilities
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

char cvsroot_util_c[] = "$Header$";

#include "swig.h"

extern SwigType *Swig_cparse_type(String *);

/* -----------------------------------------------------------------------------
 * Swig_cparse_replace_descriptor()
 *
 * Replaces type descriptor string $descriptor() with the SWIG type descriptor
 * string.
 * ----------------------------------------------------------------------------- */

void Swig_cparse_replace_descriptor(String *s) {
  char   tmp[512];
  String *arg = 0;
  SwigType *t;

  while (Strstr(s,"$descriptor(")) {
    char *d = tmp;
    int  level = 0;
    char *c = Strstr(s,"$descriptor(");
    while (*c) {
      if (*c == '(') level++;
      if (*c == ')') {
	level--;
	if (level == 0) {
	  break;
	}
      }
      *d = *c;
      d++;
      c++;
    }
    *d = 0;
    arg = NewString(tmp+12);
    t = Swig_cparse_type(arg);
    Delete(arg);
    arg = 0;

    if (t) {
      String *mangle;
      String *descriptor;
      
      mangle = SwigType_manglestr(t);
      descriptor = NewStringf("SWIGTYPE%s",mangle);
      SwigType_remember(t);
      *d = ')';
      d++;
      *d = 0;
      Replace(s,tmp,descriptor,DOH_REPLACE_ANY);
      Delete(mangle);
      Delete(descriptor);
    } else {
      Swig_error(Getfile(s),Getline(s),"Bad $descriptor() macro.\n");
      break;
    }
  }
}

/* -----------------------------------------------------------------------------
 * cparse_normalize_void()
 *
 * This function is used to replace arguments of the form (void) with empty
 * arguments in C++
 * ----------------------------------------------------------------------------- */

void cparse_normalize_void(Node *n) {
  String *decl = Getattr(n,"decl");
  Parm   *parms = Getattr(n,"parms");

  if (SwigType_isfunction(decl)) {
    if ((ParmList_len(parms) == 1) && (SwigType_type(Getattr(parms,"type")) == T_VOID)) {
      Replaceall(decl,"f(void).","f().");
      Delattr(n,"parms");
    }
  }
}
