/* ------------------------------------------------------------
 * --- Argc & Argv ---
 * ------------------------------------------------------------ */

%fragment("SWIG_AsArgcArgv","header") %{
SWIGSTATIC(char**)
SWIG_AsArgcArgv(PyObject* input,
		      swig_type_info* ppchar_info,
		      swig_type_info* pchar_info,
		      size_t* argc, int* owner)
{  
  char **argv = 0;
  size_t i = 0;
  if (SWIG_ConvertPtr(input, (void **)&argv, ppchar_info, 0) == -1) {
    PyErr_Clear();
    int list = PyList_Check(input);
    if (list || PyTuple_Check(input)) {
      *argc = list ? PyList_Size(input) : PyTuple_Size(input);
      argv = swig_new_array(char*, *argc + 1);
      *owner = 1;
      for (; i < *argc; ++i) {
	PyObject *obj = list ? PyList_GetItem(input,i) : PyTuple_GetItem(input,i);
	argv[i] = SWIG_AsCharPtr(obj, pchar_info);
	if (PyErr_Occurred()) {
	  PyErr_Clear();
	  PyErr_SetString(PyExc_TypeError,"list or tuple must contain strings only");
	}
      }
      argv[i] = 0;
      return argv;
    } else {
      *argc = 0;
      PyErr_SetString(PyExc_TypeError,"a list or tuple is expected");
      return 0;
    }
  } else {
    /* seems dangerous, but the user asked for it... */
    while (argv[i] != 0) ++i;
    *argc = i;
    owner = 0;
    return argv;
  }
}
%}

/*
  This typemap works with either a char**, a python list or a python
  tuple
 */

%typemap(in,fragment="SWIG_AsArgcArgv") (int ARGC, char **ARGV)
  (int owner) {
  size_t argc = 0;
  char **argv = SWIG_AsArgcArgv($input, $descriptor(char**), 
				      $descriptor(char*), &argc, &owner);
  if (PyErr_Occurred()) { 
    $1 = 0; $2 = 0;
    SWIG_fail;
  } else {  
    $1 = ($1_ltype) argc;
    $2 = ($2_ltype) argv;
  }
}

%typemap(freearg) (int ARGC, char **ARGV) (owner) {
  if (owner) swig_delete_array($2);
}

