static import example;
import tango.io.Stdout;

void main() {
  int a = 37;
  int b = 42;

  // Now call our C function with a bunch of callbacks.
  Stdout( "Trying some C callback functions:" ).newline;
  Stdout( "    a        = " )( a ).newline;
  Stdout( "    b        = " )( b ).newline;
  Stdout( "    ADD(a,b) = " )( example.do_op(a,b,example.ADD) ).newline;
  Stdout( "    SUB(a,b) = " )( example.do_op(a,b,example.SUB) ).newline;
  Stdout( "    MUL(a,b) = " )( example.do_op(a,b,example.MUL) ).newline;

  Stdout( "\nThe names of the C callback function classes in D:" ).newline;
  Stdout( "    ADD      = " )( example.ADD.classinfo.name ).newline;
  Stdout( "    SUB      = " )( example.SUB.classinfo.name ).newline;
  Stdout( "    MUL      = " )( example.MUL.classinfo.name ).newline;
}
