/* File : example.cxx */

#include <Python.h>
#include "swigrun.swg"
#include "pyrun.swg"
#include "pyrunalias.swg"

#include "example.h"


Manager* convert_to_Manager(PyObject *py_obj)
{
  Manager* c_ptr;
  swig_type_info *ty = SWIG_TypeQuery("Manager *");
  printf("manager ty %x \n", ty);
  if (SWIG_ConvertPtr(py_obj, (void **) &c_ptr, ty, 0) == -1) {
    c_ptr = 0;
  } else {
    Py_XINCREF(py_obj);
  }
  return c_ptr;
}

