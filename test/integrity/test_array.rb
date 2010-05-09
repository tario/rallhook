
require "test/unit"
require "test/basic_proc"

class TestArray < Test::Unit::TestCase

  include BasicHookProcTest

  def test_each
         [1,2,3].inject do |x,y|
        print x,"-",y,"\n"
        9
      end

    hook {
      [1,2,3].inject do |x,y|
        print x,"-",y,"\n"
        9
      end
    }
  end

  def test_array_basic
    assert_equal hook{
      [1,2,3,4]
    }, [1,2,3,4]
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

    h0 = Hash.new
    h1 = Hash.new
    h2 = Hash.new
    # oracle testing, the result of the method out of hook must be the same in the hook
    25.times do
      ary = [rand(20),rand(20),rand(20),rand(20),rand(20)]
      h0[ary] = f0(ary)
      h1[ary] = f1(ary)
      h2[ary] = f2(ary)

    end

    h0.each do |input,output|
      assert_equal hook{f0(input)}, output
      assert_equal hook{f1(input)}, h1[input]
#      assert_equal hook{f2(input)}, h2[input]
    end
  end

end
