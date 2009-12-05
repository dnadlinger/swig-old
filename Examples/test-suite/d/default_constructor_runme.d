module default_constructor_runme;

import tango.core.Exception;
import default_constructor;

void main() {
  // Protected destructor test.
  try {
      {
	scope g = new G();
      }
      throw new Exception("Protected destructor exception should have been thrown");
  } catch (FinalizeException) {
  }

  // Private destructor test.
  try {
      {
	scope f = new FFF();
      }
      throw new Exception("Private destructor exception should have been thrown");
  } catch (FinalizeException) {
  }
}