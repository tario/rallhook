require "test/unit"
require "test/basic_proc"


class TestClassMethod < Test::Unit::TestCase

  class << self
    attr_accessor :global_binding
  end

  TestClassMethod.global_binding = binding

  class X
    def self.foo
    end
  end

  class MethodHandler < RallHook::HookHandler
    def handle_method (klass,self_,m,method_id)
      if m
      file = klass.instance_method(m).body.file
      end

      nil # do nothing
    end
  end


  def test_file
    assert_nothing_raised do
      MethodHandler.hook do
        eval( "
          class Y < X
          end
          Y.foo
        ", TestClassMethod.global_binding)

      end
    end
  end
end
