
require "test/unit"
require "test/basic_proc"

class TestArray < Test::Unit::TestCase

  include BasicHookProcTest


  def test_array_basic
    oracle_hook_assert { [1,2,3,4] }
  end

  def f0(input)
    input.select{|i| i%2==0}
  end
  def f1(input)
    input.map{|i| i*5+2}
  end
  def f2(input)
    input.inject{|x,y| x*y}
  end

  def test_array_blocks

    # oracle testing, the result of the method out of hook must be the same in the hook
    25.times do
      ary = [rand(20),rand(20),rand(20),rand(20),rand(20)]
      oracle_hook_assert do
        f0(ary)
      end
      oracle_hook_assert do
        f1(ary)
      end
      oracle_hook_assert do
        f2(ary)
      end

    end

  end

end
