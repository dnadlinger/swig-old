#!/usr/bin/env ruby
#
# Put description here
#
# Author::    gga
# Copyright:: 2007
# License::   SWIG
#

require 'swig_assert'

require 'static_const_member_2'

include Static_const_member_2

c = Test_int.new
a = Test_int::Forward_field	# should be available as a class constant
a = Test_int::Current_profile	# should be available as a class constant
a = Test_int::RightIndex	# should be available as a class constant
a = Test_int::Backward_field	# should be available as a class constant
a = Test_int::LeftIndex		# should be available as a class constant
a = Test_int.cavity_flags

