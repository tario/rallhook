require "test/unit"
require "test/basic_proc"

class TestBinding < Test::Unit::TestCase

  include BasicHookProcTest

  def generate_binding
    a = 1
    b = 2
    c = 3

    binding
  end

  def f0()
    eval("a+b+c", generate_binding)
  end

  def test_array_blocks
    oracle_hook_assert do
      f0
    end
  end

end
