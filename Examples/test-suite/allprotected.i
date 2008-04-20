// Tests for the allprotected option

%module(directors="1", allprotected="1") allprotected

%{
#include <string>
%}

%include "std_string.i"

%feature("director") PublicBase;
%feature("director") ProtectedBase;

// protected types not supported (ProtectedEnum, IntegerType). Make sure they can be ignored.
%ignore ProtectedBase::protectedenum;
%ignore ProtectedBase::typedefs;

%inline %{
class Klass {
  std::string name;
public:
  Klass(const std::string& n) : name(n) {}
  std::string getName() { return name; }
};

class PublicBase {
  std::string str;
public:
  enum AnEnum { EnumVal1, EnumVal2 };
public:
  PublicBase(const char* s): str(s) {}
  virtual ~PublicBase() { }
  virtual std::string virtualMethod() const { return "PublicBase"; }
  Klass instanceMethod(Klass k) const { return k; }
  static Klass staticMethod(Klass k) { return k; }
  int instanceMemberVariable;
  static int staticMemberVariable;
  static const int staticConstMemberVariable = 20;
  AnEnum anEnum;
};
int PublicBase::staticMemberVariable = 10;

class ProtectedBase {
  std::string str;
public:
  enum AnEnum { EnumVal1, EnumVal2 };
  std::string getName() { return str; }
protected:
  ProtectedBase(const char* s): str(s) {}
  virtual ~ProtectedBase() { }
  virtual std::string virtualMethod() const { return "ProtectedBase"; }
  Klass instanceMethod(Klass k) const { return k; }
  static Klass staticMethod(Klass k) { return k; }
  int instanceMemberVariable;
  static int staticMemberVariable;
  static const int staticConstMemberVariable = 20;
  AnEnum anEnum;

// unsupported: types defined with protected access and thus methods/variables which use them
  enum ProtectedEnum { ProtEnumVal1, ProtEnumVal2 };
  typedef int IntegerType;
  ProtectedEnum protectedenum;
  IntegerType typedefs(IntegerType it) { return it; }
};
int ProtectedBase::staticMemberVariable = 10;

%}

