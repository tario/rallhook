require "rallhook"

class HookProc
	
	attr_accessor :rallhook
	
	def call (klass,self_,m,args, method_id)
	
		print "method call #{m}:#{method_id} over #{self_}:#{self_.class}|#{klass} args: #{args.inspect}\n"
		# read the method body
		ret = nil
		if block_given?
			ret = self_.hooked_send(klass,method_id,*args) do |*args_|
				yield(*args_)
			end
		else
			ret = self_.hooked_send(klass,method_id,*args)
		end
		ret
	end
end

def print_hook
	rallhook = RallHook.new
	hook_proc = HookProc.new
	rallhook.hook hook_proc do
		yield
	end
end