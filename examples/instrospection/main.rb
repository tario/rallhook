require "rubygems"
require "rallhook"
require "source1.rb"
require "source2.rb"

x = X.new
y = Y.new

print "definition of method :foo in #{x}: ", x.method(:foo).body.file,"\n" # source1.rb
print "definition of method :foo in #{y}: ", y.method(:foo).body.file,"\n" # source2.rb

print "definition of method :foo if class X in object #{y}: ", y.method(X,:foo).body.file,"\n" # source1.rb
print "definition of method :foo if class Y in object #{y}: ", y.method(Y,:foo).body.file,"\n" # source1.rb
