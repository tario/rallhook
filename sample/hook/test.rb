require "rubygems"
require "rallhook"
require "hook_proc"

print_hook do
	print 2+2,"\n"
	print [1,2,3].inject{|x,y| x+y},"\n"
end
