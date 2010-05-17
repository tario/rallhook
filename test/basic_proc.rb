require "test/unit"
require "rallhook"

class BasicHookProc
  def call(klass,recv_,m,args, method_id)
    ret = nil
    if block_given?
      ret = recv_.hooked_send(klass,method_id,*args) do |*args_|
        yield(*args_)
      end
    else
      ret = recv_.hooked_send(klass,method_id,*args)
    end
    ret
  end
end

module BasicHookProcTest
  def hook
    rallhook = RallHook.new
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