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

MethodHandler.hook do
	print 2+2,"\n"
	print [1,2,3].inject{|x,y| x+y},"\n"
end
