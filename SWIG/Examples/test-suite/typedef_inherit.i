// Inheritance through a typedef name
%module typedef_inherit

%inline %{
class Foo {
public:
     virtual char *blah() {
	return (char *) "Foo::blah";
     }
};

typedef Foo FooObj;

class Bar : public FooObj {
 public:
  virtual char *blah() {
    return (char *) "Bar::blah";
  };
};

char *do_blah(FooObj *f) {
  return f->blah();
}

typedef struct {
   virtual char *blah() {     
       return (char *) "Spam::blah";
   }
} Spam;

struct Grok : public Spam {
   virtual char *blah() {
       return (char *) "Grok::blah";
   }
};

char *do_blah2(Spam *s) {
   return s->blah();
}
%}

