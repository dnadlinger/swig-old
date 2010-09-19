module runme;

import tango.io.Stdout;
static import example;

void main() {
  int a = 37;
  int b = 42;

  Stdout( "Trying some C callback functions:" ).newline;
  Stdout( "    a        = " )( a ).newline;
  Stdout( "    b        = " )( b ).newline;
  Stdout( "    ADD(a,b) = " )( example.do_op( a, b, example.ADD ) ).newline;
  Stdout( "    SUB(a,b) = " )( example.do_op( a, b, example.SUB ) ).newline;
  Stdout( "    MUL(a,b) = " )( example.do_op( a, b, example.MUL ) ).newline;
}
