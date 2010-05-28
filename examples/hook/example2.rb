require "rubygems"
require "rallhook"

include RallHook

class HookProc
	def handle_method (klass,self_,m,method_id)

		# print to the standard output details about the method called
		print "method call #{m}:#{method_id} over #{self_}:#{self_.class}\n"
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
Hook.new.hook hook_proc do
	y = Y.new
	y.foo
end
