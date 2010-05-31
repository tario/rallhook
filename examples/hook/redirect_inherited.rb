require "rubygems"
require "rallhook"

include RallHook

class MethodHandler
	include RallHook::Helper

	def handle_method (klass,recv,m,method_id)
		
		print "called method #{m} over #{recv}\n"
		
		if m == :inherited
			return redirect_call(klass,recv,:ssss) 
		end

		nil # do nothing
	end
end

mhandler = MethodHandler.new
Hook.hook mhandler do
# all ruby calls in this block are intercepted by hook_recv::call
class N
end

end

