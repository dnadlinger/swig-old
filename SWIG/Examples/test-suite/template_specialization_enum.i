%module template_specialization_enum


%inline %{

  enum Hello 
  {
    hi, hello
  };
  

  template <Hello, class A>
    struct C
    {
    };
  

  template <Hello, class BB>
    struct Base
    {
    };  
  
  
  template <class A>
    struct C<hello , A>  : Base<hello, A>
    {
      int hello()
      {
	return hello;
      }
      
    protected:
      C()
      {
      }
    };
  

  template <class A>
    struct C<hi , A> : Base<hi, A>
    {
      int hi()
      {
	return hi;
      }

    protected:
      C()
      {
      }
    };
  
      
%}

%template(Base_dd) Base<hi, int>;
%template(Base_ii) Base<hello, int>;

%template(C_i) C<hi, int>;
%template(C_d) C<hello, int>;
