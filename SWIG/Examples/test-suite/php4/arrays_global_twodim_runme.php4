<?php
// Sample test file

require "tests.php4";
require "arrays_global_twodim.php";

// No new functions
check::functions(array(fn_taking_arrays,get_2d_array));
// No new classes
check::classes(array(simplestruct));
check::globals(array(array_c,array_sc,array_uc,array_s,array_us,array_i,array_ui,array_l,array_ul,array_ll,array_f,array_d,array_struct,array_structpointers,array_ipointers,array_enum,array_enumpointers,array_const_i));
$a1=array(10,11,12,13);
$a2=array(14,15,16,17);
$a=array($a1,$a2);

$_a=check::get(array_const_i);

for($x=0;$x<count($a1);$x++) {
  for($y=0;$y<2;$y++) {
    check::equal($a[$y][$x],get_2d_array($_a,$y,$x),"check array $x,$y");
  }
}

check::done();
?>
