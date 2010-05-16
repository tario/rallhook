require "rallhook"
require "hook_proc"

class X
	def foo
	end
end

class Y <X
	def foo
		super
	end
end

print_hook do
	y = Y.new
	y.foo
end