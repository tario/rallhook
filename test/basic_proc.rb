require "test/unit"
require "rallhook"

class BasicHookProc

 include RallHook::Helper

  class BasicMethodWrapper < MethodWrapper

    def call(*args)
      if block_given?
        original_call(*args) do |*x|
         yield(*x)
        end
      else
        original_call(*args)
      end
    end
  end

  def handle_method(klass,recv_,m, method_id)
    return BasicMethodWrapper.redirect_handler(klass,recv_,m,method_id)
  end
end

module BasicHookProcTest
  def hook
    rallhook = RallHook::Hook.new
    basichook = BasicHookProc.new

    rallhook.hook basichook do
      yield
    end
  end

  def oracle_hook_assert_equal
    result = nil
    hook do
      result = yield
    end

    assert_equal result, yield
  end
  alias oracle_hook_assert oracle_hook_assert_equal
end