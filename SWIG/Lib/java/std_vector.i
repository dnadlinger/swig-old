//
// SWIG typemaps for std::vector
// Luigi Ballabio
// May 7, 2002
//
// Java implementation


%include exception.i

// containers

// methods which can raise are caused to throw an IndexError
%exception std::vector::get {
    try {
        $action
    } catch (std::out_of_range& e) {
        SWIG_exception(SWIG_IndexError,const_cast<char*>(e.what()));
    }
}

%exception std::vector::set {
    try {
        $action
    } catch (std::out_of_range& e) {
        SWIG_exception(SWIG_IndexError,const_cast<char*>(e.what()));
    }
}


// ------------------------------------------------------------------------
// std::vector
// 
// The aim of all that follows would be to integrate std::vector with 
// Java as much as possible, namely, to allow the user to pass and 
// be returned Java (arrays? containers?)
// const declarations are used to guess the intent of the function being
// exported; therefore, the following rationale is applied:
// 
//   -- f(std::vector<T>), f(const std::vector<T>&), f(const std::vector<T>*):
//      the parameter being read-only, either a Java sequence or a
//      previously wrapped std::vector<T> can be passed.
//   -- f(std::vector<T>&), f(std::vector<T>*):
//      the parameter must be modified; therefore, only a wrapped std::vector
//      can be passed.
//   -- std::vector<T> f():
//      the vector is returned by copy; therefore, a Java sequence of T:s 
//      is returned which is most easily used in other Java functions
//   -- std::vector<T>& f(), std::vector<T>* f(), const std::vector<T>& f(),
//      const std::vector<T>* f():
//      the vector is returned by reference; therefore, a wrapped std::vector
//      is returned
// ------------------------------------------------------------------------

%{
#include <vector>
#include <algorithm>
#include <stdexcept>
%}

// exported class

namespace std {
    
    template<class T> class vector {
        // add generic typemaps here
      public:
        vector(unsigned int size = 0);
        unsigned int size() const;
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %rename(add) push_back;
        void push_back(const T& x);
        %extend {
            T& get(int i) {
                int size = int(self->size());
                if (i>=0 && i<size)
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(int i, const T& x) {
                int size = int(self->size());
                if (i>=0 && i<size)
                    (*self)[i] = x;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };


    // specializations for built-ins

    %define specialize_std_vector(T)
    template<> class vector<T> {
        // add specialized typemaps here
      public:
        vector(unsigned int size = 0);
        unsigned int size() const;
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %rename(add) push_back;
        void push_back(T x);
        %extend {
            T get(int i) {
                int size = int(self->size());
                if (i>=0 && i<size)
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(int i, T x) {
                int size = int(self->size());
                if (i>=0 && i<size)
                    (*self)[i] = x;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
    %enddef

    specialize_std_vector(bool);
    specialize_std_vector(char);
    specialize_std_vector(int);
    specialize_std_vector(short);
    specialize_std_vector(long);
    specialize_std_vector(unsigned char);
    specialize_std_vector(unsigned int);
    specialize_std_vector(unsigned short);
    specialize_std_vector(unsigned long);
    specialize_std_vector(float);
    specialize_std_vector(double);
    specialize_std_vector(std::string);

}

