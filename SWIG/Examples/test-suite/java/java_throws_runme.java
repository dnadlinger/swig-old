
import java_throws.*;

public class java_throws_runme {

  static {
    try {
        System.loadLibrary("java_throws");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
      System.exit(1);
    }
  }

  public static void main(String argv[]) 
  {
      // Check the exception classes in the main typemaps
      boolean pass = false;

      // This won't compile unless all of these exceptions are in the throw clause
      try {
        short s = java_throws.full_of_exceptions(10);
      }
      catch (ClassNotFoundException e) {}
      catch (NoSuchFieldException e) { pass = true; }
      catch (InstantiationException e) {}
      catch (CloneNotSupportedException e) {}
      catch (IllegalAccessException e) {}

      if (!pass)
        throw new RuntimeException("Test 1 failed");

      // Check the exception class in the throw typemap
      pass = false;
      try {
        java_throws.throw_spec_function(100);
      }
      catch (IllegalAccessException e) { pass = true; }

      if (!pass)
        throw new RuntimeException("Test 2 failed");

      // Check newfree typemap throws attribute
      try {
        TestClass tc = java_throws.makeTestClass();
      }
      catch (NoSuchMethodException e) {}

      // Check javaout typemap throws attribute
      pass = false;
      try {
        int myInt = java_throws.ioTest();
      }
      catch (java.io.IOException e) { pass = true; }

      if (!pass)
        throw new RuntimeException("Test 4 failed");
      
  }
}
