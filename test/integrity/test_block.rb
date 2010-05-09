
require "test/unit"
require "test/basic_proc"

class TestBlock < Test::Unit::TestCase

  include BasicHookProcTest

  def block_func(*a)
    yield(*a)
  end

  def _test_block_passing(*test_args)

    result = nil
    hook do
      block_func(*test_args) do |*args|
        result = args
      end
    end

    assert_equal result, test_args

  end

  def test_block
    _test_block_passing(1)
    _test_block_passing(1,2)
    _test_block_passing("3333")
    _test_block_passing("3333",9999, 1.32)
  end
end

