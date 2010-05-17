require "rallhook"
require "source1.rb"
require "source2.rb"

x = X.new
y = Y.new

print x.method(:foo).body.file,"\n" # source1.rb
print y.method(:foo).body.file,"\n" # source2.rb

print y.method(X,:foo).body.file,"\n" # source1.rb
print y.method(Y,:foo).body.file,"\n" # source2.rb
