require "test/unit"
require "test/basic_proc"

class TestSuper < Test::Unit::TestCase

  include BasicHookProcTest

  class X
    def foo
      [1]
    end
  end
  class Y < X
    def foo
      super+[2]
    end
  end
  class Z < Y
    def foo
      super+[3]
    end
  end

  def f0(klass)
    z = klass.new
    z.foo
  end
  def test_basic
    oracle_hook_assert_equal do f0(X) end
    oracle_hook_assert_equal do f0(Y) end
    oracle_hook_assert_equal do f0(Z) end
  end
end

