/* ----------------------------------------------------------------------------- 
 * naming.c
 *
 *     Functions for generating various kinds of names during code generation
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

char cvsroot_naming_c[] = "$Header$";

#include "swig.h"
#include <ctype.h>

/* Hash table containing naming data */

static Hash *naming_hash = 0;

/* #define SWIG_DEBUG  */

/* -----------------------------------------------------------------------------
 * Swig_naming_init()
 *
 * Init the naming system
 * ----------------------------------------------------------------------------- */
static String *k_construct = 0;
static String *k_constructor = 0;
static String *k_destroy=  0;
static String *k_destructor = 0;
static String *k_disown = 0;
static String *k_name = 0;
static String *k_start = 0;
static String *k_value = 0;
static String *k_wrapper = 0;
static String *k_nodetype = 0;
static String *k_member = 0;
static String *k_get = 0;
static String *k_set = 0;

void
Swig_naming_init() {
  k_construct = NewString("construct");
  k_constructor = NewString("constructor");
  k_destroy = NewString("destroy");
  k_destructor = NewString("destructor");
  k_disown = NewString("disown");
  k_name = NewString("name");
  k_nodetype = NewString("nodeType");
  k_start = NewString("*");
  k_value = NewString("value");
  k_wrapper = NewString("wrapper");
  k_member = NewString("member");
  k_set = NewString("set");
  k_get = NewString("get");
}


/* -----------------------------------------------------------------------------
 * Swig_name_register()
 *
 * Register a new naming format.
 * ----------------------------------------------------------------------------- */

void
Swig_name_register(const String_or_char *method, const String_or_char *format) {
  if (!naming_hash) naming_hash = NewHash();
  Setattr(naming_hash,method,format);
}

void
Swig_name_unregister(const String_or_char *method) {
  if (naming_hash) {
    Delattr(naming_hash,method);
  }
}

static int name_mangle(String *r) {
  char  *c;
  int    special;
  special = 0;
  Replaceall(r,"::","_");
  c = Char(r);
  while (*c) {
    if (!isalnum((int) *c) && (*c != '_')) {
      special = 1;
      switch(*c) {
      case '+':
	*c = 'a';
	break;
      case '-':
	*c = 's';
	break;
      case '*':
	*c = 'm';
	break;
      case '/':
	*c = 'd';
	break;
      case '<':
	*c = 'l';
	break;
      case '>':
	*c = 'g';
	break;
      case '=':
	*c = 'e';
	break;
      case ',':
	*c = 'c';
	break;
      case '(':
	*c = 'p';
	break;
      case ')':
	*c = 'P';
	break;
      case '[':
	*c = 'b';
	break;
      case ']':
	*c = 'B';
	break;
      case '^':
	*c = 'x';
	break;
      case '&':
	*c = 'A';
	break;
      case '|':
	*c = 'o';
	break;
      case '~':
	*c = 'n';
	break;
      case '!':
	*c = 'N';
	break;
      case '%':
	*c = 'M';
	break;
      case '.':
	*c = 'f';
	break;
      case '?':
	*c = 'q';
	break;
      default:
	*c = '_';
	break;
      }
    }
    c++;
  }
  if (special) Append(r,"___");
  return special;
}

/* -----------------------------------------------------------------------------
 * Swig_name_mangle()
 *
 * Converts all of the non-identifier characters of a string to underscores.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_mangle(const String_or_char *s) {
#if 0
  String *r = NewString(s);
  name_mangle(r);
  return r;
#else
  return Swig_string_mangle(s);
#endif
}

/* -----------------------------------------------------------------------------
 * Swig_name_wrapper()
 *
 * Returns the name of a wrapper function.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_wrapper(const String_or_char *fname) {
  String *r;
  String *f;

  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_wrapper);
  if (!f) {
    Append(r,"_wrap_%f");
  } else {
    Append(r,f);
  }
  Replace(r,"%f",fname, DOH_REPLACE_ANY);
  name_mangle(r);
  return r;
}


/* -----------------------------------------------------------------------------
 * Swig_name_member()
 *
 * Returns the name of a class method.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_member(const String_or_char *classname, const String_or_char *mname) {
  String *r;
  String *f;
  String *rclassname;
  char   *cname;

  rclassname = SwigType_namestr(classname);
  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_member);
  if (!f) {
    Append(r,"%c_%m");
  } else {
    Append(r,f);
  }
  cname = Char(rclassname);
  if ((strncmp(cname,"struct ", 7) == 0) ||
      ((strncmp(cname,"class ", 6) == 0)) ||
      ((strncmp(cname,"union ", 6) == 0))) {
    cname = strchr(cname, ' ')+1;
  }
  Replace(r,"%c",cname, DOH_REPLACE_ANY);
  Replace(r,"%m",mname, DOH_REPLACE_ANY);
  /*  name_mangle(r);*/
  Delete(rclassname);
  return r;
}

/* -----------------------------------------------------------------------------
 * Swig_name_get()
 *
 * Returns the name of the accessor function used to get a variable.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_get(const String_or_char *vname) {
  String *r;
  String *f;

#ifdef SWIG_DEBUG
  Printf(stdout,"Swig_name_get:  '%s'\n", vname); 
#endif

  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_get);
  if (!f) {
    Append(r,"%v_get");
  } else {
    Append(r,f);
  }
  Replace(r,"%v",vname, DOH_REPLACE_ANY);
  /* name_mangle(r); */
  return r;
}

/* ----------------------------------------------------------------------------- 
 * Swig_name_set()
 *
 * Returns the name of the accessor function used to set a variable.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_set(const String_or_char *vname) {
  String *r;
  String *f;

  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_set);
  if (!f) {
    Append(r,"%v_set");
  } else {
    Append(r,f);
  }
  Replace(r,"%v",vname, DOH_REPLACE_ANY);
  /* name_mangle(r); */
  return r;
}

/* -----------------------------------------------------------------------------
 * Swig_name_construct()
 *
 * Returns the name of the accessor function used to create an object.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_construct(const String_or_char *classname) {
  String *r;
  String *f;
  String *rclassname;
  char *cname;

  rclassname = SwigType_namestr(classname);
  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_construct);
  if (!f) {
    Append(r,"new_%c");
  } else {
    Append(r,f);
  }

  cname = Char(rclassname);
  if ((strncmp(cname,"struct ", 7) == 0) ||
      ((strncmp(cname,"class ", 6) == 0)) ||
      ((strncmp(cname,"union ", 6) == 0))) {
    cname = strchr(cname, ' ')+1;
  }
  Replace(r,"%c",cname, DOH_REPLACE_ANY);
  Delete(rclassname);
  return r;
}


/* -----------------------------------------------------------------------------
 * Swig_name_copyconstructor()
 *
 * Returns the name of the accessor function used to copy an object.
 * ----------------------------------------------------------------------------- */

String *
Swig_name_copyconstructor(const String_or_char *classname) {
  String *r;
  String *f;
  String *rclassname;
  char *cname;

  rclassname = SwigType_namestr(classname);
  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_construct);
  if (!f) {
    Append(r,"copy_%c");
  } else {
    Append(r,f);
  }

  cname = Char(rclassname);
  if ((strncmp(cname,"struct ", 7) == 0) ||
      ((strncmp(cname,"class ", 6) == 0)) ||
      ((strncmp(cname,"union ", 6) == 0))) {
    cname = strchr(cname, ' ')+1;
  }

  Replace(r,"%c",cname, DOH_REPLACE_ANY);
  Delete(rclassname);
  return r;
}

/* -----------------------------------------------------------------------------
 * Swig_name_destroy()
 *
 * Returns the name of the accessor function used to destroy an object.
 * ----------------------------------------------------------------------------- */

String *Swig_name_destroy(const String_or_char *classname) {
  String *r;
  String *f;
  String *rclassname;
  char *cname;
  rclassname = SwigType_namestr(classname);
  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_destroy);
  if (!f) {
    Append(r,"delete_%c");
  } else {
    Append(r,f);
  }

  cname = Char(rclassname);
  if ((strncmp(cname,"struct ", 7) == 0) ||
      ((strncmp(cname,"class ", 6) == 0)) ||
      ((strncmp(cname,"union ", 6) == 0))) {
    cname = strchr(cname, ' ')+1;
  }
  Replace(r,"%c",cname, DOH_REPLACE_ANY);
  Delete(rclassname);
  return r;
}


/* -----------------------------------------------------------------------------
 * Swig_name_disown()
 *
 * Returns the name of the accessor function used to disown an object.
 * ----------------------------------------------------------------------------- */

String *Swig_name_disown(const String_or_char *classname) {
  String *r;
  String *f;
  String *rclassname;
  char *cname;
  rclassname = SwigType_namestr(classname);
  r = NewStringEmpty();
  if (!naming_hash) naming_hash = NewHash();
  f = Getattr(naming_hash,k_disown);
  if (!f) {
    Append(r,"disown_%c");
  } else {
    Append(r,f);
  }

  cname = Char(rclassname);
  if ((strncmp(cname,"struct ", 7) == 0) ||
      ((strncmp(cname,"class ", 6) == 0)) ||
      ((strncmp(cname,"union ", 6) == 0))) {
    cname = strchr(cname, ' ')+1;
  }
  Replace(r,"%c",cname, DOH_REPLACE_ANY);
  Delete(rclassname);
  return r;
}


/* -----------------------------------------------------------------------------
 * Swig_name_object_set()
 *
 * Sets an object associated with a name and optional declarators. 
 * ----------------------------------------------------------------------------- */

void 
Swig_name_object_set(Hash *namehash, String *name, SwigType *decl, DOH *object) {
  DOH *n;

#ifdef SWIG_DEBUG
  Printf(stdout,"Swig_name_object_set:  '%s', '%s'\n", name, decl); 
#endif
  n = Getattr(namehash,name);
  if (!n) {
    n = NewHash();
    Setattr(namehash,name,n);
    Delete(n);
  }
  /* Add an object based on the declarator value */
  if (!decl) {
    Setattr(n,k_start,object);
  } else {
    SwigType *cd = Copy(decl);
    Setattr(n,cd,object);
    Delete(cd);
  }
}

/* -----------------------------------------------------------------------------
 * Swig_name_object_get()
 *
 * Return an object associated with an optional class prefix, name, and 
 * declarator.   This function operates according to name matching rules
 * described for the %rename directive in the SWIG manual.
 * ----------------------------------------------------------------------------- */

static DOH *get_object(Hash *n, String *decl) {
  DOH *rn = 0;
  if (!n) return 0;
  if (decl) {
    rn = Getattr(n,decl);
  } else {
    rn = Getattr(n,k_start);
  }
  return rn;
}

static
DOH *
name_object_get(Hash *namehash, String *tname, SwigType *decl, SwigType *ncdecl) {
  DOH* rn = 0;
  Hash   *n = Getattr(namehash,tname);
  if (n) {
    rn = get_object(n,decl);
    if ((!rn) && ncdecl) rn = get_object(n,ncdecl);
    if (!rn) rn = get_object(n,0);
  }
  return rn;
}

DOH *
Swig_name_object_get(Hash *namehash, String *prefix, String *name, SwigType *decl) {
  String *tname = NewStringEmpty();
  DOH    *rn = 0;
  char   *ncdecl = 0;

  if (!namehash) return 0;

  /* DB: This removed to more tightly control feature/name matching */
  /*  if ((decl) && (SwigType_isqualifier(decl))) {
    ncdecl = strchr(Char(decl),'.');
    ncdecl++;
  }
  */

  /* Perform a class-based lookup (if class prefix supplied) */
  if (prefix) {
    if (Len(prefix)) {
      Printf(tname,"%s::%s", prefix, name);
      rn = name_object_get(namehash, tname, decl, ncdecl);
      if (!rn) {
	String *cls = Swig_scopename_last(prefix);
	if (!Equal(cls,prefix)) {
	  Clear(tname);
	  Printf(tname,"*::%s::%s",cls,name);
	  rn = name_object_get(namehash, tname, decl, ncdecl);
	}
	Delete(cls);
      }
      /* A template-based class lookup, check name first */
      if (!rn && SwigType_istemplate(name)) {
	String *t_name = SwigType_templateprefix(name);
	if (!Equal(t_name,name)) {
	  rn = Swig_name_object_get(namehash, prefix, t_name, decl);
	}
	Delete(t_name);
      }
      /* A template-based class lookup */
      if (!rn && SwigType_istemplate(prefix)) {
	String *t_prefix = SwigType_templateprefix(prefix);
	if (Strcmp(t_prefix,prefix) != 0) {
	  String *t_name = SwigType_templateprefix(name);
	  rn = Swig_name_object_get(namehash, t_prefix, t_name, decl);
	  Delete(t_name);
	}
	Delete(t_prefix);
      }
      /* A wildcard-based class lookup */
      if (!rn) {
	if (!Equal(name,k_start)) {
	  rn = Swig_name_object_get(namehash, prefix, k_start, decl);
	}
      }
    }
    if (!rn) {
      Clear(tname);
      Printf(tname,"*::%s",name);
      rn = name_object_get(namehash, tname, decl, ncdecl);
    }
  } else {
    /* Lookup in the global namespace only */
    Clear(tname);
    Printf(tname,"::%s",name);
    rn = name_object_get(namehash, tname, decl, ncdecl);
  }
  /* Catch-all */
  if (!rn) {
    rn = name_object_get(namehash, name, decl, ncdecl);
  }
  if (!rn) {
    rn = name_object_get(namehash, k_start, decl, ncdecl);
  }
  Delete(tname);
  return rn;
}

/* -----------------------------------------------------------------------------
 * Swig_name_object_inherit()
 *
 * Implements name-based inheritance scheme. 
 * ----------------------------------------------------------------------------- */

void
Swig_name_object_inherit(Hash *namehash, String *base, String *derived) {
  Iterator ki;
  String *bprefix;
  String *dprefix;
  char *cbprefix;
  int   plen;

  if (!namehash) return;
  
  bprefix = NewStringf("%s::",base);
  dprefix = NewStringf("%s::",derived);
  cbprefix = Char(bprefix);
  plen = strlen(cbprefix);
  for (ki = First(namehash); ki.key; ki = Next(ki)) {
    char *k = Char(ki.key);
    if (strncmp(k,cbprefix,plen) == 0) {
      Iterator oi;
      String *nkey = NewStringf("%s%s",dprefix,k+plen);
      Hash *n = ki.item;
      Hash *newh = Getattr(namehash,nkey);
      if (!newh) {
	newh = NewHash();
	Setattr(namehash,nkey,newh);
	Delete(newh);
      }
      for (oi = First(n); oi.key; oi = Next(oi)) {
	if (!Getattr(newh,oi.key)) {
	  String *ci = Copy(oi.item);
	  Setattr(newh,oi.key,ci);
	  Delete(ci);
	}
      }
      Delete(nkey);
    }
  }
  Delete(bprefix);
  Delete(dprefix);
}

/* -----------------------------------------------------------------------------
 * merge_features()
 *
 * Given a hash, this function merges the features in the hash into the node.
 * ----------------------------------------------------------------------------- */

static void merge_features(Hash *features, Node *n) {
  Iterator ki;

  if (!features) return;
  for (ki = First(features); ki.key; ki = Next(ki)) {
    String *ci = Copy(ki.item);    
    Setattr(n,ki.key,ci);
    Delete(ci);
  }
}

/* -----------------------------------------------------------------------------
 * Swig_features_get()
 *
 * Attaches any features in the features hash to the node that matches
 * the declaration, decl.
 * ----------------------------------------------------------------------------- */

static 
void features_get(Hash *features, String *tname, SwigType *decl, SwigType *ncdecl, Node *node)
{
  Node *n = Getattr(features,tname);
#ifdef SWIG_DEBUG
  Printf(stdout,"  features_get: %s\n", tname);
#endif
  if (n) {
    merge_features(get_object(n,0),node);
    if (ncdecl) merge_features(get_object(n,ncdecl),node);
    merge_features(get_object(n,decl),node);
  }
}

void
Swig_features_get(Hash *features, String *prefix, String *name, SwigType *decl, Node *node) {
  char   *ncdecl = 0;
  String *rdecl = 0;
  SwigType *rname = 0;
  if (!features) return;

  /* MM: This removed to more tightly control feature/name matching */
  /*
  if ((decl) && (SwigType_isqualifier(decl))) {
    ncdecl = strchr(Char(decl),'.');
    ncdecl++;
  }
  */

  /* very specific hack for template constructors/destructors */
  if (name && SwigType_istemplate(name) &&
      (HashCheckAttr(node,k_nodetype,k_constructor)
       || HashCheckAttr(node, k_nodetype,k_destructor))) {
    String *nprefix = NewStringEmpty();
    String *nlast = NewStringEmpty();
    String *tprefix;
    Swig_scopename_split(name, &nprefix, &nlast);    
    tprefix = SwigType_templateprefix(nlast);
    Delete(nlast);
    if (Len(nprefix)) {
      Append(nprefix,"::");
      Append(nprefix,tprefix);
      Delete(tprefix);
      rname = nprefix;      
    } else {
      rname = tprefix;
      Delete(nprefix);
    }
    rdecl = Copy(decl);
    Replaceall(rdecl,name,rname);
    decl = rdecl;
    name = rname;
  }
  
#ifdef SWIG_DEBUG
  Printf(stdout,"Swig_features_get: %s %s %s\n", prefix, name, decl);
#endif

  /* Global features */
  features_get(features, "", 0, 0, node);
  if (name) {
    String *tname = NewStringEmpty();
    /* Catch-all */
    features_get(features, name, decl, ncdecl, node);
    /* Perform a class-based lookup (if class prefix supplied) */
    if (prefix) {
      /* A class-generic feature */
      if (Len(prefix)) {
	Printf(tname,"%s::",prefix);
	features_get(features, tname, decl, ncdecl, node);
      }
      /* A wildcard-based class lookup */
      Clear(tname);
      Printf(tname,"*::%s",name);
      features_get(features, tname, decl, ncdecl, node);
      /* A specific class lookup */
      if (Len(prefix)) {
	/* A template-based class lookup */
	if (SwigType_istemplate(prefix)) {
	  String *tprefix = SwigType_templateprefix(prefix);
	  Clear(tname);
	  Printf(tname,"%s::%s",tprefix,name);
	  features_get(features, tname, decl, ncdecl, node);
	  Delete(tprefix);
	}
	Clear(tname);
	Printf(tname,"%s::%s",prefix,name);
	features_get(features, tname, decl, ncdecl, node);
      }
    } else {
      /* Lookup in the global namespace only */
      Clear(tname);
      Printf(tname,"::%s",name);
      features_get(features, tname, decl, ncdecl, node);
    }
    Delete(tname);
  }
  if (name && SwigType_istemplate(name)) {
    String *dname = Swig_symbol_template_deftype(name,0);
    if (Strcmp(dname,name)) {    
      Swig_features_get(features, prefix, dname, decl, node);
    }
    Delete(dname);
  }

  Delete(rname);
  Delete(rdecl);
}


/* -----------------------------------------------------------------------------
 * Swig_feature_set()
 *
 * Sets a feature name and value. Also sets optional feature attributes as
 * passed in by featureattribs. Optional feature attributes are given a full name
 * concatenating the feature name plus ':' plus the attribute name.
 * ----------------------------------------------------------------------------- */

void 
Swig_feature_set(Hash *features, const String_or_char *name, SwigType *decl, const String_or_char *featurename, String *value, Hash *featureattribs) {
  Hash *n;
  Hash *fhash;

#ifdef SWIG_DEBUG
  Printf(stdout,"Swig_feature_set: %s %s %s %s\n", name, decl, featurename,value);  
#endif

  n = Getattr(features,name);
  if (!n) {
    n = NewHash();
    Setattr(features,name,n);
    Delete(n);
  }
  if (!decl) {
    fhash = Getattr(n,k_start);
    if (!fhash) {
      fhash = NewHash();
      Setattr(n,k_start,fhash);
      Delete(fhash);
    }
  } else {
    fhash = Getattr(n,decl);
    if (!fhash) {
      String *cdecl_ = Copy(decl);
      fhash = NewHash();
      Setattr(n,cdecl_,fhash);
      Delete(cdecl_);
      Delete(fhash);
    }
  }
  if (value) {
    Setattr(fhash,featurename,value);
  } else {
    Delattr(fhash,featurename);
  }

  {
    /* Add in the optional feature attributes */
    Hash *attribs = featureattribs;
    while(attribs) {
      String *attribname = Getattr(attribs,k_name);
      String *featureattribname = NewStringf("%s:%s", featurename, attribname);
      if (value) {
        String *attribvalue = Getattr(attribs,k_value);
        Setattr(fhash,featureattribname,attribvalue);
      } else {
        Delattr(fhash,featureattribname);
      }
      attribs = nextSibling(attribs);
      Delete(featureattribname);
    }
  }

  if (name && SwigType_istemplate(name)) {
    String *dname = Swig_symbol_template_deftype(name,0);
    if (Strcmp(dname,name)) {    
      Swig_feature_set(features, dname, decl, featurename, value, featureattribs);
    }
    Delete(dname);
  }
}





