require "rubygems"
require "rallhook"

include RallHook

class HookProc
	def handle_method (klass,self_,m,args, method_id)
	
		print "method call #{m}:#{method_id} over #{self_}:#{self_.class}|#{klass} args: #{args.inspect}\n"
		nil # do nothing
	end
end

hook_proc = HookProc.new
Hook.new.hook hook_proc do
	print 2+2,"\n"
	print [1,2,3].inject{|x,y| x+y},"\n"
end
