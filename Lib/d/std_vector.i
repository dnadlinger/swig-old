/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * std_vector.i
 *
 * SWIG typemaps for std::vector<T>, D implementation.
 *
 * The D wrapper is made to loosely resemble a tango.util.container.more.Vector
 * and to provide built-in array-like access.
 *
 * If T does define an operator==, then use the SWIG_STD_VECTOR_ENHANCED
 * macro to obtain enhanced functionality (none yet), for example:
 *
 *   SWIG_STD_VECTOR_ENHANCED(SomeNamespace::Klass)
 *   %template(VectKlass) std::vector<SomeNamespace::Klass>;
 *
 * Warning: heavy macro usage in this file. Use swig -E to get a sane view on
 * the real file contents!
 * ----------------------------------------------------------------------------- */

// Warning: Use the typemaps here in the expectation that the macros they are in will change name.

%include <std_common.i>

// MACRO for use within the std::vector class body
%define SWIG_STD_VECTOR_MINIMUM_INTERNAL(CONST_REFERENCE_TYPE, CTYPE...)
%pragma(d) proxydmoduleimports=%{
import tango.core.Exception;
import tango.io.Stdout;
%}

%typemap(dcode) std::vector<CTYPE > %{
  public this($typemap(cstype, CTYPE)[] values) {
    this();
    append(values);
  }

  alias push_back add;
  alias push_back push;
  alias push_back opCatAssign;
  alias size length;
  alias opSlice slice;

  public $typemap(cstype, CTYPE) opIndexAssign($typemap(cstype, CTYPE) value, size_t index) {
    if (index >= size()) {
      throw new NoSuchElementException("Tried to assign to element out of vector bounds.");
    }
    setElement(index, value);
    return value;
  }

  public $typemap(cstype, CTYPE) opIndex(size_t index) {
    if (index >= size()) {
      throw new NoSuchElementException("Tried to read from element out of vector bounds.");
    }
    return getElement(index);
  }

  public void append($typemap(cstype, CTYPE)[] value...) {
    foreach (v; value) {
      add(v);
    }
  }

  public $typemap(cstype, CTYPE)[] opSlice() {
    $typemap(cstype, CTYPE)[] array = new $typemap(cstype, CTYPE)[size()];
    foreach (i, ref value; array) {
      value = getElement(i);
    }
    return array;
  }

  public int opApply(int delegate(ref $typemap(cstype, CTYPE) value) dg) {
    int result;

    size_t currentSize = size();
    for (size_t i = 0; i < currentSize; ++i) {
      auto value = getElement(i);
      result = dg(value);
      setElement(i, value);
    }
    return result;
  }

  public int opApply(int delegate(ref size_t index, ref $typemap(cstype, CTYPE) value) dg) {
    int result;

    size_t currentSize = size();
    for (size_t i = 0; i < currentSize; ++i) {
      auto value = getElement(i);

      // Workaround for http://d.puremagic.com/issues/show_bug.cgi?id=2443.
      auto index = i;

      result = dg(index, value);
      setElement(i, value);
    }
    return result;
  }

  public void capacity(size_t value) {
    if (value < size()) {
      throw new IllegalArgumentException( "Tried to make the capacity of a vector smaller than its size." );
    }

    reserve(value);
  }
%}

  public:
    typedef size_t size_type;
    typedef CTYPE value_type;
    typedef CONST_REFERENCE_TYPE const_reference;
    void clear();
    void push_back(const value_type& x);
    size_type size() const;
    size_type capacity() const;
    void reserve(size_type n) throw (std::length_error);
    vector();
    vector(const vector &other);
    %extend {
      vector(size_type capacity) throw (std::length_error) {
        std::vector<CTYPE >* pv = 0;
	pv = new std::vector<CTYPE >();

	// Might throw std::length_error.
	pv->reserve(capacity);

	return pv;
      }

      size_type unused() {
	return $self->capacity() - $self->size();
      }

      const_reference remove() throw (std::out_of_range) {
        if ($self->empty()) {
          throw std::out_of_range("Tried to remove last element from empty vector.");
        }

        std::vector<CTYPE >::const_reference value = $self->back();
        $self->pop_back();
        return value;
      }

      const_reference remove(size_type index) throw (std::out_of_range) {
        if (index >= $self->size()) {
          throw std::out_of_range("Tried to remove element with invalid index.");
        }

        std::vector<CTYPE >::iterator it = $self->begin() + index;
        std::vector<CTYPE >::const_reference value = *it;
        $self->erase(it);
        return value;
      }
    }

    // Wrappers for setting/getting items with the possibly thrown exception
    // specified (important for SWIG wrapper generation).
    %extend {
      const_reference getElement(size_type index) throw (std::out_of_range) {
        if ((index < 0) || ($self->size() <= index)) {
	  throw std::out_of_range("Tried to get value of element with invalid index.");
        }
        return (*$self)[index];
      }
   }
   // Use const value_type& instead of const_reference to work around SWIG code
   // generation issue when using const pointers as vector elements (like
   // std::vector< const int* >).
   %extend {
      void setElement(size_type index, const value_type& val) throw (std::out_of_range) {
        if ((index < 0) || ($self->size() <= index)) {
	  throw std::out_of_range("Tried to set value of element with invalid index.");
        }
        (*$self)[index] = val;
      }
    }
%enddef

%define SWIG_STD_VECTOR_MINIMUM(CTYPE...)
SWIG_STD_VECTOR_MINIMUM_INTERNAL(const value_type&, CTYPE)
%enddef

// Extra methods added to the collection class if operator== is defined for the class being wrapped
// The class will then implement IList<>, which adds extra functionality
%define SWIG_STD_VECTOR_EXTRA_OP_EQUALS_EQUALS(CTYPE...)
    %extend {
    }
%enddef

// Macros for std::vector class specializations/enhancements
%define SWIG_STD_VECTOR_ENHANCED(CTYPE...)
namespace std {
  template<> class vector<CTYPE > {
    SWIG_STD_VECTOR_MINIMUM_INTERNAL(const value_type&, CTYPE)
    SWIG_STD_VECTOR_EXTRA_OP_EQUALS_EQUALS(CTYPE)
  };
}
%enddef

// Legacy macros
%define SWIG_STD_VECTOR_SPECIALIZE(CSTYPE, CTYPE...)
#warning SWIG_STD_VECTOR_SPECIALIZE macro deprecated, please see d/std_vector.i and switch to SWIG_STD_VECTOR_ENHANCED
SWIG_STD_VECTOR_ENHANCED(CTYPE)
%enddef

%define SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(CSTYPE, CTYPE...)
#warning SWIG_STD_VECTOR_SPECIALIZE_MINIMUM macro deprecated, it is no longer required
%enddef

%{
#include <vector>
#include <stdexcept>
%}

%dmethodmodifiers std::vector::getElement "private"
%dmethodmodifiers std::vector::setElement "private"
%dmethodmodifiers std::vector::reserve "private"

namespace std {
  // primary (unspecialized) class template for std::vector
  // does not require operator== to be defined
  template<class T> class vector {
    SWIG_STD_VECTOR_MINIMUM(T)
  };
  // specializations for pointers
  template<class T> class vector<T*> {
    SWIG_STD_VECTOR_MINIMUM_INTERNAL(const value_type&, T*)
    SWIG_STD_VECTOR_EXTRA_OP_EQUALS_EQUALS(T*)
  };
  template<class T> class vector<const T*> {
    SWIG_STD_VECTOR_MINIMUM_INTERNAL(const value_type&, const T*)
    SWIG_STD_VECTOR_EXTRA_OP_EQUALS_EQUALS(const T*)
  };
  // bool is a bit different in the C++ standard
  template<> class vector<bool> {
    SWIG_STD_VECTOR_MINIMUM_INTERNAL(bool, bool)
    SWIG_STD_VECTOR_EXTRA_OP_EQUALS_EQUALS(bool)
  };
}

// template specializations for std::vector
// these provide extra collections methods as operator== is defined
SWIG_STD_VECTOR_ENHANCED(char)
SWIG_STD_VECTOR_ENHANCED(signed char)
SWIG_STD_VECTOR_ENHANCED(unsigned char)
SWIG_STD_VECTOR_ENHANCED(short)
SWIG_STD_VECTOR_ENHANCED(unsigned short)
SWIG_STD_VECTOR_ENHANCED(int)
SWIG_STD_VECTOR_ENHANCED(unsigned int)
SWIG_STD_VECTOR_ENHANCED(long)
SWIG_STD_VECTOR_ENHANCED(unsigned long)
SWIG_STD_VECTOR_ENHANCED(long long)
SWIG_STD_VECTOR_ENHANCED(unsigned long long)
SWIG_STD_VECTOR_ENHANCED(float)
SWIG_STD_VECTOR_ENHANCED(double)
SWIG_STD_VECTOR_ENHANCED(std::string) // also requires a %include <std_string.i>
