require 'lib_std_string'

include Lib_std_string

# Checking expected use of %typemap(in) std::string {}
test_value("Fee")

# Checking expected result of %typemap(out) std::string {}
raise RuntimeError unless test_value("Fi") == "Fi"

# Verify type-checking for %typemap(in) std::string {}
exceptionRaised = false
begin
  test_value(0)
rescue TypeError
  exceptionRaised = true
ensure
  raise RuntimeError unless exceptionRaised
end

# Checking expected use of %typemap(in) const std::string & {}
test_const_reference("Fo")

# Checking expected result of %typemap(out) const std::string& {}
raise RuntimeError unless test_const_reference("Fum") == "Fum"

# Verify type-checking for %typemap(in) const std::string & {}
exceptionRaised = false
begin
  test_const_reference(0)
rescue TypeError
  exceptionRaised = true
ensure
  raise RuntimeError unless exceptionRaised
end

#
# Input and output typemaps for pointers and non-const references to
# std::string are *not* supported; the following tests confirm
# that none of these cases are slipping through.
#

exceptionRaised = false
begin
  test_pointer("foo")
rescue TypeError
  exceptionRaised = true
ensure
  raise RuntimeError unless exceptionRaised
end

result = test_pointer_out()
raise RuntimeError if result.is_a? String

exceptionRaised = false
begin
  test_const_pointer("bar")
rescue TypeError
  exceptionRaised = true
ensure
  raise RuntimeError unless exceptionRaised
end

result = test_const_pointer_out()
raise RuntimeError if result.is_a? String

exceptionRaised = false
begin
  test_reference("foo")
rescue TypeError
  exceptionRaised = true
ensure
  raise RuntimeError unless exceptionRaised
end

result = test_reference_out()
raise RuntimeError if result.is_a? String

