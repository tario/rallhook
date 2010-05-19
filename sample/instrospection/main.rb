require "rubygems"
require "rallhook"
require "source1.rb"
require "source2.rb"

x = X.new
y = Y.new

print x._method(:foo).body.file,"\n" # source1.rb
print y._method(:foo).body.file,"\n" # source2.rb

print y._method(X,:foo).body.file,"\n" # source1.rb
print y._method(Y,:foo).body.file,"\n" # source2.rb
