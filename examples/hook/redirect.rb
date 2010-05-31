require "rubygems"
require "rallhook"

include RallHook

class MethodHandler
	include RallHook::Helper

	def handle_method (klass,recv,m,method_id)

		if m == :foo
			# redirects the calls of method foo to method bar on the same object
			return redirect_call(klass,recv, :bar)
		end

		if m == :bar
			# redirects the calls of method bar to method bar on the method_handler
			return redirect_call(self.class,self, :bar)
		end

		nil # do nothing
	end
	
	def bar
		print "bar in MethodHandler\n"
	end
end

class X
	def foo
		print "foo in X\n"
	end
	def bar
		print "bar in X\n"
	end
end

mhandler = MethodHandler.new

print "WITHOUT HOOK:\n"

x = X.new
x.foo
x.bar

print "WITH HOOK:\n"

Hook.hook mhandler do
# all ruby calls in this block are intercepted by hook_recv::call
x = X.new
x.foo # redirected to X#bar
x.bar # redirected to MethodHandler#bar

end

