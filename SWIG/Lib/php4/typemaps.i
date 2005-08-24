//
// SWIG Typemap library
// Richard Palmer
// Oct 3, 2001
//
// PHP4 implementation
//
//
// This library provides standard typemaps for modifying SWIG's behavior.
// With enough entries in this file, I hope that very few people actually
// ever need to write a typemap.
//

//
// Define macros to define the following typemaps:
//
// INPUT arguments by Pointer
// OUTPUT arguments by Pointer
// INPUT arguments by Php reference
// OUTPUT arguments by Php reference
//
%define double_typemap(TYPE)
%typemap(in) TYPE *INPUT(TYPE temp)
{
        temp = (TYPE) Z_DVAL_PP($input);
        $1 = &temp;
}
%typemap(argout) TYPE *INPUT "";
%typemap(in,numinputs=0) TYPE *OUTPUT(TYPE temp)
{
  $1 = &temp;
}
%typemap(argout,fragment="t_output_helper") TYPE *OUTPUT
{
  zval *o;
  MAKE_STD_ZVAL(o);
  ZVAL_DOUBLE(o,temp$argnum);
  t_output_helper( &$result, o );
}
%typemap(in) TYPE *REFERENCE (TYPE dvalue)
{
  if(!ParameterPassedByReference(ht, argvi))
  {
        SWIG_PHP_Error(E_WARNING, "Parameter wasn't passed by reference");
	RETURN_NULL();
  }

  dvalue = (TYPE) (*$input)->value.dval;
  $1 = &dvalue;
}
%typemap(argout) TYPE *REFERENCE
{
  $1->value.dval = (double)(*$arg);
  $1->type = IS_DOUBLE;
}
%enddef

%define int_typemap(TYPE)
%typemap(in) TYPE *INPUT(TYPE temp)
{
        temp = (TYPE) Z_LVAL_PP($input);
        $1 = &temp;
}
%typemap(argout) TYPE *INPUT "";
%typemap(in,numinputs=0) TYPE *OUTPUT(TYPE temp)
{
  $1 = &temp;
}
%typemap(argout,fragment="t_output_helper") TYPE *OUTPUT
{
  zval *o;
  MAKE_STD_ZVAL(o);
  ZVAL_LONG(o,temp$argnum);
  t_output_helper( &$result, o );
}
%typemap(in) TYPE *REFERENCE (TYPE lvalue)
{
  if(!ParameterPassedByReference(ht, argvi))
  {
	SWIG_PHP_Error(E_WARNING, "Parameter wasn't passed by reference");
	RETURN_NULL();
  }

  lvalue = (TYPE) (*$input)->value.lval;
  $1 = &lvalue;
}
%typemap(argout) TYPE	*REFERENCE
{

  (*$arg)->value.lval = (long)(*$input);
  (*$arg)->type = IS_LONG;
}
%enddef

double_typemap(float);
double_typemap(double);

int_typemap(int);
int_typemap(short);
int_typemap(long);
int_typemap(unsigned int);
int_typemap(unsigned short);
int_typemap(unsigned long);
int_typemap(unsigned char);

%typemap(in) float *INOUT = float *INPUT;
%typemap(in) double *INOUT = double *INPUT;

%typemap(in) int *INOUT = int *INPUT;
%typemap(in) short *INOUT = short *INPUT;
%typemap(in) long *INOUT = long *INPUT;
%typemap(in) unsigned *INOUT = unsigned *INPUT;
%typemap(in) unsigned short *INOUT = unsigned short *INPUT;
%typemap(in) unsigned long *INOUT = unsigned long *INPUT;
%typemap(in) unsigned char *INOUT = unsigned char *INPUT;

%typemap(argout) float *INOUT = float *OUTPUT;
%typemap(argout) double *INOUT= double *OUTPUT;

%typemap(argout) int *INOUT = int *OUTPUT;
%typemap(argout) short *INOUT = short *OUTPUT;
%typemap(argout) long *INOUT= long *OUTPUT;
%typemap(argout) unsigned short *INOUT= unsigned short *OUTPUT;
%typemap(argout) unsigned long *INOUT = unsigned long *OUTPUT;
%typemap(argout) unsigned char *INOUT = unsigned char *OUTPUT;

%typemap(in) char INPUT[ANY] ( char temp[$1_dim0] )
{
  convert_to_string_ex($input);
  char *val = Z_LVAL_PP($input);
  strncpy(temp,val,$1_dim0);
  $1 = temp;
}
%typemap(in,numinputs=0) char OUTPUT[ANY] ( char temp[$1_dim0] )
{
  $1 = temp;
}
%typemap(argout) char OUTPUT[ANY]
{
  size_t len = strnlen($1,$1_dim0);
  RETURN_STRINGL( $1, len );
}

%typemap(in,numinputs=0) void **OUTPUT (int force),
                         void *&OUTPUT (int force)
{
  /* If they pass NULL by reference, make it into a void*
     This bit should go in arginit if arginit support init-ing scripting args */
  if(SWIG_ConvertPtr(*$input, (void **) &$1, $1_descriptor) < 0) {
    /* So... we didn't get a ref or ptr, but we'll accept NULL by reference */
    if ((*$input)->type==IS_NULL && PZVAL_IS_REF(*$input)) {
#ifdef __cplusplus
      ptr=new $*1_ltype;
#else
      ptr=($*1_ltype) calloc(1,sizeof($*1_ltype));
#endif
      $1=&ptr;
      /* have to passback arg$arg too */
      force=1;
    } else {  /* wasn't a pre/ref/thing, OR anything like an int thing */
      force=0;
      SWIG_PHP_Error(E_ERROR, "Type error in argument $arg of $symname.");
    }
  } else force=0;
}

%typemap(argout) void **OUTPUT,
                 void *&OUTPUT
{
  if (force$argnum) {  /* pass back arg$argnum through params ($arg) if we can */
    if(! PZVAL_IS_REF(*$arg)) {
      SWIG_PHP_Error(E_WARNING, "Parameter $argnum of $symname wasn't passed by reference");
    } else {
      SWIG_SetPointerZval(*$arg, (void *) ptr$argnum, $*1_descriptor, 1);
    }
  }
}
