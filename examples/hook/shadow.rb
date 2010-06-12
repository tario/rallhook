require "rubygems"
require "rallhook"

include RallHook

class MethodHandler < RallHook::HookHandler
	def handle_method (klass,self_,m,method_id)
		# print to the standard output details about the method called
		print "method call #{m}:#{method_id} over #{self_}:#{self_.class}\n"
		nil # do nothing
	end
end

class X
	def foo
		print "outside of hook foo\n"
	end
end

x = X.new

x.foo

MethodHandler.hook do
	class X
		def foo
			print "inside of hook foo\n"
		end
		
		def bar
			print "inside of hook bar\n"
		end
	end
	
	x.foo
	x.bar
end

x.foo
begin
x.bar # NoMethodError
rescue NoMethodError
print "there is no method bar in X outside the hook\n"
end
