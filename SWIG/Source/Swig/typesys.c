/* -----------------------------------------------------------------------------
 * typesys.c
 *
 *     SWIG type system management.   These functions are used to manage
 *     the C++ type system including typenames, typedef, type scopes, 
 *     inheritance, and namespaces.   Generation of support code for the
 *     run-time type checker is also handled here.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 * ----------------------------------------------------------------------------- */

char cvsroot_typesys_c[] = "$Header$";

#include "swig.h"

/* -----------------------------------------------------------------------------
 * Synopsis
 *
 * The purpose of this module is to manage type names and scoping issues related
 * to the C++ type system.   The primary use is tracking typenames through typedef
 * and inheritance. 
 *
 * New typenames are introduced by typedef, class, and enum declarations.
 * Each type is declared in a scope.  This is either the global scope, a
 * class, or a namespace.  For example:
 *
 *  typedef int A;         // Typename A, in global scope
 *  namespace Foo {
 *    typedef int A;       // Typename A, in scope Foo::
 *  }
 *  class Bar {            // Typename Bar, in global scope
 *    typedef int A;       // Typename A, in scope Bar::
 *  }
 *
 * To manage scopes, the type system is constructed as a tree of hash tables.  Each 
 * hash table contains the following attributes:
 *
 *    "name"            -  Scope name
 *    "qname"           -  Fully qualified typename
 *    "typetab"         -  Type table containing typenames and typedef information
 *    "symtab"          -  Hash table of symbols defined in a scope
 *    "inherit"         -  List of inherited scopes       
 *    "parent"          -  Parent scope
 * 
 * Typedef information is stored in the "typetab" hash table.  For example,
 * if you have these declarations:
 *
 *      typedef int A;
 *      typedef A B;
 *      typedef B *C;
 *
 * typetab is built as follows:
 *
 *      "A"     : "int"
 *      "B"     : "A"
 *      "C"     : "p.B"
 *
 * To resolve a type back to its root type, one repeatedly expands on the type base.
 * For example:
 *
 *     C *[40]   ---> a(40).p.C       (string type representation, see stype.c)
 *               ---> a(40).p.p.B     (C --> p.B)
 *               ---> a(40).p.p.A     (B --> A)
 *               ---> a(40).p.p.int   (A --> int)
 *
 * For inheritance, SWIG tries to resolve types back to the base class. For instance, if
 * you have this:
 *
 *     class Foo {
 *     public:
 *        typedef int Integer;
 *     };
 *
 *     class Bar : public Foo {
 *           void blah(Integer x);
 *     }
 *
 * The argument type of Bar::blah will be set to Foo::Integer.   
 *
 * The scope-inheritance mechanism is used to manage C++ namespace aliases.
 * For example, if you have this:
 *
 *    namespace Foo {
 *         typedef int Integer;
 *    }
 *
 *   namespace F = Foo;
 *
 * In this case, "F::" is defined as a scope that "inherits" from Foo.  Internally,
 * "F::" will merely be an empty scope that refers to Foo.  SWIG will never 
 * place new type information into a namespace alias---attempts to do so
 * will generate a warning message (in the parser) and will place information into 
 * Foo instead.
 *
 *----------------------------------------------------------------------------- */

static Typetab *current_scope = 0;                /* Current type scope                           */
static Hash    *current_typetab = 0;              /* Current type table                           */
static Hash    *current_symtab  = 0;              /* Current symbol table                         */
static Typetab *global_scope  = 0;                /* The global scope                             */
static Hash    *scopes        = 0;                /* Hash table containing fully qualified scopes */

/* Performance optimization */
static Hash     *typedef_resolve_cache = 0;
static Hash     *typedef_all_cache = 0;
static Hash     *typedef_qualified_cache = 0;

static void flush_cache() {
  typedef_resolve_cache = 0;
  typedef_all_cache = 0;
  typedef_qualified_cache = 0;
}

/* Initialize the scoping system */

void SwigType_typesystem_init() {
  if (global_scope) Delete(global_scope);
  if (scopes)       Delete(scopes);

  current_scope = NewHash();
  global_scope  = current_scope;

  Setattr(current_scope,"name","");              /* No name for global scope */
  current_typetab = NewHash();
  Setattr(current_scope,"typetab", current_typetab);

  current_symtab = 0;
  scopes = NewHash();
  Setattr(scopes,"",current_scope);
}

/* -----------------------------------------------------------------------------
 * SwigType_typedef()
 *
 * Defines a new typedef in the current scope. Returns -1 if the type name is 
 * already defined.  
 * ----------------------------------------------------------------------------- */

int SwigType_typedef(SwigType *type, String_or_char *name) {
  Typetab *SwigType_find_scope(Typetab *, String *s);
  if (Getattr(current_typetab, name)) return -1;   /* Already defined */
  if (Strcmp(type,name) == 0) {                    /* Can't typedef a name to itself */
    return 0;
  }
  
  /* Check if 'type' is already a scope.  If so, we create an alias in the type
     system for it.  This is needed to make strange nested scoping problems work
     correctly.  */
  {
    Typetab *t = SwigType_find_scope(current_scope,type);
    if (t) {
      SwigType_new_scope(name);
      SwigType_inherit_scope(t);
      SwigType_pop_scope();
    }
  }
  Setattr(current_typetab,name,type);
  flush_cache();
  return 0;
}


/* -----------------------------------------------------------------------------
 * SwigType_typedef_class()
 *
 * Defines a class in the current scope. 
 * ----------------------------------------------------------------------------- */

int SwigType_typedef_class(String_or_char *name) {
  String *cname;
  /*  Printf(stdout,"class : '%s'\n", name); */
  if (Getattr(current_typetab, name)) return -1;   /* Already defined */
  cname = NewString(name);
  Setmeta(cname,"class","1");
  Setattr(current_typetab,cname,cname);
  Delete(cname);
  flush_cache();
  return 0;
}

/* -----------------------------------------------------------------------------
 * SwigType_scope_name()
 *
 * Returns the qualified scope name of a type table
 * ----------------------------------------------------------------------------- */

String *
SwigType_scope_name(Typetab *ttab) {
  String *qname = NewString(Getattr(ttab,"name"));
  ttab = Getattr(ttab,"parent");
  while (ttab) {
    String *pname = Getattr(ttab,"name");
    if (Len(pname)) {
      Insert(qname,0,"::");
      Insert(qname,0,pname);
    }
    ttab = Getattr(ttab,"parent");
  }
  return qname;
}
/* -----------------------------------------------------------------------------
 * SwigType_new_scope()
 *
 * Creates a new scope
 * ----------------------------------------------------------------------------- */

void SwigType_new_scope(String_or_char *name) {
  Typetab *s;
  Hash    *ttab;
  String  *qname;

  if (!name) {
    name = "<unnamed>";
  }
  s = NewHash();
  Setattr(s,"name", name);
  Setattr(s,"parent", current_scope);
  ttab = NewHash();
  Setattr(s,"typetab", ttab);

  /* Build fully qualified name and */
  qname = SwigType_scope_name(s);
  Setattr(scopes,qname,s);
  Setattr(s,"qname",qname);
  Delete(qname);

  current_scope = s;
  current_typetab = ttab;
  current_symtab = 0;
  flush_cache();
}

/* -----------------------------------------------------------------------------
 * SwigType_inherit_scope()
 *
 * Makes the current scope inherit from another scope.  This is used for both
 * C++ class inheritance, namespaces, and namespace aliases.
 * ----------------------------------------------------------------------------- */

void 
SwigType_inherit_scope(Typetab *scope) {
  List *inherits;
  int   i, len;
  inherits = Getattr(current_scope,"inherit");
  if (!inherits) {
    inherits = NewList();
    Setattr(current_scope,"inherit", inherits);
  }
  assert(scope != current_scope);
  
  len = Len(inherits);
  for (i = 0; i < len; i++) {
    Node *n = Getitem(inherits,i);
    if (n == scope) return;
  }
  Append(inherits,scope);
}

/* -----------------------------------------------------------------------------
 * SwigType_scope_alias()
 *
 * Creates a scope-alias.
 * ----------------------------------------------------------------------------- */

void
SwigType_scope_alias(String *aliasname, Typetab *ttab) {
  String *q;
  /*  Printf(stdout,"alias: '%s' '%x'\n", aliasname, ttab);*/
  q = SwigType_scope_name(current_scope);
  if (Len(q)) {
    Append(q,"::");
  } 
  Append(q,aliasname);
  Setattr(scopes,q,ttab);
  flush_cache();
}

/* -----------------------------------------------------------------------------
 * SwigType_using_scope()
 *
 * Import another scope into this scope.
 * ----------------------------------------------------------------------------- */

void
SwigType_using_scope(Typetab *scope) {
  SwigType_inherit_scope(scope);
  {
    List *ulist;
    int   i, len;
    ulist = Getattr(current_scope,"using");
    if (!ulist) {
      ulist = NewList();
      Setattr(current_scope,"using", ulist);
    }
    assert(scope != current_scope);
    len = Len(ulist);
    for (i = 0; i < len; i++) {
      Typetab *n = Getitem(ulist,i);
      if (n == scope) return;
    }
    Append(ulist,scope);
  }
  flush_cache();
}
 
/* -----------------------------------------------------------------------------
 * SwigType_pop_scope()
 *
 * Pop off the last scope and perform a merge operation.  Returns the hash
 * table for the scope that was popped off.
 * ----------------------------------------------------------------------------- */

Typetab *SwigType_pop_scope() {
  Typetab *s, *s1;
  s = Getattr(current_scope,"parent");
  if (!s) {
    current_scope = 0;
    current_typetab = 0;
    current_symtab = 0;
    return 0;
  }
  s1 = current_scope;
  current_scope = s;
  current_typetab = Getattr(s,"typetab");
  current_symtab = Getattr(s,"symtab");
  flush_cache();
  return s1;
}

/* -----------------------------------------------------------------------------
 * SwigType_set_scope()
 *
 * Set the scope.  Returns the old scope.
 * ----------------------------------------------------------------------------- */

Typetab *
SwigType_set_scope(Typetab *t) {
  Typetab *old = current_scope;
  if (!t) t = global_scope;
  current_scope = t;
  current_typetab = Getattr(t,"typetab");
  current_symtab = Getattr(t,"symtab");
  flush_cache();
  return old;
}

/* -----------------------------------------------------------------------------
 * SwigType_attach_symtab()
 *
 * Attaches a symbol table to a type scope
 * ----------------------------------------------------------------------------- */

void
SwigType_attach_symtab(Symtab *sym) {
  Setattr(current_scope,"symtab",sym);
  current_symtab = sym;
}

/* -----------------------------------------------------------------------------
 * SwigType_print_scope()
 *
 * Debugging function for printing out current scope
 * ----------------------------------------------------------------------------- */

void SwigType_print_scope(Typetab *t) {
  Hash   *ttab;
  Iterator  i,j;

  for (i = First(scopes); i.key; i = Next(i)) {
    t = i.item;
    ttab = Getattr(i.item,"typetab");
    
    Printf(stdout,"Type scope '%s' (%x)\n", i.key, i.item);
    {
      List *inherit = Getattr(i.item,"inherit");
      if (inherit) {
	Iterator j;
	for (j = First(inherit); j.item; j = Next(j)) {
	  Printf(stdout,"    Inherits from '%s' (%x)\n", Getattr(j.item,"qname"), j.item);
	}
      }
    }
    Printf(stdout,"-------------------------------------------------------------\n");
    for (j = First(ttab); j.key; j = Next(j)) {
      Printf(stdout,"%40s -> %s\n", j.key, j.item);
    }
  }
}

Typetab *
SwigType_find_scope(Typetab *s, String *nameprefix) {
  Typetab *ss;
  String  *nnameprefix = 0;
  static   int check_parent = 1;

  /*  Printf(stdout,"find_scope: %x(%s) '%s'\n", s, Getattr(s,"name"), nameprefix); */

  if (SwigType_istemplate(nameprefix)) {
    nnameprefix = SwigType_typedef_resolve_all(nameprefix);
    nameprefix = nnameprefix;
  }

  ss = s;
  while (ss) {
    String *full;
    String *qname = Getattr(ss,"qname");
    if (qname) {
      full = NewStringf("%s::%s", qname, nameprefix);
    } else {
      full = NewString(nameprefix);
    }
    if (Getattr(scopes,full)) {
      s = Getattr(scopes,full);
    } else {
      s = 0;
    }
    Delete(full);
    if (s) {
      if (nnameprefix) Delete(nnameprefix);
      return s;
    }
    if (!s) {
      /* Check inheritance */
      List *inherit;
      inherit = Getattr(ss,"using");
      if (inherit) {
	Typetab *ttab;
	int i, len;
	len = Len(inherit);
	for (i = 0; i < len; i++) {
	  int oldcp = check_parent;
	  ttab = Getitem(inherit,i);
	  check_parent = 0;
	  s = SwigType_find_scope(ttab,nameprefix);
	  check_parent = oldcp;
	  if (s) {
	    if (nnameprefix) Delete(nnameprefix);
	    return s;
	  }
	}
      }
    }
    if (!check_parent) break;
    ss = Getattr(ss,"parent");
  }
  if (nnameprefix) Delete(nnameprefix);
  return 0;
}

/* ----------------------------------------------------------------------------- 
 * SwigType_typedef_resolve()
 *
 * Resolves a typedef and returns a new type string.  Returns 0 if there is no
 * typedef mapping.  base is a name without qualification.
 * ----------------------------------------------------------------------------- */

static Typetab  *resolved_scope = 0;

/* Internal function */
static SwigType *
typedef_resolve(Typetab *s, String *base) {
  Hash     *ttab;
  SwigType *type;
  List     *inherit;

  if (!s) return 0;
  /* Printf(stdout,"Typetab %s : %s\n", Getattr(s,"name"), base);  */

  if (Getmark(s)) return 0;
  Setmark(s,1);

  ttab = Getattr(s,"typetab");
  type = Getattr(ttab,base);
  if (type) {
    resolved_scope = s;
    Setmark(s,0);
    return type;
  }
  /* Hmmm. Not found in my scope.  It could be in an inherited scope */
  inherit = Getattr(s,"inherit");
  if (inherit) {
    int i,len;
    len = Len(inherit);
    for (i = 0; i < len; i++) {
      type = typedef_resolve(Getitem(inherit,i), base);
      if (type) {
	Setmark(s,0);
	return type;
      }
    }
  }
  type = typedef_resolve(Getattr(s,"parent"), base);
  Setmark(s,0);
  return type;
}

SwigType *SwigType_typedef_resolve(SwigType *t) {
  String *base;
  String *type = 0;
  String *r = 0;
  Typetab  *s;
  Hash     *ttab;
  String *namebase = 0;
  String *nameprefix = 0;
  int     newtype = 0;

  /*
  if (!noscope) {
    noscope = NewString("");
  }
  */

  resolved_scope = 0;

  /*
  if (!typedef_resolve_cache) {
    typedef_resolve_cache = NewHash();
  }
  r = Getattr(typedef_resolve_cache,t);
  if (r) {
    if (r != noscope) {
      resolved_scope = Getmeta(r,"scope");
      return Copy(r);
    } else {
      return 0;
    }
  }
  */

  base = SwigType_base(t);

  /* Printf(stdout,"base = '%s'\n", base); */

  if (SwigType_issimple(base)) {
    s = current_scope;
    ttab = current_typetab;
    if (Strncmp(base,"::",2) == 0) {
      s = global_scope;
      ttab = Getattr(s,"typetab");
      Delitem(base,0);
      Delitem(base,0);
    }
    /* Do a quick check in the local scope */
    type = Getattr(ttab,base);
    if (type) {
      resolved_scope = s;
    }
    if (!type) {
      /* Didn't find in this scope.   We need to do a little more searching */
      if (Swig_scopename_check(base)) {
	/* A qualified name. */
	nameprefix = Swig_scopename_prefix(base);
	/*	Printf(stdout,"nameprefix = '%s'\n", nameprefix);*/
	if (nameprefix) {
	  /* Name had a prefix on it.   See if we can locate the proper scope for it */
	  s = SwigType_find_scope(s,nameprefix);

	  /* Couldn't locate a scope for the type.  */
	  if (!s) {
	    Delete(base);
	    Delete(nameprefix);
	    r = 0;
	    goto return_result;
	  }
	  /* Try to locate the name starting in the scope */
	  namebase = Swig_scopename_last(base);
	  /* Printf(stdout,"namebase = '%s'\n", namebase); */
	  type = typedef_resolve(s,namebase);
	  /* Printf(stdout,"%s type = '%s'\n", Getattr(s,"name"), type); */
	  if ((type) && (!Swig_scopename_check(type))) {
	    Typetab *rtab = resolved_scope;
	    String *qname = Getattr(resolved_scope,"qname");
	    /* If qualified *and* the typename is defined from the resolved scope, we qualify */
	    if ((qname) && typedef_resolve(resolved_scope,type)) {
	      type = Copy(type);
	      Insert(type,0,"::");
	      Insert(type,0,qname);
	      newtype = 1;
	    } 
	    resolved_scope = rtab;
	  }
	} else {
	  /* Name is unqualified. */
	  type = typedef_resolve(s,base);
	}
      } else {
	/* Name is unqualified. */
	type = typedef_resolve(s,base);
      }
    }
    
    if (type && (Strcmp(base,type) == 0)) {
      if (newtype) Delete(type);
      Delete(base);
      Delete(namebase);
      Delete(nameprefix);
      r = 0;
      goto return_result;
    }

    /* If the type is a template, and no typedef was found, we need to check the
     template arguments one by one to see if they can be resolved. */

    if (!type && SwigType_istemplate(base)) {
      List *tparms;
      String *suffix;
      int i,sz;
      int rep = 0;
      type = SwigType_templateprefix(base);
      newtype = 1;
      suffix = SwigType_templatesuffix(base);
      Append(type,"<(");
      tparms = SwigType_parmlist(base);
      sz = Len(tparms);
      for (i = 0; i < sz; i++) {
	SwigType *tpr;
	SwigType *tp = Getitem(tparms, i);
	if (!rep) {
	  tpr = SwigType_typedef_resolve(tp);
	} else {
	  tpr = 0;
	}
	if (tpr) {
	  Append(type,tpr);
	  Delete(tpr);
	  rep = 1;
	} else {
	  Append(type,tp);
	}
	if ((i+1) < sz) Append(type,",");
      }
      Append(type,")>");
      Append(type,suffix);
      Delete(suffix);
      Delete(tparms);
      if (!rep) {
	Delete(type);
	type = 0;
      }
    }
    if (namebase) Delete(namebase);
    if (nameprefix) Delete(nameprefix);
    namebase = 0;
    nameprefix = 0;
  } else {
    if (SwigType_isfunction(base)) {
      List *parms;
      int i,sz;
      int rep = 0;
      type = NewString("f(");
      newtype = 1;
      parms = SwigType_parmlist(base);
      sz = Len(parms);
      for (i = 0; i < sz; i++) {
	SwigType *tpr;
	SwigType *tp = Getitem(parms, i);
	if (!rep) {
	  tpr = SwigType_typedef_resolve(tp);
	} else {
	  tpr = 0;
	}
	if (tpr) {
	  Append(type,tpr);
	  Delete(tpr);
	  rep = 1;
	} else {
	  Append(type,tp);
	}
	if ((i+1) < sz) Append(type,",");
      }
      Append(type,").");
      Delete(parms);
      if (!rep) {
	Delete(type);
	type = 0;
      }
    } else if (SwigType_ismemberpointer(base)) {
      String *rt;
      String *mtype = SwigType_parm(base);
      rt = SwigType_typedef_resolve(mtype);
      if (rt) {
	type = NewStringf("m(%s).", rt);
	newtype = 1;
	Delete(rt);
      }
      Delete(mtype);
    } else {
      type = 0;
    }
  }
  r = SwigType_prefix(t);
  if (!type) {
    if (r && Len(r)) {
      if ((Strstr(r,"f(") || (Strstr(r,"m(")))) {
	SwigType *rt = SwigType_typedef_resolve(r);
	if (rt) {
	  Delete(r);
	  Append(rt,base);
	  Delete(base);
	  r = rt;
	  goto return_result;
	}
      }
    }
    Delete(r);
    Delete(base);
    r = 0;
    goto return_result;
  }
  Delete(base);
  Append(r,type);
  if (newtype) {
    Delete(type);
  }

 return_result:
  /*
  {
    String *key = NewString(t);
    if (!r) {
      Setattr(typedef_resolve_cache,key,noscope);
    } else {
      SwigType *r1;
      Setattr(typedef_resolve_cache,key,r);
      Setmeta(r,"scope",resolved_scope);
      r1 = Copy(r);
      Delete(r);
      r = r1;
    }
    Delete(key);
  }
  */
  return r;
}

/* -----------------------------------------------------------------------------
 * SwigType_typedef_resolve_all()
 *
 * Fully resolve a type down to its most basic datatype
 * ----------------------------------------------------------------------------- */

SwigType *SwigType_typedef_resolve_all(SwigType *t) {
  SwigType *n;
  SwigType *r;

  if (!typedef_all_cache) {
    typedef_all_cache = NewHash();
  }
  r = Getattr(typedef_all_cache,t);
  if (r) {
    return Copy(r);
  }
  r = NewString(t);
  while ((n = SwigType_typedef_resolve(r))) {
    Delete(r);
    r = n;
  }
  {
    String *key;
    SwigType *rr = Copy(r);
    key = NewString(t);
    Setattr(typedef_all_cache,key,rr);
    Delete(key);
    Delete(rr);
  }
  return r;
}


/* -----------------------------------------------------------------------------
 * SwigType_typedef_qualified()
 *
 * Given a type declaration, this function tries to fully qualify it according to
 * typedef scope rules.
 * ----------------------------------------------------------------------------- */

SwigType *SwigType_typedef_qualified(SwigType *t) 
{
  List   *elements;
  String *result;
  int     i,len;

  if (!typedef_qualified_cache) typedef_qualified_cache = NewHash();
  result = Getattr(typedef_qualified_cache,t);
  if (result) {
    String *rc = Copy(result);
    return rc;
  }

  result = NewString("");
  elements = SwigType_split(t);
  len = Len(elements);
  for (i = 0; i < len; i++) {
    String *e = Getitem(elements,i);
    if (SwigType_issimple(e)) {
      if (!SwigType_istemplate(e)) {
	String *isenum = 0;
	if (SwigType_isenum(e)) {
	  isenum = e;
	  e = NewString(Char(e)+5);
	}
	resolved_scope = 0;
	if (typedef_resolve(current_scope,e)) {
	  /* resolved_scope contains the scope that actually resolved the symbol */
	  String *qname = Getattr(resolved_scope,"qname");
	  if (qname) {
	    Insert(e,0,"::");
	    Insert(e,0,qname);
	    if (isenum) {
	      Clear(isenum);
	      Printf(isenum, "enum %s", e);
	      Delete(e);
	    }
	  }
	} else {
	  if (Swig_scopename_check(e)) {
	    String *tqname;
	    String *qlast;
	    String *qname = Swig_scopename_prefix(e);
	    if (qname) {
	      qlast = Swig_scopename_last(e);
	      tqname = SwigType_typedef_qualified(qname);
	      Clear(e);
	      Printf(e,"%s::%s", tqname, qlast);
	      Delete(qname);
	      Delete(qlast);
	      Delete(tqname);
	    }
	    /* Automatic template instantiation might go here??? */
	  } else {
	    /* It's a bare name.  It's entirely possible, that the
               name is part of a namespace. We'll check this by unrolling
               out of the current scope */
	    
	    Typetab *cs = current_scope;
	    while (cs) {
	      String *qs = SwigType_scope_name(cs);
	      if (Len(qs)) {
		Append(qs,"::");
	      }
	      Append(qs,e);
	      if (Getattr(scopes,qs)) {
		Clear(e);
		Append(e,qs);
		Delete(qs);
		break;
	      }
	      Delete(qs);
	      cs = Getattr(cs,"parent");
	    }
	  }
	}
	if (isenum) e = isenum;

      } else {
	/* Template.  We need to qualify template parameters as well as the template itself */
	String *tprefix, *qprefix;
	String *tsuffix;
	Iterator pi;
	Parm   *p;
	List *parms = SwigType_parmlist(e);
	tprefix = SwigType_templateprefix(e);
	tsuffix = SwigType_templatesuffix(e);
	qprefix = SwigType_typedef_qualified(tprefix);
	Printv(qprefix,"<(",NIL);
	pi = First(parms);
	while ((p = pi.item)) {
	  String *qt = SwigType_typedef_qualified(p);
	  if ((Strcmp(qt,p) == 0)) { /*  && (!Swig_scopename_check(qt))) { */
	    /* No change in value.  It is entirely possible that the parameter is an integer value.
	       If there is a symbol table associated with this scope, we're going to check for this */

	    if (current_symtab) {
	      Node *lastnode = 0;
	      String *value = Copy(p);
	      while (1) {
		Node *n = Swig_symbol_clookup(value,current_symtab);
		if (n == lastnode) break;
		lastnode = n;
		if (n) {
		  if (Strcmp(nodeType(n),"enumitem") == 0) {
		    /* An enum item.   Generate a fully qualified name */
		    String *qn = Swig_symbol_qualified(n);
		    if (Len(qn)) {
		      Append(qn,"::");
		      Append(qn,Getattr(n,"name"));
		      Delete(value);
		      value = qn;
		      continue;
		    } else {
		      Delete(qn);
		      break;
		    }
		  } else if ((Strcmp(nodeType(n),"cdecl") == 0) && (Getattr(n,"value"))) {
		    Delete(value);
		    value = Copy(Getattr(n,"value"));
		    continue;
		  }
		}
		break;
	      }
	      Append(qprefix,value);
	      Delete(value);
	    } else {
	      Append(qprefix,p);
	    }
	  } else {
	    Append(qprefix,qt);
	  }
	  Delete(qt);
	  pi= Next(pi);
	  if (pi.item) {
	    Append(qprefix,",");
	  }
	}
	Append(qprefix,")>");
	Append(qprefix,tsuffix);
	Delete(tsuffix);
	Clear(e);
	Append(e,qprefix);
	Delete(tprefix);
	Delete(qprefix);
	Delete(parms);
      }
      if (Strncmp(e,"::",2) == 0) {
	Delitem(e,0);
	Delitem(e,0);
      }
      Append(result,e);
    } else if (SwigType_isfunction(e)) {
      List *parms = SwigType_parmlist(e);
      String *s = NewString("f(");
      Iterator pi;
      pi = First(parms);
      while (pi.item) {
	String *pq = SwigType_typedef_qualified(pi.item);
	Append(s,pq);
	Delete(pq);
	pi = Next(pi);
	if (pi.item) {
	  Append(s,",");
	}
      }
      Append(s,").");
      Append(result,s);
      Delete(s);
      Delete(parms);
    } else if (SwigType_isarray(e)) {
      String *ndim;
      String *dim = SwigType_parm(e);
      ndim = Swig_symbol_string_qualify(dim,0);
      Printf(result,"a(%s).",ndim);
      Delete(dim);
      Delete(ndim);
    } else {
      Append(result,e);
    }
  }
  Delete(elements);
  {
    String *key, *cresult;
    key = NewString(t);
    cresult = NewString(result);
    Setattr(typedef_qualified_cache,key,cresult);
    Delete(key);
    Delete(cresult);
  }
  return result;
}

/* ----------------------------------------------------------------------------- 
 * SwigType_istypedef()
 *
 * Checks a typename to see if it is a typedef.
 * ----------------------------------------------------------------------------- */

int SwigType_istypedef(SwigType *t) {
  String *type;

  type = SwigType_typedef_resolve(t);
  if (type) {
    Delete(type);
    return 1;
  } else {
    return 0;
  }
}


/* -----------------------------------------------------------------------------
 * SwigType_typedef_using()
 *
 * Processes a 'using' declaration to import types from one scope into another.
 * Name is a qualified name like A::B.
 * ----------------------------------------------------------------------------- */

int SwigType_typedef_using(String_or_char *name) {
  String *base;
  String *td;
  String *prefix;
  Typetab *s;
  Typetab *tt = 0;

  String *defined_name = 0;

  /*  Printf(stdout,"using %s\n", name);*/

  if (!Swig_scopename_check(name)) return -1;     /* Not properly qualified */
  base   = Swig_scopename_last(name);

  /* See if the base is already defined in this scope */
  if (Getattr(current_typetab,base)) {
    Delete(base);
    return -1;
  }

  /* See if the using name is a scope */
  /*  tt = SwigType_find_scope(current_scope,name);
      Printf(stdout,"tt = %x, name = '%s'\n", tt, name); */

  /* We set up a typedef  B --> A::B */
  Setattr(current_typetab,base,name);

  /* Find the scope name where the symbol is defined */
  td = SwigType_typedef_resolve(name);
  /*  Printf(stdout,"td = '%s' %x\n", td, resolved_scope); */
  if (resolved_scope) {
    defined_name = Getattr(resolved_scope,"qname");
    if (defined_name) {
      defined_name = Copy(defined_name);
      Append(defined_name,"::");
      Append(defined_name,base);
    }
  }
  if (td) Delete(td);

  /*  Printf(stdout,"defined_name = '%s'\n", defined_name);*/
  tt = SwigType_find_scope(current_scope,defined_name);

  /* Figure out the scope the using directive refers to */
  {
    prefix = Swig_scopename_prefix(name);
    s = SwigType_find_scope(current_scope,prefix);
    if (s) {
      Hash *ttab = Getattr(s,"typetab");
      if (!Getattr(ttab,base) && defined_name) {
	Setattr(ttab,base, defined_name);
      }
    }
  }

  if (tt) {
    /* Using directive had it's own scope.  We need to do create a new scope for it */
    SwigType_new_scope(base);
    SwigType_inherit_scope(tt);
    SwigType_pop_scope();
  }

  if (defined_name) Delete(defined_name);
  Delete(prefix);
  Delete(base);
  return 0;
}

/* -----------------------------------------------------------------------------
 * SwigType_isclass()
 *
 * Determines if a type defines a class or not.   A class is defined by
 * its type-table entry maps to itself.  Note: a pointer to a class is not
 * a class.
 * ----------------------------------------------------------------------------- */

int
SwigType_isclass(SwigType *t) {
  SwigType *qty, *qtys;
  int  isclass = 0;

  qty = SwigType_typedef_resolve_all(t);
  qtys = SwigType_strip_qualifiers(qty);
  if (SwigType_issimple(qtys)) {
    String *td = SwigType_typedef_resolve(qtys);
    if (td) {
	Delete(td);
    }
    if (resolved_scope) {
      isclass = 1;
    }
    /* Hmmm. Not a class.  If a template, it might be uninstantiated */
    if (!isclass && SwigType_istemplate(qtys)) {
      String *tp = SwigType_templateprefix(qtys);
      if (Strcmp(tp,t) != 0) {
	isclass = SwigType_isclass(tp);
      }
      Delete(tp);
    }
  }
  Delete(qty);
  Delete(qtys);
  return isclass;
}

/* -----------------------------------------------------------------------------
 * SwigType_type()
 *
 * Returns an integer code describing the datatype.  This is only used for
 * compatibility with SWIG1.1 language modules and is likely to go away once
 * everything is based on typemaps.
 * ----------------------------------------------------------------------------- */

int SwigType_type(SwigType *t) 
{
  char *c;
  /* Check for the obvious stuff */
  c = Char(t);

  if (strncmp(c,"p.",2) == 0) {
    if (SwigType_type(c+2) == T_CHAR) return T_STRING;
    else return T_POINTER;
  }
  if (strncmp(c,"a(",2) == 0) return T_ARRAY;
  if (strncmp(c,"r.",2) == 0) return T_REFERENCE;
  if (strncmp(c,"m(",2) == 0) return T_MPOINTER;
  if (strncmp(c,"q(",2) == 0) {
    while(*c && (*c != '.')) c++;
    if (*c) return SwigType_type(c+1);
    return T_ERROR;
  }
  if (strncmp(c,"f(",2) == 0) return T_FUNCTION;
  
  /* Look for basic types */
  if (strcmp(c,"int") == 0) return T_INT;
  if (strcmp(c,"long") == 0) return T_LONG;
  if (strcmp(c,"short") == 0) return T_SHORT;
  if (strcmp(c,"unsigned") == 0) return T_UINT;
  if (strcmp(c,"unsigned short") == 0) return T_USHORT;
  if (strcmp(c,"unsigned long") == 0) return T_ULONG;
  if (strcmp(c,"unsigned int") == 0) return T_UINT;
  if (strcmp(c,"char") == 0) return T_CHAR;
  if (strcmp(c,"signed char") == 0) return T_SCHAR;
  if (strcmp(c,"unsigned char") == 0) return T_UCHAR;
  if (strcmp(c,"float") == 0) return T_FLOAT;
  if (strcmp(c,"double") == 0) return T_DOUBLE;
  if (strcmp(c,"void") == 0) return T_VOID;
  if (strcmp(c,"bool") == 0) return T_BOOL;
  if (strcmp(c,"long long") == 0) return T_LONGLONG;
  if (strcmp(c,"unsigned long long") == 0) return T_ULONGLONG;
  if (strncmp(c,"enum ",5) == 0) return T_INT;

  if (strcmp(c,"v(...)") == 0) return T_VARARGS;
  /* Hmmm. Unknown type */
  if (SwigType_istypedef(t)) {
    int r;
    SwigType *nt = SwigType_typedef_resolve(t);
    r = SwigType_type(nt);
    Delete(nt);
    return r;
  } 
  return T_USER;
}

/******************************************************************************
 ***                         * * * WARNING * * *                            ***
 ***                                                                        ***
 *** Don't even think about modifying anything below this line unless you   ***
 *** are completely on top of *EVERY* subtle aspect of the C++ type system  ***
 *** and you are prepared to suffer endless hours of agony trying to        ***
 *** debug the SWIG run-time type checker after you break it.               ***
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * SwigType_remember()
 *
 * This function "remembers" a datatype that was used during wrapper code generation
 * so that a type-checking table can be generated later on. It is up to the language
 * modules to actually call this function--it is not done automatically.
 *
 * Type tracking is managed through two separate hash tables.  The hash 'r_mangled'
 * is mapping between mangled type names (used in the target language) and 
 * fully-resolved C datatypes used in the source input.   The second hash 'r_resolved'
 * is the inverse mapping that maps fully-resolved C datatypes to all of the mangled
 * names in the scripting languages.  For example, consider the following set of
 * typedef declarations:
 *
 *      typedef double Real;
 *      typedef double Float;
 *      typedef double Point[3];
 *
 * Now, suppose that the types 'double *', 'Real *', 'Float *', 'double[3]', and
 * 'Point' were used in an interface file and "remembered" using this function.
 * The hash tables would look like this:
 *
 * r_mangled {
 *   _p_double : [ p.double, a(3).double ]
 *   _p_Real   : [ p.double ]
 *   _p_Float  : [ p.double ]
 *   _Point    : [ a(3).double ]
 *
 * r_resolved {
 *   p.double     : [ _p_double, _p_Real, _p_Float ]
 *   a(3).double  : [ _p_double, _Point ]
 * }
 *
 * Together these two hash tables can be used to determine type-equivalency between
 * mangled typenames.  To do this, we view the two hash tables as a large graph and
 * compute the transitive closure.
 * ----------------------------------------------------------------------------- */

static Hash *r_mangled = 0;                /* Hash mapping mangled types to fully resolved types */
static Hash *r_resolved = 0;               /* Hash mapping resolved types to mangled types       */
static Hash *r_ltype = 0;                  /* Hash mapping mangled names to their local c type   */
static Hash *r_clientdata = 0;             /* Hash mapping resolved types to client data         */
static Hash *r_remembered = 0;             /* Hash of types we remembered already */

static void (*r_tracefunc)(SwigType *t, String *mangled, String *clientdata) = 0;

void SwigType_remember_clientdata(SwigType *t, const String_or_char *clientdata) {
  String *mt;
  SwigType *lt;
  Hash   *h;
  SwigType *fr;
  SwigType *qr;
  String   *tkey;

  if (!r_mangled) {
    r_mangled = NewHash();
    r_resolved = NewHash();
    r_ltype = NewHash();
    r_clientdata = NewHash();
    r_remembered = NewHash();
  }

  {
    String *last;
    last = Getattr(r_remembered,t);
    if (last && (Cmp(last,clientdata) == 0)) return;
  }

  tkey = Copy(t);
  Setattr(r_remembered, tkey, clientdata ? NewString(clientdata) : (void *) "");
  Delete(tkey);

  mt = SwigType_manglestr(t);               /* Create mangled string */

  if (r_tracefunc) {
    (*r_tracefunc)(t,mt, (String *) clientdata);
  }

  if (SwigType_istypedef(t)) {
    lt = Copy(t);
  } else {
    lt = SwigType_ltype(t);
  }
  Setattr(r_ltype, mt, lt);
  fr = SwigType_typedef_resolve_all(t);     /* Create fully resolved type */
  qr = SwigType_typedef_qualified(fr);
  Delete(fr);

  /* Added to deal with possible table bug */
  fr = SwigType_strip_qualifiers(qr);
  Delete(qr);

  /*Printf(stdout,"t = '%s'\n", t);
    Printf(stdout,"fr= '%s'\n\n", fr); */
  
  if (Strstr(t,"<") && !(Strstr(t,"<("))) {
    Printf(stdout,"Bad template type passed to SwigType_remember: %s\n", t);
    assert(0);
  }

  h = Getattr(r_mangled,mt);
  if (!h) {
    h = NewHash();
    Setattr(r_mangled,mt,h);
    Delete(h);
  }
  Setattr(h,fr,mt);

  h = Getattr(r_resolved, fr);
  if (!h) {
    h = NewHash();
    Setattr(r_resolved,fr,h);
    Delete(h);
  }
  Setattr(h,mt,fr);

  if (clientdata) {
    String *cd = Getattr(r_clientdata,fr);
    if (cd) {
      if (Strcmp(clientdata,cd) != 0) {
	Printf(stderr,"*** Internal error. Inconsistent clientdata for type '%s'\n", SwigType_str(fr,0));
	Printf(stderr,"*** '%s' != '%s'\n", clientdata, cd);
	assert(0);
      } 
    } else {
      Setattr(r_clientdata, fr, NewString(clientdata));
    }
  }

  /* If the remembered type is a reference, we also remember the pointer version.
     This is to prevent odd problems with mixing pointers and references--especially
     when different functions are using different typenames (via typedef). */

  if (SwigType_isreference(t)) {
    SwigType *tt = Copy(t);
    SwigType_del_reference(tt);
    SwigType_add_pointer(tt);
    SwigType_remember_clientdata(tt,clientdata);
  }
}

void
SwigType_remember(SwigType *ty) {
  SwigType_remember_clientdata(ty,0);
}

void (*SwigType_remember_trace(void (*tf)(SwigType *, String *, String *)))(SwigType *, String *, String *) {
  void (*o)(SwigType *, String *, String *) = r_tracefunc;
  r_tracefunc = tf;
  return o;
}

/* -----------------------------------------------------------------------------
 * SwigType_equivalent_mangle()
 *
 * Return a list of all of the mangled typenames that are equivalent to another
 * mangled name.   This works as follows: For each fully qualified C datatype
 * in the r_mangled hash entry, we collect all of the mangled names from the
 * r_resolved hash and combine them together in a list (removing duplicate entries).
 * ----------------------------------------------------------------------------- */

List *SwigType_equivalent_mangle(String *ms, Hash *checked, Hash *found) {
  List *l;
  Hash *h;
  Hash *ch;
  Hash *mh;

  if (found) {
    h = found;
  } else {
    h = NewHash();
  }
  if (checked) {
    ch = checked;
  } else {
    ch = NewHash();
  }
  if (Getattr(ch,ms)) goto check_exit;    /* Already checked this type */
  Setattr(h,ms,"1");
  Setattr(ch, ms, "1");
  mh = Getattr(r_mangled,ms);
  if (mh) {
    Iterator ki;
    ki = First(mh);
    while (ki.key) {
      Hash *rh;
      if (Getattr(ch,ki.key)) {
	ki = Next(ki);
	continue;
      }
      Setattr(ch,ki.key,"1");
      rh = Getattr(r_resolved,ki.key);
      if (rh) {
	Iterator rk;
	rk = First(rh);
	while (rk.key) {
	  Setattr(h,rk.key,"1");
	  SwigType_equivalent_mangle(rk.key,ch,h);
	  rk = Next(rk);
	}
      }
      ki = Next(ki);
    }
  }
 check_exit:
  if (!found) {
    l = Keys(h);
    Delete(h);
    Delete(ch);
    return l;
  } else {
    return 0;
  }
}

/* -----------------------------------------------------------------------------
 * SwigType_clientdata_collect()
 *
 * Returns the clientdata field for a mangled type-string.
 * ----------------------------------------------------------------------------- */

static
String *SwigType_clientdata_collect(String *ms, Hash *checked) {
  Hash *ch;
  Hash *mh;
  String *clientdata = 0;

  if (checked) {
    ch = checked;
  } else {
    ch = NewHash();
  }
  if (Getattr(ch,ms)) goto check_exit;    /* Already checked this type */
  Setattr(ch, ms, "1");
  mh = Getattr(r_mangled,ms);
  if (mh) {
    Iterator ki;
    ki = First(mh);
    while (ki.key) {
      Hash *rh;
      Setattr(ch,ki.key,"1");
      clientdata = Getattr(r_clientdata,ki.key);
      if (clientdata) goto check_exit;
      rh = Getattr(r_resolved,ki.key);
      if (rh) {
	Iterator rk;
	rk = First(rh);
	while (rk.key) {
	  clientdata = SwigType_clientdata_collect(rk.key,ch);
	  if (clientdata) goto check_exit;
	  rk = Next(rk);
	}
      }
      ki = Next(ki);
    }
  }
 check_exit:
  if (!checked) {
    Delete(ch);
  }
  return clientdata;
}




/* -----------------------------------------------------------------------------
 * SwigType_inherit()
 *
 * Record information about inheritance.  We keep a hash table that keeps
 * a mapping between base classes and all of the classes that are derived
 * from them.
 *
 * subclass is a hash that maps base-classes to all of the classes derived from them.
 * ----------------------------------------------------------------------------- */

static Hash   *subclass = 0;
static Hash   *conversions = 0;

void
SwigType_inherit(String *derived, String *base, String *cast) {
  Hash *h;
  if (!subclass) subclass = NewHash();
  
  /* Printf(stdout,"'%s' --> '%s'  '%s'\n", derived, base, cast); */

  if (SwigType_istemplate(derived)) {
    derived = SwigType_typedef_qualified(SwigType_typedef_resolve_all(derived));
  }
  if (SwigType_istemplate(base)) {
    base = SwigType_typedef_qualified(SwigType_typedef_resolve_all(base));
  }

  /* Printf(stdout,"'%s' --> '%s'  '%s'\n", derived, base, cast);*/

  h = Getattr(subclass,base);
  if (!h) {
    h = NewHash();
    Setattr(subclass,base,h);
  }
  if (!Getattr(h,derived)) {
    Setattr(h,derived, cast ? cast : (void *) "");
  }

}

/* -----------------------------------------------------------------------------
 * SwigType_issubtype()
 *
 * Determines if a t1 is a subtype of t2
 * ----------------------------------------------------------------------------- */

int
SwigType_issubtype(SwigType *t1, SwigType *t2) {
  SwigType *ft1, *ft2;
  String   *b1, *b2;
  Hash     *h;
  int       r = 0;

  if (!subclass) return 0;

  ft1 = SwigType_typedef_resolve_all(t1);
  ft2 = SwigType_typedef_resolve_all(t2);
  b1 = SwigType_base(ft1);
  b2 = SwigType_base(ft2);
  
  h = Getattr(subclass,b2);
  if (h) {
    if (Getattr(h,b1)) {
      r = 1;
    }
  }
  Delete(ft1);
  Delete(ft2);
  Delete(b1);
  Delete(b2);
  /* Printf(stdout, "issubtype(%s,%s) --> %d\n", t1, t2, r); */
  return r;
}

/* -----------------------------------------------------------------------------
 * SwigType_inherit_equiv()
 *
 * Modify the type table to handle C++ inheritance
 * ----------------------------------------------------------------------------- */

void SwigType_inherit_equiv(File *out) {
  String *ckey;
  String *prefix, *base;
  Hash   *sub;
  Hash   *rh;
  List   *rlist;
  Iterator rk, bk, ck;

  if (!conversions) conversions = NewHash();
  if (!subclass) subclass = NewHash();

  rk = First(r_resolved);
  while (rk.key) {
    /* rkey is a fully qualified type.  We strip all of the type constructors off of it just to get the base */
    base = SwigType_base(rk.key);
    /* Check to see whether the base is recorded in the subclass table */
    sub = Getattr(subclass,base);
    Delete(base);
    if (!sub) {
      rk = Next(rk);
      continue;
    }

    /* This type has subclasses.  We now need to walk through these subtypes and generate pointer converion functions */

    rh = Getattr(r_resolved, rk.key);
    rlist = NewList();
    for (ck = First(rh); ck.key; ck = Next(ck)) {
      Append(rlist,ck.key);
    }
    /*    Printf(stdout,"rk.key = '%s'\n", rk.key);
	  Printf(stdout,"rh = %x '%s'\n", rh,rh); */

    bk = First(sub);
    while (bk.key) {
      prefix= SwigType_prefix(rk.key);
      Append(prefix,bk.key);
      /*      Printf(stdout,"set %x = '%s' : '%s'\n", rh, SwigType_manglestr(prefix),prefix); */
      Setattr(rh,SwigType_manglestr(prefix),prefix);
      ckey = NewStringf("%s+%s",SwigType_manglestr(prefix), SwigType_manglestr(rk.key));
      if (!Getattr(conversions,ckey)) {
	String *convname = NewStringf("%sTo%s", SwigType_manglestr(prefix), SwigType_manglestr(rk.key));
	Printf(out,"static void *%s(void *x) {\n", convname);
	Printf(out,"    return (void *)((%s) %s ((%s) x));\n", SwigType_lstr(rk.key,0), Getattr(sub,bk.key), SwigType_lstr(prefix,0));
	Printf(out,"}\n");
	Setattr(conversions,ckey,convname);
	Delete(ckey);	

	/* This inserts conversions for typedefs */
	{
	  Hash *r = Getattr(r_resolved, prefix);
	  if (r) {
	    Iterator rrk;
	    rrk=First(r);
	    while (rrk.key) {
	      Iterator rlk;
	      String *rkeymangle;

	      /* Make sure this name equivalence is not due to inheritance */
	      if (Cmp(prefix, Getattr(r,rrk.key)) == 0) {
		rkeymangle = SwigType_manglestr(rk.key);
		ckey = NewStringf("%s+%s", rrk.key, rkeymangle);
		if (!Getattr(conversions, ckey)) {
		  Setattr(conversions, ckey, convname);
		}
		Delete(ckey);
		for (rlk = First(rlist); rlk.item; rlk = Next(rlk)) {
		  ckey = NewStringf("%s+%s", rrk.key, rlk.item);
		  Setattr(conversions, ckey, convname);
		  Delete(ckey);
		}
		Delete(rkeymangle);
		/* This is needed to pick up other alternative names for the same type.
                   Needed to make templates work */
	        Setattr(rh,rrk.key,rrk.item);   
	      }
	      rrk = Next(rrk);
	    }
	  }
	}
	Delete(convname);
      }
      Delete(prefix);
      bk = Next(bk);
    }
    rk = Next(rk);
  }
}

/* -----------------------------------------------------------------------------
 * SwigType_type_table()
 *
 * Generate the type-table for the type-checker.
 * ----------------------------------------------------------------------------- */

void
SwigType_emit_type_table(File *f_forward, File *f_table) {
  Iterator ki;
  String *types, *table;
  int i = 0;

  if (!r_mangled) {
    r_mangled = NewHash();
    r_resolved = NewHash();
  }

  Printf(f_table,"\n/* -------- TYPE CONVERSION AND EQUIVALENCE RULES (BEGIN) -------- */\n\n");

  SwigType_inherit_equiv(f_table);

  /* #define DEBUG 1 */
#ifdef DEBUG
  Printf(stdout,"---r_mangled---\n");
  Printf(stdout,"%s\n", r_mangled);
  
  Printf(stdout,"---r_resolved---\n");
  Printf(stdout,"%s\n", r_resolved);

  Printf(stdout,"---r_ltype---\n");
  Printf(stdout,"%s\n", r_ltype);

  Printf(stdout,"---subclass---\n");
  Printf(stdout,"%s\n", subclass);

  Printf(stdout,"---conversions---\n");
  Printf(stdout,"%s\n", conversions);

#endif
  table = NewString("");
  types = NewString("");
  Printf(table,"static swig_type_info *swig_types_initial[] = {\n");

  ki = First(r_mangled);
  Printf(f_forward,"\n/* -------- TYPES TABLE (BEGIN) -------- */\n\n");
  while (ki.key) {
    List *el;
    Iterator ei;
    String *cd;

    Printf(f_forward,"#define  SWIGTYPE%s swig_types[%d] \n", ki.key, i);
    Printv(types,"static swig_type_info _swigt_", ki.key, "[] = {", NIL);
    
    cd = SwigType_clientdata_collect(ki.key,0);
    if (!cd) cd = "0";
    Printv(types,"{\"", ki.key, "\", 0, \"", SwigType_str(Getattr(r_ltype,ki.key),0),"\", ", cd, "},", NIL);
    el = SwigType_equivalent_mangle(ki.key,0,0);
    for (ei = First(el); ei.item; ei = Next(ei)) {
      String *ckey;
      String *conv;
      ckey = NewStringf("%s+%s", ei.item, ki.key);
      conv = Getattr(conversions,ckey);
      if (conv) {
	Printf(types,"{\"%s\", %s},", ei.item, conv);
      } else {
	Printf(types,"{\"%s\"},", ei.item);
      }
      Delete(ckey);
    }
    Delete(el);
    Printf(types,"{0}};\n");
    Printv(table, "_swigt_", ki.key, ", \n", NIL);
    ki = Next(ki);
    i++;
  }

  Printf(table, "0\n};\n");
  Printf(f_forward,"static swig_type_info *swig_types[%d];\n", i+1);
  Printf(f_forward,"\n/* -------- TYPES TABLE (END) -------- */\n\n");
  Printf(f_table,"%s\n", types);
  Printf(f_table,"%s\n", table);
  Printf(f_table,"\n/* -------- TYPE CONVERSION AND EQUIVALENCE RULES (END) -------- */\n\n");
  Delete(types);
  Delete(table);
}
