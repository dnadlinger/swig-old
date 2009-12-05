module catches_runme;

import catches;

void main() {
  // test_catches()
  try {
    catches.test_catches(1);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "C++ int exception thrown, value: 1")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }

  try {
    catches.test_catches(2);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "two")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }

  try {
    catches.test_catches(3);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "C++ ThreeException const & exception thrown")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }

  // test_exception_specification()
  try {
    catches.test_exception_specification(1);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "C++ int exception thrown, value: 1")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }

  try {
    catches.test_exception_specification(2);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "unknown exception")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }

  try {
    catches.test_exception_specification(3);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "unknown exception")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }

  // test_catches_all()
  try {
    catches.test_catches_all(1);
    throw new Exception("missed exception");
  } catch (Exception e) {
    if (e.msg != "unknown exception")
      throw new Exception("Failed to propagate C++ exception. Received instead: " ~ e.msg);
  }
}
