%module operbool

%rename(operbool) operator bool();
%warnfilter(SWIGWARN_D_NAME_COLLISION) operator bool();

%inline %{
  class Test {
  public:
    operator bool() {
      return false;
    }
  };
%}
