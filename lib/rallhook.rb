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

		def initialize(klass, recv, m, unhook = nil)
			@klass = klass
			@recv = recv
			@method = m
      @unhook = unhook
		end
	end

	class ReturnHolder
	 	def initialize(v)
			@value = v
    end
    def value(*x)
      @value
    end
  end

	module Helper

		def redirect_call(klass, recv, m)
			::RallHook::Redirect_.new(klass,recv,m)
		end

		def return_value(v)
      recv = ReturnHolder.new(v)
      klass = recv.class
      m = :value
			redirect_call(klass, recv, m)
	  end

    class MethodWrapper

      attr_accessor :klass, :recv, :method_name, :method_id

      def rehook
        if block_given?
          ::RallHook::Hook.rehook do
            yield
          end
        else
          ::RallHook::Hook.rehook
        end
      end

      def from(a)
        ::RallHook::Hook.new.from(a)
        self
      end

      def original_call(*args)
       mname = self.method_name
       mklass = self.klass

       if block_given?
         from(2).rehook do
           recv.method(mklass,mname).call(*args) do |*blockargs|
             yield(*blockargs)
           end
         end
       else
         from(2).rehook do
           recv.method(mklass,mname).call(*args)
         end
       end
      end

      def call(*args)
      end

      def call_with_rehook(*args)
        call(*args)
      ensure
        rehook
      end

      def self.redirect_handler(klass,recv,method_name, method_id)
        mw = self.new
        mw.klass = klass
        mw.recv = recv
        mw.method_name = method_name
        mw.method_id = method_id
        mw.redirect_with_unhook(:call_with_rehook)
      end

    end
  end


end

class Object

  def redirect_with_unhook(m, klass = nil)
    if klass
      ::RallHook::Redirect_.new(klass,self,m,true)
    else
      ::RallHook::Redirect_.new(self.class,self,m,true)
    end

  end
	def redirect(m, klass = nil)
		if klass
			::RallHook::Redirect_.new(klass,self,m)
		else
			::RallHook::Redirect_.new(self.class,self,m)
		end
	end
end
