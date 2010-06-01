require "test/unit"
require "test/basic_proc"

class InstrospectionTestCall < Test::Unit::TestCase

  class X
    def foo
      "fooX"
    end
  end

  class Y < X
    def foo
      "fooY"
    end
  end

  def test_file
    y = Y.new

    assert_equal y.specific_method(:foo).call, "fooY"
    assert_equal y.specific_method(X,:foo).call, "fooX"
    assert_equal y.specific_method(Y,:foo).call, "fooY"

    assert_raise NameError do
      y.specific_method(Fixnum,:foo)
    end
  end
end
