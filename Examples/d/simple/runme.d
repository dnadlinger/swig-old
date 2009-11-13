module runme;

static import example;
import tango.io.Stdout;

/*
 * Call our gcd() function.
 */
int x = 42;
int y = 105;
int g = example.gcd( x, y );
Stdout.format( "The gcd of {} and {} is {}.", x, y, g ).newline;

/*
 * Manipulate the Foo global variable.
 */

// Output its current value
Stdout.format( "Foo = {}" + example.Foo ).newline;

// Change its value
example.Foo = 3.1415926;

// See if the change took effect
Stdout.format( "Foo = {}" + example.Foo ).newline;