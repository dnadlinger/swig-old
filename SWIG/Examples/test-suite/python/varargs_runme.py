import varargs

if varargs.test("Hello") != "Hello":
    raise RuntimeError, "Failed"

f = varargs.Foo("Greetings")
if f.str != "Greetings":
    raise RuntimeError, "Failed"

if f.test("Hello") != "Hello":
    raise RuntimeError, "Failed"
