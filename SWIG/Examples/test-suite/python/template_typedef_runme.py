from template_typedef import *

d = make_Identity_float()
c = make_Identity_real()


try:
  a = d.this
  a = c.this
except:
  raise RuntimeError

try:
  e = make_Multiplies_float_float_float_float(d, d)
  a = e.this
except:
  print e, "is not an instance"
  raise RuntimeError

try:
  f = make_Multiplies_real_real_real_real(c, c)
  a = f.this
except:
  print f, "is not an instance"
  raise RuntimeError

try:
  g = make_Multiplies_float_float_real_real(d, c)
  a = g.this
except:
  print g, "is not an instance"
  raise RuntimeError

