module li_std_vector_runme;

import tango.core.Exception;
import tango.io.Stdout;
import Integer = tango.text.convert.Integer;
import li_std_vector;

const size_t SIZE = 20;

void main() {
  // Setup vector.
  auto vect = new DoubleVector();
  for (size_t i = 0; i < SIZE; ++i) {
    vect ~= i * 10.1;
  }

  if (vect.length != SIZE) {
    throw new Exception("length test failed.");
  }

  // Item indexing.
  vect[0] = 200.1;
  if (vect[0] != 200.1) {
    throw new Exception("indexing test failed");
  }
  vect[0] = 0 * 10.1;

  // Out of range test.
  try {
    vect[vect.length] = 777.1;
    throw new Exception("out of range test failed");
  } catch (NoSuchElementException) {
  }

  // foreach test.
  {
    foreach (i, value; vect) {
      if (value != (i * 10.1)) {
	throw new Exception("foreach test failed, i:" ~ Integer.toString(i));
      }
    }
  }

  // Slice tests.
  {
    double[] array = vect[];
    foreach (i, value; array) {
      if (vect[i] != value) {
	throw new Exception("slice test 1 failed, i:" ~ Integer.toString(i));
      }
    }

    auto sVector = new StructVector();
    for (int i=0; i < 10; i++) {
      sVector ~= new Struct(i/10.0);
    }

    Struct[] sArray = sVector[];
    foreach(i, value; sArray) {
      if (value.num != sVector[i].num) {
	throw new Exception("slice test 2 failed, i:" ~ Integer.toString(i));
      }
    }

    foreach (ref s; sVector) {
      s.num = s.num + 20.0;
    }
    foreach(i, value; sArray) {
      if (value.num != sVector[i].num) {
	throw new Exception("slice test 3 failed (a deep copy was made), i:" ~ Integer.toString(i));
      }
    }
  }

  // remove() tests.
  {
    auto iVector = new IntVector();
    for (int i = 0; i < SIZE; i++) {
      iVector ~= i;
    }

    iVector.remove(iVector.length - 1);
    iVector.remove(SIZE / 2);
    iVector.remove(0);

    try {
      iVector.remove(iVector.size);
      throw new Exception("remove test failed");
    } catch (NoSuchElementException) {
    }
  }

  // capacity tests.
  {
    auto dv = new DoubleVector(10);
    if ((dv.capacity != 10) || (dv.length != 0)) {
      throw new Exception("constructor setting capacity test failed");
    }

    // TODO: Is this really required (and spec'ed) behavior?
    dv.capacity = 20;
    if (dv.capacity != 20) {
      throw new Exception("capacity test (1) failed");
    }

    dv ~= 1.11;
    try {
      dv.capacity = dv.length - 1;
      throw new Exception("capacity test (2) failed");
    } catch (IllegalArgumentException) {
    }
  }

  // clear() test
  vect.clear();
  if (vect.size != 0) {
    throw new Exception("clear test failed");
  }

  // Finally test the methods being wrapped
  {
    auto iv = new IntVector();
    for (int i=0; i<4; i++) {
      iv ~= i;
    }

    double x = average(iv);
    x += average(new IntVector([1, 2, 3, 4]));
    RealVector rv = half(new RealVector([10.0f, 10.5f, 11.0f, 11.5f]));

    auto dv = new DoubleVector();
    for (size_t i = 0; i < SIZE; i++) {
      dv ~= i / 2.0;
    }
    halve_in_place(dv);

    RealVector v0 = vecreal(new RealVector());
    float flo = 123.456f;
    v0 ~= flo;
    flo = v0[0];

    IntVector v1 = vecintptr(new IntVector());
    IntPtrVector v2 = vecintptr(new IntPtrVector());
    IntConstPtrVector v3 = vecintconstptr(new IntConstPtrVector());

    v1 ~= 123;
    v2.clear();
    v3.clear();

    StructVector v4 = vecstruct(new StructVector());
    StructPtrVector v5 = vecstructptr(new StructPtrVector());
    StructConstPtrVector v6 = vecstructconstptr(new StructConstPtrVector());

    v4 ~= new Struct(123);
    v5 ~= new Struct(123);
    v6 ~= new Struct(123);
  }

  // Test vectors of pointers
  {
    auto vector = new StructPtrVector();
    for (size_t i = 0; i < SIZE; i++) {
      vector ~= new Struct(i/10.0);
    }

    Struct[] array = vector[];

    for (size_t i = 0; i < SIZE; i++) {
      if (array[i].num != vector[i].num) {
	throw new Exception("StructPtrVector test 1 failed, i:" ~ Integer.toString(i));
      }
    }

    foreach (ref s; vector) {
      s.num = s.num + 20.0;
    }

    for (size_t i = 0; i < SIZE; i++) {
      if (array[i].num != (20.0 + (i / 10.0))) {
	throw new Exception("StructPtrVector test 2 failed (a deep copy was incorrectly made), i:" ~ Integer.toString(i));
      }
    }
  }

  // Test vectors of const pointers
  {
    auto vector = new StructConstPtrVector();
    for (size_t i = 0; i < SIZE; i++) {
      vector ~= new Struct(i / 10.0);
    }

    Struct[] array = vector[];

    for (size_t i = 0; i < SIZE; i++) {
      if (array[i].num != vector[i].num) {
	throw new Exception("StructConstPtrVector test 1 failed, i:" ~ Integer.toString(i));
      }
    }

    foreach (ref s; vector) {
      s.num = s.num + 20.0;
    }

    for (size_t i = 0; i < SIZE; i++) {
      if (array[i].num != (20.0 + (i / 10.0))) {
	throw new Exception("StructConstPtrVector test 2 failed (a deep copy was incorrectly made), i:" ~ Integer.toString(i));
      }
    }
  }

  // dispose()
  {
    {
      scope vs = new StructVector();
      vs ~= new Struct(0.0);
      vs ~= new Struct(11.1);
    }
    {
      scope vd = new DoubleVector();
      vd ~= 0.0;
      vd ~= 11.1;
    }
  }
}
