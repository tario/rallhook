require "test/unit"
require "rallhook"

class BasicHookProc
  def handle_method(klass,recv_,m,args, method_id)
	  nil
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