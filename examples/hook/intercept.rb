require "rubygems"
require "rallhook"

include RallHook

class MethodHandler
	include RallHook::Helper
	
	class FooMethodWrapper < MethodWrapper
		def call(a)
			# replace the a parameter with a+5
			original_call(a+5)
		end
	end

	def handle_method (klass,recv,m, method_id)

		if m == :foo
			return FooMethodWrapper.redirect_handler(klass,recv,m,method_id)
		end

		nil # do nothing
	end
	
end

class X
	def foo(a)
		print "foo in X with a = #{a}\n"
	end
end
x = X.new
print "calling X#foo without hook\n"
x.foo 4 # "foo in X with a = 4"

print "calling X#foo WITH hook\n"
mhandler = MethodHandler.new
Hook.hook mhandler do
x.foo 4 # "foo in X with a = 9" :)
end

