module inherit_target_language_runme;

import inherit_target_language;

void main() {
  (new Derived1()).targetLanguageBaseMethod();
  (new Derived2()).targetLanguageBaseMethod();

  (new MultipleDerived1()).targetLanguageBaseMethod();
  (new MultipleDerived2()).targetLanguageBaseMethod();
  (new MultipleDerived3()).f();
  (new MultipleDerived4()).g();

  auto baseX = new BaseX();
  baseX.basex();
  baseX.targetLanguageBase2Method();

  auto derivedX = new DerivedX();
  derivedX.basex();
  derivedX.derivedx();
  derivedX.targetLanguageBase2Method();
}
