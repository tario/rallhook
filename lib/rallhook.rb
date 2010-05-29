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
#
#Internal class used for return method redirection messages
#Example:
# ...
# class X
#   def foo
#   end
# end
# def handle_method
#   return RallHook::Redirect.new(X, X.new, :foo)
# end
# ...
#
#Note: Use x.redirect(:foo) instead (see Object#redirect )
#
  class Redirect
		include MethodRedirect

		def initialize(klass, recv, m, unhook = nil)
			@klass = klass
			@recv = recv
			@method = m
      @unhook = unhook
		end
	end
#
#Internal class used by RallHook::Helper#return_value
#
	class ReturnHolder
	 	def initialize(v)
			@value = v
    end
    def value(*x)
      @value
    end
  end

#
#This module brings together classes and methods to facilitate the handling
#
	module Helper

#Equivalent to:
# recv.redirect(m, klass)
		def redirect_call(klass, recv, m)
			::RallHook::Redirect.new(klass,recv,m)
		end

#Return a value as return value of the method being handled
#Example:
#
# include RallHook::Helper
# def handle_method
#   return_value(4) # all methods called returns 4
# end
#
		def return_value(v)
      recv = ReturnHolder.new(v)
      klass = recv.class
      m = :value
			redirect_call(klass, recv, m)
	  end

#
#This class acts as base to define classes that acts as receptors and wrappers of
#redirected methods. Allows interception of methods with its parameters and controlled recall
#in a hooking logic
#
#Example 1: basic modifications of arguments
#
# class MethodHandler
#  include RallHook::Helper
#
#  class FooMethodWrapper < RallHook::MethodWrapper
#    def call(foo_argument)
#      original_call(foo_argument + 5) # add 5 in all calls to foo
#    end
#  end
#
#  def handle_method (klass,recv,m,method_id)
#   if m == :foo
#    FooMethodWrapper.redirect_handler(klass,recv,m,method_id)
#   else
#    nil
#   end
#  end
# end
#
# ... hooking using MethodHandler ( see Hook#hook, README and examples)
#
#
#Example 2: generic recall of methods
#
# class MethodHandler
#  include RallHook::Helper
#
#  class GenericMethodWrapper < RallHook::MethodWrapper
#    def call(*x)
#      # call with reyield if block_given
#      if block_given?
#        original_call(*x) do |*a|
#          yield(*a)
#        end
#      else
#        original_call(*x)
#      end
#    end
#  end
#
#  def handle_method (klass,recv,m,method_id)
#    return GenericMethodWrapper.redirect_handler(klass,recv,m,method_id)
#  end
# end
#
# ... hooking using MethodHandler ( see Hook#hook, README and examples)
#
#
    class MethodWrapper

      attr_accessor :klass, :recv, :method_name, :method_id

#Reactivate the hook status to intercept methods
#Allow to pass a block to enable the hook status only in that block
#
#Example:
# class MethodHandler
#  include RallHook::Helper
#
#  class FooMethodWrapper < RallHook::MethodWrapper
#    def call(foo_argument)
#      original_call(foo_argument + 5) # add 5 in all calls to foo
#      rehook do  # the print "hello world hooked" are intercepted too
#         print "hello world hooked\n"
#      end
#    end
#  end
#
#  ... definition of handle_method that use  FooMethodWrapper as redirect, etc...
#
#Note: is not necesary to use rehook to hook the nested calls in original_call
#MethodWrapper#original_call does that internally
#
      def rehook
        if block_given?
          ::RallHook::Hook.rehook do
            yield
          end
        else
          ::RallHook::Hook.rehook
        end
      end

#
#Same as Hook#from
#
      def from(a)
        ::RallHook::Hook.from(a)
        self
      end

#
#Recall the original method(specified in initialization parameters)
#
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

#
#Target of redirection, should be implemented to intercept the call
#
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

#
#Makes a redirect_handler to wrap a method call with this MethodWrapper. Example:
#
# FooMethodWrapper.redirect_handler(klass,recv,m,method_id)
#
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



class HookHandler

#default method_handler of a Hook does nothing with any method call
  def handle_method
    nil
  end

#enable the hook. Example
#
#  class MethodHandler < Hook
#    def handle_method
#      nil # do nothing
#    end
#  end
#
#  MethodHandler.hook do
#     print "hello world\n"
#  end
#

  def hook
    if block_given?
      ::RallHook::Hook.hook self do
        yield
      end
    else
      ::RallHook::Hook.hook self
    end
  end

#Instance handler and activate the hook
#Example:
#
#  class MethodHandler < Hook
#    def handle_method
#      nil # do nothing
#    end
#  end
#
#  MethodHandler.hook do
#    print "hello world\n"
#  end

  def self.hook
    if block_given?
      self.new.hook do
        yield
      end
    else
      self.new.hook
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
      ::RallHook::Redirect.new(klass,self,method_name,true)
    else
      ::RallHook::Redirect.new(self.class,self,method_name,true)
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
			::RallHook::Redirect.new(klass,self,method_name)
		else
			::RallHook::Redirect.new(self.class,self,method_name)
		end
	end
end
