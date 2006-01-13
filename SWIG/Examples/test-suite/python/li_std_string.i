%module li_std_string
%include <std_basic_string.i>
%include <std_string.i>

%inline %{

struct A : std::string 
{
  A(const std::string& s) : std::string(s)
  {
  }
};

struct B 
{
  B(const std::string& s) : cname(0), name(s), a(s)
  {
  }
  
  char *cname;
  std::string name;
  A a;

};
 

const char* test_ccvalue(const char* x) {
   return x;
}

char* test_cvalue(char* x) {
   return x;
}
  

std::string test_value(std::string x) {
   return x;
}

const std::string& test_const_reference(const std::string &x) {
   return x;
}

void test_pointer(std::string *x) {
}

std::string *test_pointer_out() {
   static std::string x = "x";
   return &x;
}

void test_const_pointer(const std::string *x) {
}

const std::string *test_const_pointer_out() {
   static std::string x = "x";
   return &x;
}

void test_reference(std::string &x) {
}

std::string& test_reference_out() {
   static std::string x = "x";
   return x;
}

#if defined(_MSC_VER)
  #pragma warning(disable: 4290) // C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#endif

void test_throw() throw(std::string){
  static std::string x = "x";
  
  throw x;
}

#if defined(_MSC_VER)
  #pragma warning(default: 4290) // C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#endif


std::basic_string<char> test_value_basic1(std::basic_string<char> x) {
   return x;
}

std::basic_string<char,std::char_traits<char> > test_value_basic2(std::basic_string<char,std::char_traits<char> > x) {
   return x;
}

std::basic_string<char,std::char_traits<char>,std::allocator<char> > test_value_basic3(std::basic_string<char,std::char_traits<char>,std::allocator<char> > x) {
   return x;
}

%}


