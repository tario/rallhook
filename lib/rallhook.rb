=begin

This file is part of the rallhook project, http://github.com/tario/

Copyright (c) 2009-2010 Roberto Dario Seminara <robertodarioseminara@gmail.com>

rallhook is free software: you can redistribute it and/or modify
it under the terms of the gnu general public license as published by
the free software foundation, either version 3 of the license, or
(at your option) any later version.

rallhook is distributed in the hope that it will be useful,
but without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.  see the
gnu general public license for more details.

you should have received a copy of the gnu general public license
along with rallhook.  if not, see <http://www.gnu.org/licenses/>.

=end
require "rubygems"
require "rallhook_base"

module RallHook
	class Redirect_
		include MethodRedirect
		
		def initialize(klass, recv, m)
			@klass = klass
			@recv = recv
			@method = m
		end
	end
	
	class ReturnValue_
		include MethodRedirect

		def initialize(v)
			@value = v
		end
	end

	module Helper
	
		def redirect_call(klass, recv, m)
			::RallHook::Redirect_.new(klass,recv,m)
		end
		
		def return_value(v)
			::RallHook::ReturnValue_.new(v)
		end
	end
end

class Object
	
	def redirect(m, klass = nil)
		if klass
			::RallHook::Redirect_.new(klass,self,m)
		else
			::RallHook::Redirect_.new(self.class,self,m)
		end
	end
end
