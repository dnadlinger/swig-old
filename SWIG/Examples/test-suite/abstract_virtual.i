%module(ruby_minherit="1") abstract_virtual

%warnfilter(813,833) D; /* C#, Java multiple inheritance */
%warnfilter(813,833) E; /* C#, Java multiple inheritance */

%inline %{
  struct A 
  {
    virtual int foo() = 0;
  };
 
  struct B : virtual A
  {
    int foo() 
    {
      return 0;
    }
  };
  
  struct C: virtual A
  {
  protected:
    C()
    {
    }
  };

  //
  // This case works
  //
  struct D : B, C
  {
    D()
    {
    }
  };

  //
  // This case doesn't work.
  // It seems the is_abstract function doesn't
  // navigate the entire set of base classes,
  // and therefore, it doesn't detect B::foo()
  //
#ifdef SWIG
  // Uncommenting this line, of course, make it works
  // %feature("notabstract") E;
#endif
  //
  struct E : C, B
  {
    E()
    {
    }    
  };
%}

