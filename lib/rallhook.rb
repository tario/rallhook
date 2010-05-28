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
	class Redirect
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
			::RallHook::Redirect.new(klass,recv,m)
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
       mid = self.method_id
       recv_ = self.recv

       if block_given?
         from(4).rehook do
           recv_.method(mklass,mid).call(*args) do |*blockargs|
             yield(*blockargs)
           end
         end
       else
         from(4).rehook do
           recv_.method(mklass,mid).call(*args)
         end
       end
      end

      def call(*args)
      end

      def call_with_rehook(*args)
        if block_given?
          call(*args) do |*x|
            yield(*x)
          end
        else
          call(*args)
        end
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

# To facilitate the use of hooking/redirection features,
# rallhook defines some methods in the Object class
#

class Object


# Same as redirect, but disable the hook/interception of methods (dont call handle_redirect) before
# make the call, the hook status may be restored with Hook#rehook
# Anyway, it is desirable to use MethodWrapper instead to "wrap" method calls
#
# see RallHook::Helper::MethodWrapper
#
  def redirect_with_unhook(method_name, klass = nil)
    if klass
      ::RallHook::Redirect.new(klass,self,m,true)
    else
      ::RallHook::Redirect.new(self.class,self,m,true)
    end

  end

# Generates a redirect message to return it in handle_method to cause the redirection of
# the method that is beign processed with the object as receiver, the method_name the new target
# and the class as second parameter if specified
#
# Example:
#   class X
#     def foo(*args)
#     end
#   end
#
#   class MethodHandler
#     @@x = x.new
#     def handle_method(klass, recv, m , method_id)
#       if m == :bar then
#         # redirect bar calls to foo in x
#         return x.redirect(:foo)
#       end
#       nil # do nothing
#     end
#   end
#
#   # hook using MethodHandler, etc... (see README and examples)
#
#
  def redirect(method_name, klass = nil)
		if klass
			::RallHook::Redirect.new(klass,self,m)
		else
			::RallHook::Redirect.new(self.class,self,m)
		end
	end
end
