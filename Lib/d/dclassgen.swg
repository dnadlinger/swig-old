/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * dclassgen.swg
 *
 * Typemaps containing D code used when generating D proxy classes.
 * ----------------------------------------------------------------------------- */

%typemap(csbase)              SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
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
  this($imcall, true);$excode$directorconnect
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
        $imcall;
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
        $imcall;
      }
      m_swigCObject = null;
    }
    super.dispose();
  }
}

// TODO: How much visibility is needed for swigGetCObject?
%typemap(dbody) SWIGTYPE %{
private void* m_swigCObject;
protected bool m_swigOwnCObject;

protected this(void* cObject, bool ownCObject) {
  m_swigCObject = cObject;
  m_swigOwnCObject = ownCObject;
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}

%typemap(dbody_derived) SWIGTYPE %{
private void* m_swigCObject;

protected this(void* cObject, bool ownCObject) {
  super($wrapdmodule.$dclassnameUpcast(cObject), ownCObject);
  m_swigCObject = cObject;
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}


/*
 * Type wrapper classes.
 */

// TODO: How much visibility is needed for the two-arguments constructor?
%typemap(dbody) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] %{
private void* m_swigCObject;

public this(void* cObject, bool futureUse) {
  m_swigCObject = cObject;
}

protected this() {
  m_swigCObject = null;
}

public static void* swigGetCObject($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCObject;
}
%}


/*
 * Member function pointer wrapper classes (see <dmemberfunctionpointers.swg>).
 */
// TODO: How much visibility is needed for swigGetCMemberPtr?
%typemap(dbody) SWIGTYPE (CLASS::*) %{
private char* m_swigCMemberPtr;

public this(char* cMemberPtr, bool futureUse) {
  m_swigCMemberPtr = cMemberPtr;
}

protected this() {
  m_swigCMemberPtr = null;
}

public static char* swigGetCMemberPtr($dclassname obj) {
  return (obj is null) ? null : obj.m_swigCMemberPtr;
}
%}
