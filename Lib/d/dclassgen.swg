/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dclassgen.swg
 *
 * Typemaps containing D code used when generating D proxy classes.
 * ----------------------------------------------------------------------------- */

%typemap(dbase)              SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dclassmodifiers)     SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) "class"
%typemap(dcode)               SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dimports)            SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dinterfaces)         SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(dinterfaces_derived) SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""

// See <denums.swg>.
%typemap(dclassmodifiers) enum SWIGTYPE "enum"
%typemap(dcode) enum SWIGTYPE ""


/*
 * Proxy classes.
 */

%typemap(dconstructor, excode=SWIGEXCODE,directorconnect="\n  swigDirectorConnect();") SWIGTYPE {
  this($wcall, true);$excode$directorconnect
}

%typemap(ddestructor) SWIGTYPE %{
~this() {
  dispose();
}
%}

// We do not use »override« attribute for generated dispose() methods to stay
// somewhat compatible to Phobos and older Tango versions where Object.dispose()
// does not exist.
%typemap(ddispose, methodname="dispose", methodmodifiers="public") SWIGTYPE {
  synchronized(this) {
    if (m_swigCObject !is null) {
      if (m_swigOwnCObject) {
        m_swigOwnCObject = false;
        $wcall;
      }
      m_swigCObject = null;
    }
  }
}

%typemap(ddispose_derived, methodname="dispose", methodmodifiers="public") SWIGTYPE {
  synchronized(this) {
    if (m_swigCObject !is null) {
      if (m_swigOwnCObject) {
        m_swigOwnCObject = false;
        $wcall;
      }
      m_swigCObject = null;
    }
    super.dispose();
  }
}

%define SWIGD_CLASS_BODIES(OP_EQUALS_RETURN)
// Unfortunately, the »package« visibility attribute does not work in D when the
// module in question is in the root package (happens if no -package is specified
// at the SWIG command line), so we are stuck with public visibility for
// swigGetCObject().
%typemap(dbody) SWIGTYPE %{
private void* m_swigCObject;
protected bool m_swigOwnCObject;

public this(void* cObject, bool ownCObject) {
  m_swigCObject = cObject;
  m_swigOwnCObject = ownCObject;
}

public override OP_EQUALS_RETURN opEquals(Object rhs) {
  if (auto other = cast($dclassname)rhs) {
    return (m_swigCObject == other.m_swigCObject);
  }
  return super.opEquals(rhs);
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}

%typemap(dbody_derived) SWIGTYPE %{
private void* m_swigCObject;

public this(void* cObject, bool ownCObject) {
  super($wrapdmodule.$dclassnameUpcast(cObject), ownCObject);
  m_swigCObject = cObject;
}

public override OP_EQUALS_RETURN opEquals(Object rhs) {
  if (auto other = cast($dclassname)rhs) {
    return (m_swigCObject == other.m_swigCObject);
  }
  return super.opEquals(rhs);
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}


/*
 * Type wrapper classes.
 */

%typemap(dbody) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] %{
private void* m_swigCObject;

public this(void* cObject, bool futureUse) {
  m_swigCObject = cObject;
}

protected this() {
  m_swigCObject = null;
}

public override OP_EQUALS_RETURN opEquals(Object rhs) {
  if (auto other = cast($dclassname)rhs) {
    return (m_swigCObject == other.m_swigCObject);
  }
  return super.opEquals(rhs);
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}


/*
 * Member function pointer wrapper classes (see <dmemberfunctionpointers.swg>).
 */

%typemap(dbody) SWIGTYPE (CLASS::*) %{
private char* m_swigCMemberPtr;

public this(char* cMemberPtr, bool futureUse) {
  m_swigCMemberPtr = cMemberPtr;
}

protected this() {
  m_swigCMemberPtr = null;
}

public override OP_EQUALS_RETURN opEquals(Object rhs) {
  if (auto other = cast($dclassname)rhs) {
    return (m_swigCMemberPtr == other.m_swigCMemberPtr);
  }
  return super.opEquals(rhs);
}


package static char* swigGetCMemberPtr($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCMemberPtr;
}
%}
%enddef


#if SWIG_D_VERSION == 1
  SWIGD_CLASS_BODIES(int)
#else
  SWIGD_CLASS_BODIES(bool)
#endif

#undef SWIGD_CLASS_BODIES
