/* ----------------------------------------------------------------------------- 
 * allocate.cxx
 *
 *     This module tries to figure out which classes and structures support
 *     default constructors and destructors in C++.   There are several rules that
 *     define this behavior including pure abstract methods, private sections,
 *     and non-default constructors in base classes.  See the ARM or
 *     Doc/Manual/SWIGPlus.html for details.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1998-2002.  The University of Chicago
 * Copyright (C) 1995-1998.  The University of Utah and The Regents of the
 *                           University of California.
 *
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

char cvsroot_allocate_cxx[] = "$Header$";

#include "swigmod.h"
static int virtual_elimination_mode = 0;   /* set to 0 on default */

/* Set virtual_elimination_mode */
void Wrapper_virtual_elimination_mode_set(int flag) {
  virtual_elimination_mode = flag;
}

/* Helper function to assist with abstract class checking.  
   This is a major hack. Sorry.  */

static String *search_decl = 0;            /* Declarator being searched */ 
static int check_implemented(Node *n) {
  String *local_decl;
  if (!n) return 0;
  while (n) {
    if (Strcmp(nodeType(n), "cdecl") == 0) {
      local_decl = Getattr(n,"decl");
      if (SwigType_isfunction(local_decl)) {
	SwigType *decl1 = SwigType_typedef_resolve_all(local_decl);
	SwigType *decl2 = SwigType_pop_function(decl1);
	if (Strcmp(decl2, search_decl) == 0) {
	  if (!Getattr(n,"abstract")) {
	    Delete(decl1);
	    Delete(decl2);
	    return 1;
	  }
	}
	Delete(decl1);
	Delete(decl2);
      }
    }
    n = Getattr(n,"csym:nextSibling");
  }
  return 0;
}


class Allocate : public Dispatcher {
  Node  *inclass;
  enum AccessMode { PUBLIC, PRIVATE, PROTECTED };
  AccessMode cplus_mode;
  int extendmode;

  /* Checks if a virtual function is the same as inherited from the bases */
  int function_is_defined_in_bases(Node *c, Node *bases) {
    int i;
    Node *b, *temp;
    String *name, *type, *local_decl, *base_decl;
    if (!bases)
      return 0;
    
    name = Getattr(c, "name");
    type = Getattr(c, "type");
    local_decl = Getattr(c, "decl");
    if (local_decl) {
      local_decl = SwigType_typedef_resolve_all(local_decl);
    } else {
      return 0;
    }

    /* Width first search */
    for (int i = 0; i < Len(bases); i++) {
      b = Getitem(bases,i);
      temp = firstChild (b);
      while (temp) {
	base_decl = Getattr(temp, "decl");
	if (base_decl) {
	  base_decl = SwigType_typedef_resolve_all(base_decl);
	  
	  if ( (checkAttribute(temp, "storage", "virtual")) &&
	       (checkAttribute(temp, "name", name)) &&
	       (checkAttribute(temp, "type", type)) &&
	       (!Strcmp(local_decl, base_decl)) ) {
	    Setattr(c, "Tiger: defined_in_base", "1");
	    Setattr(c, "feature:ignore", "1");
	    Delete(base_decl);
	    Delete(local_decl);
	    return 1;
	  }
	  Delete(base_decl);
	}
	temp = nextSibling(temp);
      }
    }
    Delete(local_decl);
    for (int i = 0; i < Len(bases); i++) {
      b = Getitem(bases,i);
      if (function_is_defined_in_bases(c, Getattr(b, "bases")))
	return 1;
    }
    return 0;
  }

  /* Checks if a class has the same virtual functions as the bases have */
  int class_is_defined_in_bases(Node *n) {
    Node *c, *bases; /* bases is the closest ancestors of class n */
    int defined = 0;

    bases = Getattr(n, "bases");

    if (!bases) return 0;

    c = firstChild(n); /* c is the members of class n */
    while (c) {
      if (checkAttribute(c, "storage", "virtual")) {
	if (function_is_defined_in_bases(c, bases))
	  defined = 1;
      }
      c = nextSibling(c);
    }
    
    if (defined)
      return 1;
    else return 0;
  }

  /* Checks if a class member is the same as inherited from the class bases */
  int class_member_is_defined_in_bases(Node *member, Node *classnode) {
    Node *bases; /* bases is the closest ancestors of classnode */
    int defined = 0;

    bases = Getattr(classnode, "bases");
    if (!bases) return 0;

    if (checkAttribute(member, "storage", "virtual")) {
      if (function_is_defined_in_bases(member, bases))
	defined = 1;
    }

    if (defined)
      return 1;
    else return 0;
  }

  /* Checks to see if a class is abstract through inheritance */
  int is_abstract_inherit(Node *n, Node *base = 0, int first = 0) {
    if (!first && (base == n)) return 0;
    if (!base) {
      /* Root node */
      Symtab *stab = Getattr(n,"symtab");         /* Get symbol table for node */
      Symtab *oldtab = Swig_symbol_setscope(stab);
      int ret = is_abstract_inherit(n,n,1);
      Swig_symbol_setscope(oldtab);
      return ret;
    }
    List *abstract = Getattr(base,"abstract");
    if (abstract) {
      for (int i = 0; i < Len(abstract); i++) {
	Node *nn = Getitem(abstract,i);
	String *name = Getattr(nn,"name");
	String *base_decl = Getattr(nn,"decl");
	if (base_decl) base_decl = SwigType_typedef_resolve_all(base_decl);
	if (Strstr(name,"~")) continue;   /* Don't care about destructors */
	int implemented = 0;
	/*	Node *dn = Swig_symbol_clookup_local(name,0);
	if (!dn) {
	  Printf(stdout,"node: %x '%s'. base: %x '%s'. member '%s'\n", n, Getattr(n,"name"), base, Getattr(base,"name"), name);
	}
	assert(dn != 0);   // Assertion of doom
	*/
	if (SwigType_isfunction(base_decl)) {
	  search_decl = SwigType_pop_function(base_decl);
	}
	Node *dn = Swig_symbol_clookup_local_check(name,0,check_implemented);
	Delete(search_decl);
	Delete(base_decl);
	/*
	while (dn && !implemented) {
	  String *local_decl = Getattr(dn,"decl");
	  if (local_decl) local_decl = SwigType_typedef_resolve_all(local_decl);
	  if (local_decl && !Strcmp(local_decl, base_decl)) {
	    if (Getattr(dn,"abstract")) return 1;
	    implemented++;
	  }
	  Delete(local_decl);
	  dn = Getattr(dn,"csym:nextSibling");
	}
	*/



	/*	if (!implemented && (Getattr(nn,"abstract"))) {
	  return 1;
	}
	*/

	if (!dn) return 1;
	/*
	if (dn && (Getattr(dn,"abstract"))) {
	  return 1;
	} 
	*/
      }
    }
    List *bases = Getattr(base,"bases");
    if (!bases) return 0;
    for (int i = 0; i < Len(bases); i++) {
      if (is_abstract_inherit(n,Getitem(bases,i))) {
	return 1;
      }
    }
    return 0;
  }


  /* Grab methods used by smart pointers */

  List *smart_pointer_methods(Node *cls, List *methods) {
    if (!methods) {
      methods = NewList();
    }
    
    Node *c = firstChild(cls);
    String *kind = Getattr(cls,"kind");
    int mode;
    if (Strcmp(kind,"class") == 0) mode = PRIVATE;
    else mode = PUBLIC;
    
    while (c) {
      if (Getattr(c,"error") || Getattr(c,"feature:ignore")) {
	c = nextSibling(c);
	continue;
      }
      if (Strcmp(nodeType(c),"cdecl") == 0) {
	if (!Getattr(c,"feature:ignore")) {
	  String *storage = Getattr(c,"storage");
	  if (!((Cmp(storage,"static") == 0) || (Cmp(storage,"typedef") == 0))) {
	    String *name = Getattr(c,"name");
	    String *symname = Getattr(c,"sym:name");
	    Node   *e    = Swig_symbol_clookup_local(name,0);
	    if (e && !Getattr(e,"feature:ignore") && (Cmp(symname, Getattr(e,"sym:name")) == 0)) {
	      Swig_warning(WARN_LANG_DEREF_SHADOW,Getfile(e),Getline(e),"Declaration of '%s' shadows declaration accessible via operator->() at %s:%d\n",
			   name, Getfile(c),Getline(c));
	    } else {
	      /* Make sure node with same name doesn't already exist */
	      int k;
	      int match = 0;
	      for (k = 0; k < Len(methods); k++) {
		e = Getitem(methods,k);
		if (Cmp(symname,Getattr(e,"sym:name")) == 0) {
		  match = 1;
		  break;
		}
		if ((!symname  || (!Getattr(e,"sym:name"))) && (Cmp(name,Getattr(e,"name")) == 0)) {
		  match = 1;
		  break;
		}
	      }
	      if (!match) {
		Node *cc = c;
		while (cc) {
		  Append(methods,cc);
		  cc = Getattr(cc,"sym:nextSibling");
		}
	      }
	    }
	  }
	}
      }

      if (Strcmp(nodeType(c),"access") == 0) {
	kind = Getattr(c,"kind");
	if (Strcmp(kind,"public") == 0) mode = PUBLIC;
	else mode = PRIVATE;
      }
      c = nextSibling(c);
    }
    /* Look for methods in base classes */
    {
      Node *bases = Getattr(cls,"bases");
      int k;
      for (k = 0; k < Len(bases); k++) {
	smart_pointer_methods(Getitem(bases,k),methods);
      }
    }
    /* Remove protected/private members */
    {
      for (int i = 0; i < Len(methods); ) {
	Node *n = Getitem(methods,i);
	if (checkAttribute(n,"access","protected") || checkAttribute(n,"access","private")) {
	  Delitem(methods,i);
	  continue;
	}
	i++;
      }
    }
    return methods;
  }

  void mark_exception_classes(ParmList *p) {
    while(p) {
      SwigType *ty = Getattr(p,"type");
      SwigType *t = SwigType_typedef_resolve_all(ty);
      Node *c = Swig_symbol_clookup(t,0);
      if (c) {
	Setattr(c,"cplus:exceptionclass","1");
      }
      p = nextSibling(p);
    }
  }

public:
  virtual int top(Node *n) {
    cplus_mode = PUBLIC;
    inclass = 0;
    extendmode = 0;
    emit_children(n);
    return SWIG_OK;
  }

  virtual int importDirective(Node *n) { return emit_children(n); }
  virtual int includeDirective(Node *n) { return emit_children(n); }
  virtual int externDeclaration(Node *n) { return emit_children(n); }
  virtual int namespaceDeclaration(Node *n) { return emit_children(n); }
  virtual int extendDirective(Node *n) {
      extendmode = 1;
      emit_children(n);
      extendmode = 0;
      return SWIG_OK;
  }

  virtual int classDeclaration(Node *n) {
    Symtab *symtab = Swig_symbol_current();
    Swig_symbol_setscope(Getattr(n,"symtab"));

    if (!CPlusPlus) {
      /* Always have default constructors/destructors in C */
      Setattr(n,"allocate:default_constructor","1");
      Setattr(n,"allocate:default_destructor","1");
    }

    if (Getattr(n,"allocate:visit")) return SWIG_OK;
    Setattr(n,"allocate:visit","1");
    
    /* Always visit base classes first */
    {
      List *bases = Getattr(n,"bases");
      if (bases) {
	for (int i = 0; i < Len(bases); i++) {
	  Node *b = Getitem(bases,i);
	  classDeclaration(b);
	}
      }
    }

    inclass = n;
    String *kind = Getattr(n,"kind");
    if (Strcmp(kind,"class") == 0) {
      cplus_mode = PRIVATE;
    } else {
      cplus_mode = PUBLIC;
    }

    emit_children(n);

    /* Check if the class is abstract via inheritance.   This might occur if a class didn't have
       any pure virtual methods of its own, but it didn't implement all of the pure methods in
       a base class */

    if (is_abstract_inherit(n)) {
      if ((!Getattr(n,"abstract")) && ((Getattr(n,"allocate:public_constructor") || (!Getattr(n,"feature:nodefault") && !Getattr(n,"allocate:has_constructor"))))) {
	if (!Getattr(n,"feature:notabstract")) {
	  Swig_warning(WARN_TYPE_ABSTRACT,Getfile(n),Getline(n),"Class '%s' might be abstract. No constructors generated. \n", SwigType_namestr(Getattr(n,"name")));
	  Setattr(n,"abstract",NewList());
	}
      }
    }

    if (!Getattr(n,"allocate:has_constructor")) {
      /* No constructor is defined.  We need to check a few things */
      /* If class is abstract.  No default constructor. Sorry */
      if (Getattr(n,"abstract")) {
	Delattr(n,"allocate:default_constructor");
      } 
      if (!Getattr(n,"allocate:default_constructor")) {
	/* Check base classes */
	List *bases = Getattr(n,"bases");
	int   allows_default = 1;
	
	for (int i = 0; i < Len(bases); i++) {
	  Node *n = Getitem(bases,i);
	  /* If base class does not allow default constructor, we don't allow it either */
	  if (!Getattr(n,"allocate:default_constructor") && (!Getattr(n,"allocate:default_base_constructor"))) {
	    allows_default = 0;
	  }
	}
	if (allows_default) {
	  Setattr(n,"allocate:default_constructor","1");
	}
      }
    }
    if (!Getattr(n,"allocate:has_destructor")) {
      /* No destructor was defined.  We need to check a few things here too */
      List *bases = Getattr(n,"bases");
      int allows_destruct = 1;

      for (int i = 0; i < Len(bases); i++) {
	Node *n = Getitem(bases,i);
	/* If base class does not allow default destructor, we don't allow it either */
	if (!Getattr(n,"allocate:default_destructor") && (!Getattr(n,"allocate:default_base_destructor"))) {
	  allows_destruct = 0;
	}
      }
      if (allows_destruct) {
	Setattr(n,"allocate:default_destructor","1");
      }
    }


    /* Check if base classes allow smart pointers, but might be hidden */
    if (!Getattr(n,"allocate:smartpointer")) {
      Node *sp = Swig_symbol_clookup((char*)"operator ->",0);
      if (sp) {
        /* Look for parent */
	Node *p = parentNode(sp);
	if (Strcmp(nodeType(p),"extend") == 0) {
	  p = parentNode(p);
	}
	if (Strcmp(nodeType(p),"class") == 0) {
	  if (Getattr(p,"feature:ignore")) {
	    Setattr(n,"allocate:smartpointer",Getattr(p,"allocate:smartpointer"));
	  }
	}
      }
    }

    /* Only care about default behavior.  Remove temporary values */
    Setattr(n,"allocate:visit","1");
    inclass = 0;
    Swig_symbol_setscope(symtab);
    return SWIG_OK;
  }

  virtual int accessDeclaration(Node *n) {
    String *kind = Getattr(n,"kind");
    if (Cmp(kind,"public") == 0) {
      cplus_mode = PUBLIC;
    } else if (Cmp(kind,"private") == 0) {
      cplus_mode = PRIVATE;
    } else if (Cmp(kind,"protected") == 0) {
      cplus_mode = PROTECTED;
    }
    return SWIG_OK;
  }

  virtual int cDeclaration(Node *n) {
    
    mark_exception_classes(Getattr(n,"throws"));

    if (inclass) {
      /* check whether the member node n is defined in class node inclass's bases */
      if (virtual_elimination_mode)
	if (checkAttribute(n, "storage", "virtual"))
	  class_member_is_defined_in_bases(n, inclass);

      String *name = Getattr(n,"name");
      if (cplus_mode != PUBLIC) {
	/* Look for a private assignment operator */
	if (Strcmp(name,"operator =") == 0) {
	  Setattr(inclass,"allocate:noassign","1");
	}
      } else {
	/* Look for smart pointer operator */
	if ((Strcmp(name,"operator ->") == 0) && (!Getattr(n,"feature:ignore"))) {
	  /* Look for version with no parameters */
	  Node *sn = n;
	  while (sn) {
	    if (!Getattr(sn,"parms")) {
	      SwigType *type = SwigType_typedef_resolve_all(Getattr(sn,"type"));
	      SwigType_push(type,Getattr(sn,"decl"));
	      Delete(SwigType_pop_function(type));
	      SwigType *base = SwigType_base(type);
	      Node *sc = Swig_symbol_clookup(base, 0);
	      if ((sc) && (Strcmp(nodeType(sc),"class") == 0)) {
		if (SwigType_check_decl(type,"p.")) {
		  List *methods = smart_pointer_methods(sc,0);
		  Setattr(inclass,"allocate:smartpointer",methods);
		  break;
		} else {
		  /* Hmmm.  The return value is not a pointer.  If the type is a value
		     or reference.  We're going to chase it to see if another operator->()
		     can be found */
		  
		  if ((SwigType_check_decl(type,"")) || (SwigType_check_decl(type,"r."))) {
		    Node *nn = Swig_symbol_clookup((char*)"operator ->", Getattr(sc,"symtab"));
		    if (nn) {
		      sn = nn;
		      continue;
		    } else {
		      break;
		    }
		  } else {
		    break;
		  }
		}
	      } else {
		break;
	      }
	    } else {
	      break;
	    }
	  }
	}
      }
    }
    return SWIG_OK;
  }

  virtual int constructorDeclaration(Node *n) {
    if (!inclass) return SWIG_OK;
    Parm   *parms = Getattr(n,"parms");

    mark_exception_classes(Getattr(n,"throws"));
    if (!extendmode) {
	if (!ParmList_numrequired(parms)) {
	    /* Class does define a default constructor */
	    /* However, we had better see where it is defined */
	    if (cplus_mode == PUBLIC) {
		Setattr(inclass,"allocate:default_constructor","1");
	    } else if (cplus_mode == PROTECTED) {
		Setattr(inclass,"allocate:default_base_constructor","1");
	    }
	}
	/* Class defines some kind of constructor. May or may not be public */
	Setattr(inclass,"allocate:has_constructor","1");
	if (cplus_mode == PUBLIC) {
	  Setattr(inclass,"allocate:public_constructor","1");
	}
    }

    /* See if this is a copy constructor */
    if (parms && (ParmList_numrequired(parms) == 1)) {
      /* Look for a few cases. X(const X &), X(X &), X(X *) */

      String *cc = NewStringf("r.q(const).%s", Getattr(inclass,"name"));
      if (Strcmp(cc,Getattr(parms,"type")) == 0) {
	Setattr(n,"copy_constructor","1");
      }
      Delete(cc);
      cc = NewStringf("r.%s", Getattr(inclass,"name"));
      if (Strcmp(cc,Getattr(parms,"type")) == 0) {
	Setattr(n,"copy_constructor","1");
      }
      Delete(cc);
      cc = NewStringf("p.%s", Getattr(inclass,"name"));
      String *ty = SwigType_strip_qualifiers(Getattr(parms,"type"));
      if (Strcmp(cc,ty) == 0) {
	Setattr(n,"copy_constructor","1");
      }
      Delete(cc);
      Delete(ty);
    }
    return SWIG_OK;
  }

  virtual int destructorDeclaration(Node *n) {
    if (!inclass) return SWIG_OK;
    if (!extendmode) {
	Setattr(inclass,"allocate:has_destructor","1");
	if (cplus_mode == PUBLIC) {
	    Setattr(inclass,"allocate:default_destructor","1");
	} else if (cplus_mode == PROTECTED) {
	    Setattr(inclass,"allocate:default_base_destructor","1");
	}
    }
    return SWIG_OK;
  }
};

void Swig_default_allocators(Node *n) {
  if (!n) return;
  Allocate *a = new Allocate;
  a->top(n);
  delete a;
}





  
