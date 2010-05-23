require "rubygems"
require "rallhook"

include RallHook

class HookProc
	def handle_method (klass,self_,m,args, method_id)
	
		print "method call #{m}:#{method_id} over #{self_}:#{self_.class}|#{klass} args: #{args.inspect}\n"
		nil # do nothing
	end
end

class X
	def foo
	end
end

class Y <X
	def foo
		super
	end
end

hook_proc = HookProc.new
RallHook::RallHook.new.hook hook_proc do
	y = Y.new
	y.foo
end
