%module namespace_nested

%warnfilter(801) hello::hi::hi0;	/* Ruby, wrong class name */
%warnfilter(801) oss::hi1<hello::Hi0 >;	/* Ruby, wrong class name */

%inline %{
  namespace hello
  {  
    namespace hi
    {      
      struct hi0
      {
      };      
    }

    template < class T1 >
    struct Hi : T1
    {
    };
  }

%}

namespace hello
{
  %template(Hi_hi0) Hi<hi::hi0>;
}


%inline %{

  namespace hello
  {
    //
    // This works 
    //
    // typedef Hi<hello::hi::hi0> Hi0;
    
    //
    // This doesn't work
    //
    typedef Hi<hi::hi0> Hi0;
  };
  
  
  namespace oss
  {
    template <class T1>
    struct hi1 : T1
    {
    };    
  }
  
%}

namespace oss
{
  %template(hi1_hi0) hi1<hello::Hi0 >;
}
