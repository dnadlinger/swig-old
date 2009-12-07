module special_variable_macros_runme;

import special_variable_macros;

void main() {
  auto name = new Name();

  if (testFred(name) != "none") {
    throw new Exception("test failed");
  }

  if (testJack(name) != "$specialname") {
    throw new Exception("test failed");
  }

  if (testJill(name) != "jilly") {
    throw new Exception("test failed");
  }

  if (testMary(name) != "SWIGTYPE_p_NameWrap") {
    throw new Exception("test failed");
  }

  if (testJim(name) != "multiname num") {
    throw new Exception("test failed");
  }

  if (testJohn(new PairIntBool(10, false)) != 123) {
    throw new Exception("test failed");
  }

  NewName newName = NewName.factory("factoryname");
  if (newName.getStoredName().getName() != "factoryname") {
    throw new Exception("test failed");
  }
}
