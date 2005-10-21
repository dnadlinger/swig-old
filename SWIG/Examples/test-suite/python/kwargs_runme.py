from kwargs import *

# Simple class
f = Foo(b=2,a=1)

if f.foo(b=1,a=2) != 3:
  raise RuntimeError

if Foo.statfoo(b=2) != 3:
  raise RuntimeError

if f.efoo(b=2) != 3:
  raise RuntimeError

if Foo.sfoo(b=2) != 3:
  raise RuntimeError


# Templated class
b = BarInt(b=2,a=1)

if b.bar(b=1,a=2) != 3:
  raise RuntimeError

if BarInt.statbar(b=2) != 3:
  raise RuntimeError

if b.ebar(b=2) != 3:
  raise RuntimeError

if BarInt.sbar(b=2) != 3:
  raise RuntimeError


# Functions
if templatedfunction(b=2) != 3:
  raise RuntimeError

if foo(a=1,b=2) != 3:
  raise RuntimeError

if foo(b=2) != 3:
  raise RuntimeError


#Funtions with keywords

if foo_kw(_from=2) != 4:
  raise RuntimeError
