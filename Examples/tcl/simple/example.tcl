# file: example.tcl
# Try to load as a dynamic module.  If not, we'll just assume

catch { load ./example.so example}
catch { load ./example.dll example}    ;# Windows

# Call our gcd() function
set x 42
set y 105
set g [gcd $x $y]
puts "The gcd of $x and $y is $g"

# Manipulate the Foo global variable

# Output its current value
puts "Foo = $Foo"

# Change its value
set Foo 3.1415926

# See if the change took effect
puts "Foo = $Foo"

