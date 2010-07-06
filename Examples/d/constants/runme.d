static import example;
import tango.io.Stdout;

void main() {
  Stdout.formatln("ICONST  = {} (should be 42)", example.ICONST);
  Stdout.formatln("FCONST  = {} (should be 2.1828)", example.FCONST);
  Stdout.formatln("CCONST  = {} (should be 'x')", example.CCONST);
  Stdout.formatln("CCONST2 = {} (this should be on a new line)", example.CCONST2);
  Stdout.formatln("SCONST  = {} (should be 'Hello World')", example.SCONST);
  Stdout.formatln("SCONST2 = {} (should be '\"Hello World\"')", example.SCONST2);
  Stdout.formatln("EXPR    = {} (should be 48.5484)", example.EXPR);
  Stdout.formatln("iconst  = {} (should be 37)", example.iconst);
  Stdout.formatln("fconst  = {} (should be 3.14)", example.fconst);

  static if ( is( typeof( example.EXTERN ) ) ) {
    Stdout.formatln( "EXTERN should not be defined, but is: {}.", example.EXTERN );
  } else {
    Stdout.formatln("EXTERN isn't defined (good)");
  }

  static if ( is( typeof( example.FOO ) ) ) {
    Stdout.formatln( "FOO should not be defined, but is: {}.", example.FOO );
  } else {
    Stdout.formatln("FOO isn't defined (good)");
  }
}