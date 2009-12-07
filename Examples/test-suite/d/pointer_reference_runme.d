module pointer_reference_runme;

import pointer_reference;

void main() {
  Struct s  = pointer_reference.get();
  if (s.value != 10) throw new Exception("get test failed");

  Struct ss = new Struct(20);
  pointer_reference.set(ss);
  if (Struct.instance.value != 20) throw new Exception("set test failed");
}
