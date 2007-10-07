import li_boost_shared_ptr.*;

public class li_boost_shared_ptr_runme {
  static {
    try {
        System.loadLibrary("li_boost_shared_ptr");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
      System.exit(1);
    }
  }

  // Debugging flag
  public final static boolean debug = false;

  public static void main(String argv[]) 
  {
    if (debug)
      System.out.println("Started");

    li_boost_shared_ptr.setDebug_shared(debug);

    final int loopCount = 1;
    for (int i=0; i<loopCount; i++) {
      new li_boost_shared_ptr_runme().runtest();
      System.gc();
      System.runFinalization();
      try {
        if (i%10 == 0) {
//          System.out.println("i: " + i + " " + Klass.getTotal_count());
          java.lang.Thread.sleep(10);
        }
      } catch (java.lang.InterruptedException e) {
      }
    }

    if (debug)
      System.out.println("Nearly finished");

    int countdown = 100;
    while (true) {
      System.gc();
      System.runFinalization();
      try {
        java.lang.Thread.sleep(10);
      } catch (java.lang.InterruptedException e) {
      }
      if (--countdown == 0)
        break;
      if (Klass.getTotal_count() == 0)
        break;
    };
    if (Klass.getTotal_count() != 0)
      throw new RuntimeException("Klass.total_count=" + Klass.getTotal_count());

    if (debug)
      System.out.println("Finished");
  }

  private void runtest() {
    // simple shared_ptr usage - created in C++
    {
      Klass k = new Klass("me oh my");
      String val = k.getValue();
      verifyValue("me oh my", val);
      verifyCount(1, k);
    }

    // simple shared_ptr usage - not created in C++
    {
      Klass k = li_boost_shared_ptr.factorycreate();
      String val = k.getValue();
      verifyValue("factorycreate", val);
      verifyCount(1, k);
    }

    // pass by shared_ptr
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.smartpointertest(k);
      String val = kret.getValue();
      verifyValue("me oh my smartpointertest", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // pass by shared_ptr pointer
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.smartpointerpointertest(k);
      String val = kret.getValue();
      verifyValue("me oh my smartpointerpointertest", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // pass by shared_ptr reference
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.smartpointerreftest(k);
      String val = kret.getValue();
      verifyValue("me oh my smartpointerreftest", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // pass by shared_ptr pointer reference
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.smartpointerpointerreftest(k);
      String val = kret.getValue();
      verifyValue("me oh my smartpointerpointerreftest", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // const pass by shared_ptr
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.constsmartpointertest(k);
      String val = kret.getValue();
      verifyValue("me oh my", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // const pass by shared_ptr pointer
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.constsmartpointerpointertest(k);
      String val = kret.getValue();
      verifyValue("me oh my", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // const pass by shared_ptr reference
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.constsmartpointerreftest(k);
      String val = kret.getValue();
      verifyValue("me oh my", val);
      verifyCount(2, k);
      verifyCount(2, kret);
    }

    // pass by value
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.valuetest(k);
      String val = kret.getValue();
      verifyValue("me oh my valuetest", val);
      verifyCount(1, k);
      verifyCount(1, kret);
    }

    // pass by pointer
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.pointertest(k);
      String val = kret.getValue();
      verifyValue("me oh my pointertest", val);
      verifyCount(1, k);
      verifyCount(1, kret);
    }

    // pass by reference
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.reftest(k);
      String val = kret.getValue();
      verifyValue("me oh my reftest", val);
      verifyCount(1, k);
      verifyCount(1, kret);
    }

    // pass by pointer reference
    {
      Klass k = new Klass("me oh my");
      Klass kret = li_boost_shared_ptr.pointerreftest(k);
      String val = kret.getValue();
      verifyValue("me oh my pointerreftest", val);
      verifyCount(1, k);
      verifyCount(1, kret);
    }

    // null tests
    {
      Klass k = null;

      if (li_boost_shared_ptr.smartpointertest(k) != null)
        throw new RuntimeException("return was not null");

      if (li_boost_shared_ptr.smartpointerpointertest(k) != null)
        throw new RuntimeException("return was not null");

      if (li_boost_shared_ptr.smartpointerreftest(k) != null)
        throw new RuntimeException("return was not null");

      if (li_boost_shared_ptr.smartpointerpointerreftest(k) != null)
        throw new RuntimeException("return was not null");

      if (!li_boost_shared_ptr.nullsmartpointerpointertest(null).equals("null pointer"))
        throw new RuntimeException("not null smartpointer pointer");

      try { li_boost_shared_ptr.valuetest(k); throw new RuntimeException("Failed to catch null pointer"); } catch (NullPointerException e) {}

      if (li_boost_shared_ptr.pointertest(k) != null)
        throw new RuntimeException("return was not null");

      try { li_boost_shared_ptr.reftest(k); throw new RuntimeException("Failed to catch null pointer"); } catch (NullPointerException e) {}
    }

    // $owner
    {
      Klass k = li_boost_shared_ptr.pointerownertest();
      String val = k.getValue();
      verifyValue("pointerownertest", val);
      verifyCount(1, k);
    }
    {
      Klass k = li_boost_shared_ptr.smartpointerpointerownertest();
      String val = k.getValue();
      verifyValue("smartpointerpointerownertest", val);
      verifyCount(1, k);
    }

    ////////////////////////////////// Derived classes ////////////////////////////////////////
    // derived pass by pointer
    {
      KlassDerived k = new KlassDerived("me oh my");
      KlassDerived kret = li_boost_shared_ptr.derivedpointertest(k);
      String val = kret.getValue();
      verifyValue("me oh my derivedpointertest-Derived", val);
      verifyCount(2, k); // includes an extra reference for the upcast
      verifyCount(2, kret);
    }

    // check derivedness
    {
      /*
      KlassDerived k = new KlassDerived("me oh my");
      KlassDerived kret = li_boost_shared_ptr.derivedpointertest(k);
      String val = kret.getValue();
      k.append("-Appended");
      val = k.getValue();
      verifyValue("me oh my derivedpointertest-Appended-Derived", val);
      val = kret.getValue();
      verifyValue("me oh my derivedpointertest-Appended-Derived", val);
      // TODO: clone it
      */
    }

    // Member variables
    // smart pointer by value
    {
      MemberVariables m = new MemberVariables();
      Klass k = new Klass("smart member value");
      m.setSmartMemberValue(k);
      String val = k.getValue();
      verifyValue("smart member value", val);
      verifyCount(2, k);

      Klass kmember = m.getSmartMemberValue();
      val = kmember.getValue();
      verifyValue("smart member value", val);
      verifyCount(3, kmember);
      verifyCount(3, k);

      m.delete();
      verifyCount(2, kmember);
      verifyCount(2, k);
    }
    // smart pointer by pointer
    {
      MemberVariables m = new MemberVariables();
      Klass k = new Klass("smart member pointer");
      m.setSmartMemberPointer(k);
      String val = k.getValue();
      verifyValue("smart member pointer", val);
      verifyCount(1, k);

      Klass kmember = m.getSmartMemberPointer();
      val = kmember.getValue();
      verifyValue("smart member pointer", val);
      verifyCount(2, kmember);
      verifyCount(2, k);

      m.delete();
      verifyCount(2, kmember);
      verifyCount(2, k);
    }
    // smart pointer by reference
    {
      MemberVariables m = new MemberVariables();
      Klass k = new Klass("smart member reference");
      m.setSmartMemberReference(k);
      String val = k.getValue();
      verifyValue("smart member reference", val);
      verifyCount(2, k);

      Klass kmember = m.getSmartMemberReference();
      val = kmember.getValue();
      verifyValue("smart member reference", val);
      verifyCount(3, kmember);
      verifyCount(3, k);

      // The C++ reference refers to SmartMemberValue...
      Klass kmemberVal = m.getSmartMemberValue();
      val = kmember.getValue();
      verifyValue("smart member reference", val);
      verifyCount(4, kmemberVal);
      verifyCount(4, kmember);
      verifyCount(4, k);

      m.delete();
      verifyCount(3, kmemberVal);
      verifyCount(3, kmember);
      verifyCount(3, k);
    }
    // plain by value
    {
      MemberVariables m = new MemberVariables();
      Klass k = new Klass("plain member value");
      m.setMemberValue(k);
      String val = k.getValue();
      verifyValue("plain member value", val);
      verifyCount(1, k);

      Klass kmember = m.getMemberValue();
      val = kmember.getValue();
      verifyValue("plain member value", val);
      verifyCount(1, kmember);
      verifyCount(1, k);

      m.delete();
      verifyCount(1, kmember);
      verifyCount(1, k);
    }
    // plain by pointer
    {
      MemberVariables m = new MemberVariables();
      Klass k = new Klass("plain member pointer");
      m.setMemberPointer(k);
      String val = k.getValue();
      verifyValue("plain member pointer", val);
      verifyCount(1, k);

      Klass kmember = m.getMemberPointer();
      val = kmember.getValue();
      verifyValue("plain member pointer", val);
      verifyCount(1, kmember);
      verifyCount(1, k);

      m.delete();
      verifyCount(1, kmember);
      verifyCount(1, k);
    }
    // plain by reference
    {
      MemberVariables m = new MemberVariables();
      Klass k = new Klass("plain member reference");
      m.setMemberReference(k);
      String val = k.getValue();
      verifyValue("plain member reference", val);
      verifyCount(1, k);

      Klass kmember = m.getMemberReference();
      val = kmember.getValue();
      verifyValue("plain member reference", val);
      verifyCount(1, kmember);
      verifyCount(1, k);

      m.delete();
      verifyCount(1, kmember);
      verifyCount(1, k);
    }

    // null member variables
    {
      MemberVariables m = new MemberVariables();

      // shared_ptr by value
      Klass k = m.getSmartMemberValue();
      if (k != null)
        throw new RuntimeException("expected null");
      m.setSmartMemberValue(null);
      k = m.getSmartMemberValue();
      if (k != null)
        throw new RuntimeException("expected null");
      verifyCount(0, k);

      // plain by value
      try { m.setMemberValue(null); throw new RuntimeException("Failed to catch null pointer"); } catch (NullPointerException e) {}
    }
  }
  private void verifyValue(String expected, String got) {
    if (!expected.equals(got))
      throw new RuntimeException("verify value failed. Expected: " + expected + " Got: " + got);
  }
  private void verifyCount(int expected, Klass k) {
    int got = li_boost_shared_ptr.use_count(k); 
    if (expected != got)
      throw new RuntimeException("verify use_count failed. Expected: " + expected + " Got: " + got);
  }
}
