require "rubygems"
require "rallhook"

include RallHook

class MethodHandler
	include RallHook::Helper
	
	class FooMethodWrapper < MethodWrapper
		def call(*x)
			# call with reyield if block_given
			if block_given?
				original_call(*x) do |*a|
					yield(*a)
				end
				# add "¿bar?"
				yield("bar?")
			else
				original_call(*x)
			end
		end
	end

	def handle_method (klass,recv,m,args, method_id)
		if m == :each
			return FooMethodWrapper.redirect_handler(klass,recv,m,method_id)
		end
		nil # do nothing
	end
end

print "calling Array#each without hook\n"
[1,2,3,4].each do |x|
	print x.inspect, " "
end
print "\n"

print "calling Array#each WITH hook\n"
mhandler = MethodHandler.new
Hook.new.hook mhandler do

# 1 2 3 4 "bar?"
[1,2,3,4].each do |x|
	print x.inspect," "
end
print "\n"

end

