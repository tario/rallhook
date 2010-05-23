require "rubygems"
require "rallhook"

include RallHook

class MethodHandler

	def handle_method (klass,self_,m,args, method_id)

		# print to the standar output details about the method called
		print "method call #{m}:#{method_id} over #{self_}:#{self_.class} args: #{args.inspect}\n"
		nil # do nothing
	end
end

mhandler = MethodHandler.new

Hook.new.hook mhandler do
# all ruby calls in this block are intercepted by hook_recv::call

	[1,2,3].each do |x|
	print x,"\n"
	end

end
