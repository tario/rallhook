require "test/unit"
require "test/basic_proc"

class InstrospectionTestFile < Test::Unit::TestCase
  def _test_file(name)
    eval("def xmethod; end", binding, name)
    assert_equal self.method(:xmethod).body.file, name
  end

  def test_file
    10.times do
    _test_file("xxx" + rand(999).to_s)
    end
  end
end
